#!/bin/bash

SCRIPT_PATH="`dirname \"$0\"`"
SCRIPT_PATH="`( cd \"$SCRIPT_PATH\" && pwd )`"
if [ -z "$SCRIPT_PATH" ] ; then
  echo "Can't determine script path"
  exit 1  # fail
fi

PIN_ROOT="$SCRIPT_PATH/../third-party/pin3.18"
PIN_ROOT="`( cd \"$PIN_ROOT\" && pwd )`"
if [ -z "$PIN_ROOT" ] ; then
  echo "Can't determine Pin root path"
  exit 1  # fail
fi
export PIN_ROOT

SPEC_ROOT="$SCRIPT_PATH/../third-party/spec2017"
SPEC_ROOT="`( cd \"$SPEC_ROOT\" && pwd )`"
if [ -z "$SPEC_ROOT" ] ; then
  echo "Can't determine SPEC root path"
  exit 1  # fail
fi
export SPEC_ROOT

BENCHMARKS="502.gcc_r 505.mcf_r 507.cactuBSSN_r 508.namd_r 510.parest_r 511.povray_r 519.lbm_r 520.omnetpp_r"
RUNCPU_CONFIG='--config=kubasz-gcc-amd64.cfg'

cd $SCRIPT_PATH
echo '===========' Compiling pintool
make tools

cd $SPEC_ROOT
source shrc

echo '===========' Compiling benchmarks
runcpu $RUNCPU_CONFIG --action=build $BENCHMARKS
