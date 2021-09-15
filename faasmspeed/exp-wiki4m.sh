#!/bin/sh

OUTFILE=wresults-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

RPS_LIST="5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100 105 110 115 120 125 130 135 140"
TIME_PER=10
FAASMSPEED=faasmspeed
PARALLELISM=1
WARMUP_TIME=2
TIMEOUT=45000
HOST=http://127.0.0.1:8080
MONITOR_HOSTS='localhost:8125;luna:8125'
#FAASMSPEED=echo
DATASET=$(seq --format='wiki4m/%03.0f' 0 255)

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -f wordcount -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} ${DATASET} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -h ${HOST} -u ndp -f wordcount -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} ${DATASET} >> $OUTFILE
done
sleep 5

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount-NDP RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -f wordcount_manual_ndp -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} ${DATASET} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -h ${HOST} -u ndp -f wordcount_manual_ndp -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} ${DATASET} >> $OUTFILE
done
