#!/bin/sh

seq --format='mnists/set%.0f' 10000 > /tmp/pcadataset
DATASET="@/tmp/pcadataset"

OUTFILE=results-pcakmm-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

FS_ARGS="pcakmm ${DATASET}"
RPS_LIST="$(seq 0.4 0.2 10) $(seq 11 1 30)"
NDP_LIST="0 3 4 6 8 9 12"
TIME_PER=20
FAASMSPEED=faasmspeed
PARALLELISM=4
WARMUP_TIME=4
TIMEOUT=45000
HOST=http://192.168.3.30:8080
MONITOR_HOSTS='luna:8125;kone:8125'
#FAASMSPEED=echo

for NDP in ${NDP_LIST}
do

for RPS in ${RPS_LIST}
do
    echo "*** PCAKMM NDP=${NDP} RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t 180000 -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    sleep 1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 60

done
