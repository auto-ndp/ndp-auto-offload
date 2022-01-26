#!/bin/sh


DATASET="@$(pwd)/logoset.txt"

OUTFILE=sresults-thumb1-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

FS_ARGS="thumbnailer_decode ${DATASET}"
RPS_LIST="$(seq 10 10 90) $(seq 100 25 500) $(seq 600 100 2000) $(seq 2200 200 3000)"
NDP_LIST="24"
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
    echo "*** Thumb_decode NDP=${NDP} RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t 180000 -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} -o > /dev/null 2>&1
    sleep 1
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 12 -f ${FS_ARGS} -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 60

done
