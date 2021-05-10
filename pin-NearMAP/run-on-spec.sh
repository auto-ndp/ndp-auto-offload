#!/bin/bash

SCRIPT_PATH="`dirname \"$0\"`"
SCRIPT_PATH="`( cd \"$SCRIPT_PATH\" && pwd )`"
if [[ -z "$SCRIPT_PATH" ]] ; then
  echo "Can't determine script path"
  exit 1
fi

PIN_ROOT="$SCRIPT_PATH/../third-party/pin3.18"
PIN_ROOT="`( cd \"$PIN_ROOT\" && pwd )`"
if [[ -z "$PIN_ROOT" ]] ; then
  echo "Can't determine Pin root path"
  exit 1
fi
export PIN_ROOT

SPEC_ROOT="$SCRIPT_PATH/../third-party/spec2017"
SPEC_ROOT="`( cd \"$SPEC_ROOT\" && pwd )`"
if [[ -z "$SPEC_ROOT" ]] ; then
  echo "Can't determine SPEC root path"
  exit 1
fi
export SPEC_ROOT

BENCHMARKS=(
  '502.gcc_r'
  '505.mcf_r'
  '507.cactuBSSN_r'
  '508.namd_r'
  '510.parest_r'
  '511.povray_r'
  '519.lbm_r'
  '520.omnetpp_r'
)
RUNCPU_CONFIG='--config=kubasz-gcc-amd64.cfg'

RESULT_PATH=$SCRIPT_PATH/specrunresults
mkdir -p $RESULT_PATH

cd $SCRIPT_PATH
echo '===========' Compiling pintool
make tools

cd $SPEC_ROOT
source shrc

# Set strict options after sourcing spec rc
set -euo pipefail
IFS=$'\n\t'

PIN_EXE=$PIN_ROOT/pin
if [[ ! -f "$PIN_EXE" ]]; then
  echo "Can't determine pin executable file location"
  exit 1
fi

NEARMAP_SO=$SCRIPT_PATH/obj-intel64/NearMAP.so
if [[ ! -f "$NEARMAP_SO" ]]; then
  echo "Can't determine NearMAP.so file location"
  exit 1
fi

echo '===========' Setting up run directories
runcpu $RUNCPU_CONFIG --action=setup "${BENCHMARKS[@]}"

echo '===========' Validating run directories

for bench in "${BENCHMARKS[@]}"; do
  benchdir=$SPEC_ROOT/benchspec/CPU/$bench/run
  cd $benchdir
  rundir=$benchdir/$(find . -maxdepth 1 -type d | sort -n | tail -1)
  cmdfile=$rundir/speccmds.cmd
  if [[ ! -f "$cmdfile" ]]; then
    echo "Can't find valid run commands for benchmark '$bench' at '$cmdfile'"
    exit 1
  fi
  echo "== Found valid cmdfile at '$cmdfile'"
done

echo '===========' Running benchmarks

for bench in "${BENCHMARKS[@]}"; do
  benchdir=$SPEC_ROOT/benchspec/CPU/$bench/run
  cd $benchdir
  rundir=$benchdir/$(find . -maxdepth 1 -type d | sort -n | tail -1)
  cmdfile=$rundir/speccmds.cmd
  if [[ ! -f "$cmdfile" ]]; then
    echo "Can't find valid run commands for benchmark '$bench' at '$cmdfile'"
    exit 1
  fi
  echo "== Running $bench"
  cd $rundir
  rcwd=$(grep -E '^-C .+$' $cmdfile | cut -c '4-')
  let cmdi=1
  for rcmd in $(grep -E '^-o .+$' speccmds.cmd | cut -d' ' -f '5-' | grep -Eo '^[^>]+'); do
    IFS=$' \t\n' read -ra racmd <<< "${rcmd}"
    echo "= Found cmd: '${racmd[@]}'"
    echo "= Running with pin"
    rresf=${RESULT_PATH}/${bench}_c${cmdi}.log
    echo > ${rresf} # erase old results
    echo "= Saving results to '$rresf'"
    echo "= Executing"
    "${PIN_EXE}" -t "${NEARMAP_SO}" -o "${rresf}" -- "${racmd[@]}" </dev/null >${rresf}.stdout 2>${rresf}.stderr || (echo "Failed with error code $?" ; exit 1)
    echo "= Done"
    ((cmdi++))
  done
done
