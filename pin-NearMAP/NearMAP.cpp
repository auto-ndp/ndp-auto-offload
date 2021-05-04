#include "pin.H"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>
using std::cerr;
using std::endl;

std::ostream *out = &cerr;

struct RWTime {
  uint64_t lastRead = 0;
  uint64_t lastWrite = 0;
};

struct AccessState {
  constexpr static uintptr_t TracePageSize = 4096;
  uint64_t currTime = 0;
  std::unordered_map<uintptr_t, RWTime> pageLastAccesses;
  std::vector<uintptr_t> phaseTimes;
} accessState;

const std::string PhaseStubName = "pinnearmap_phase";

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o",
                                 "pin_nearmap.log",
                                 "specify file name for NearMAP output");

INT32 Usage() {
  cerr << KNOB_BASE::StringKnobSummary() << endl;

  return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

void summarizeLastAccesses(uint32_t startTime, uint32_t endTime) {
  if (endTime < startTime) {
    cerr << "Error: End time < Start time" << endl;
  }
  uint64_t uniqRO = 0, uniqRW = 0, uniqWO = 0;
  for (const auto &access : accessState.pageLastAccesses) {
    const uintptr_t pageIdx = access.first;
    const RWTime rwtime = access.second;
    const bool didRead =
        rwtime.lastRead >= startTime && rwtime.lastRead < endTime;
    const bool didWrite =
        rwtime.lastWrite >= startTime && rwtime.lastWrite < endTime;
    if (!didRead && !didWrite) {
      continue;
    } else if (didRead && !didWrite) {
      uniqRO++;
    } else if (didRead && didWrite) {
      uniqRW++;
    } else {
      uniqWO++;
    }
    *out << "trace;" << ';' << startTime << ';' << endTime << ';' << pageIdx
         << ';';
    if (didRead) {
      *out << 'R';
    }
    if (didWrite) {
      *out << 'W';
    }
    *out << ";\n";
  }
  out->flush();
  cerr << "Rtn RO:" << uniqRO << " RW:" << uniqRW << " WO:" << uniqWO
       << " TOT:" << uniqRO + uniqRW + uniqWO << "\n";
}

// VOID BeforeBbl() {
//   accessState.currTime++;
//   accessState.bbTimeStack.push_back(accessState.currTime);
// }

// VOID AfterBbl(UINT32 routineNameIdx) {
//   accessState.currTime++;
//   const uint64_t endTime = accessState.currTime;
//   auto &timeStack = accessState.bbTimeStack;
//   const uint64_t startTime = timeStack.back();
//   if (timeStack.size() > 1) {
//     timeStack.pop_back();
//   } else {
//     cerr << "Warning: Trace stack underflow, results might not be accurate"
//          << endl;
//   }
//   summarizeLastAccesses(routineNameIdx, startTime, endTime);
// }

VOID OnRead(ADDRINT addr, UINT32 accessSz) {
  const uintptr_t apage = addr / AccessState::TracePageSize;
  const uintptr_t epage = (addr + accessSz - 1) / AccessState::TracePageSize;
  uint64_t time = accessState.currTime++;
  auto &map = accessState.pageLastAccesses;
  for (uintptr_t page = apage; page <= epage; page++) {
    map[page].lastRead = time;
  }
}

VOID OnWrite(ADDRINT addr, UINT32 accessSz) {
  const uintptr_t apage = addr / AccessState::TracePageSize;
  const uintptr_t epage = (addr + accessSz - 1) / AccessState::TracePageSize;
  uint64_t time = accessState.currTime++;
  auto &map = accessState.pageLastAccesses;
  for (uintptr_t page = apage; page <= epage; page++) {
    map[page].lastWrite = time;
  }
}

extern "C" {
void PhaseStubReplacement(const char *name) {
  accessState.currTime++;
  uint64_t currTime = accessState.currTime;
  uint64_t prevTime = accessState.phaseTimes.back();
  *out << "phase;" << prevTime << ';' << currTime << ';' << name << endl;
  cerr << "Phase " << name << " from " << prevTime << " to " << currTime
       << endl;
  accessState.phaseTimes.push_back(currTime);
  summarizeLastAccesses(prevTime, currTime);
}
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

VOID Routine(RTN rtn, VOID *v) {
  RTN_Open(rtn);
  do {
    if (RTN_Name(rtn) != PhaseStubName) {
      break;
    }
    RTN_Replace(rtn, (AFUNPTR)PhaseStubReplacement);
    *out << "Phase stub found and instrumented\n";
  } while (0);
  RTN_Close(rtn);
}

VOID Instruction(INS ins, VOID *v) {
  if (INS_IsMemoryRead(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)OnRead, IARG_MEMORYREAD_EA,
                   IARG_MEMORYREAD_SIZE, IARG_END);
  }
  if (INS_IsMemoryWrite(ins)) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)OnWrite, IARG_MEMORYWRITE_EA,
                   IARG_MEMORYWRITE_SIZE, IARG_END);
  }
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v) {
  PhaseStubReplacement("program-finish");
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet
 * started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments,
 *                              including pin -t <toolname> -- ...
 */
int main(int argc, char *argv[]) {
  PIN_InitSymbols();
  // Initialize PIN library. Print help message if -h(elp) is specified
  // in the command line or the command line is invalid
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  accessState.phaseTimes.reserve(128);
  accessState.phaseTimes.push_back(0);

  std::string fileName = KnobOutputFile.Value();

  if (!fileName.empty()) {
    out = new std::ofstream(fileName.c_str());
  }

  RTN_AddInstrumentFunction(Routine, NULL);

  INS_AddInstrumentFunction(Instruction, NULL);

  // Register function to be called when the application exits
  PIN_AddFiniFunction(Fini, 0);

  cerr << "===============================================" << endl;
  cerr << "This application is instrumented by NearMAP" << endl;
  if (!KnobOutputFile.Value().empty()) {
    cerr << "See file " << KnobOutputFile.Value() << " for analysis results"
         << endl;
  }
  cerr << "===============================================" << endl;

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
