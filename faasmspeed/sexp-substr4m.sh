#!/bin/sh

seq --format='wiki4m/frag_%06.0f User' 0 20274 > /tmp/substr_dataset
DATASET="@/tmp/substr_dataset"

OUTFILE=sresults-substr4m-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

FS_ARGS="substr ${DATASET}"
RPS_LIST="$(seq 10 10 90) $(seq 100 25 600) $(seq 700 100 1000)"
NDP_LIST=24
TIME_PER=10
FAASMSPEED=faasmspeed
PARALLELISM=2
WARMUP_TIME=2
TIMEOUT=45000
HOST=http://192.168.3.10:8080
MONITOR_HOSTS='luna:8125;kone:8125'
#FAASMSPEED=echo

for NDP in ${NDP_LIST}
do

for RPS in ${RPS_LIST}
do
    echo "*** Substr NDP=${NDP} RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t 180000 -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    sleep 1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 60

done
