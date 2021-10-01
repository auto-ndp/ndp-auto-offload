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
  static uintptr_t TracePageSizeLog2;
  uint64_t currTime = 0;
  std::unordered_map<uintptr_t, RWTime> pageLastAccesses;
  std::vector<uintptr_t> phaseTimes;
} accessState;

const std::string PhaseStubName = "pinnearmap_phase";
volatile uint64_t InstructionCounter{0};

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o",
                                 "pin_nearmap.log",
                                 "specify file name for NearMAP output");

KNOB<uint32_t> KnobPageSize(KNOB_MODE_WRITEONCE, "pintool", "p", "4096",
                            "page size for tracking");

uint64_t lastReadPage{0}, lastWritePage{0};

INT32 Usage() {
  cerr << KNOB_BASE::StringKnobSummary() << endl;

  return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

void summarizeLastAccesses(uint64_t startTime, uint64_t endTime) {
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
  *out << "summary-ro-rw-wo-tot-insn;" << uniqRO << ';' << uniqRW << ';'
       << uniqWO << ';' << uniqRO + uniqRW + uniqWO << "\n";
  out->flush();
  cerr << "Rtn RO:" << uniqRO << " RW:" << uniqRW << " WO:" << uniqWO
       << " TOT:" << uniqRO + uniqRW + uniqWO << " Insn:" << InstructionCounter
       << "\n";
}

VOID OnRead(ADDRINT addr, UINT32 accessSz) {
  const uintptr_t apage = addr >> AccessState::TracePageSizeLog2;
  const uintptr_t epage =
      (addr + accessSz - 1) >> AccessState::TracePageSizeLog2;
  uint64_t time = accessState.currTime++;
  auto &map = accessState.pageLastAccesses;
  for (uintptr_t page = apage; page <= epage; page++) {
    map[page].lastRead = time;
  }
}

VOID OnWrite(ADDRINT addr, UINT32 accessSz) {
  const uintptr_t apage = addr >> AccessState::TracePageSizeLog2;
  const uintptr_t epage =
      (addr + accessSz - 1) >> AccessState::TracePageSizeLog2;
  uint64_t time = accessState.currTime++;
  auto &map = accessState.pageLastAccesses;
  for (uintptr_t page = apage; page <= epage; page++) {
    map[page].lastWrite = time;
  }
}

VOID CountBbl(UINT32 numInstInBbl) { InstructionCounter += numInstInBbl; }

extern "C" {
void PhaseStubReplacement(const char *name) {
  accessState.currTime++;
  uint64_t currTime = accessState.currTime;
  uint64_t prevTime = accessState.phaseTimes.back();
  *out << "phase;" << (1 << AccessState::TracePageSizeLog2) << ';' << prevTime
       << ';' << currTime << ';' << name << ';' << InstructionCounter << endl;
  cerr << "Starting new phase " << name << ", previous phase: from " << prevTime
       << " to " << currTime << " with insns " << InstructionCounter << endl;
  accessState.phaseTimes.push_back(currTime);
  summarizeLastAccesses(prevTime, currTime);
  InstructionCounter = 0;
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

VOID Trace(TRACE trace, VOID *v) {
  // Visit every basic block in the trace
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    // Insert a call to CountBbl() before every basic bloc, passing the number
    // of instructions
    BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)CountBbl, IARG_UINT32,
                   BBL_NumIns(bbl), IARG_END);
  }
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v) { PhaseStubReplacement("program-finish"); }

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

  uintptr_t pageSize = KnobPageSize.Value();
  AccessState::TracePageSizeLog2 = 0;
  while (pageSize > 1) {
    pageSize <<= 1;
    AccessState::TracePageSizeLog2++;
  }
  cerr << "Using page size: " << (1 << AccessState::TracePageSizeLog2) << endl;

  if (!fileName.empty()) {
    out = new std::ofstream(fileName.c_str());
  }

  TRACE_AddInstrumentFunction(Trace, 0);

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
