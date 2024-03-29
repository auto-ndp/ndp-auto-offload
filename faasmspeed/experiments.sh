#!/bin/sh

OUTFILE=results-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

FS_ARGS="wordcount frankenmod.txt"
RPS_LIST="10 20 30 40 50 60 70 80 90 100 120 140 160 180 200 250 300 310 320 330 340 350 360 370 380 390 400 410 420 430 440 450 460 470 480 490 500 520 540 560 600 700 800 900 1000 1100 1200 1300 1400 1500"
NDP_LIST="0 1 2 3 4"
TIME_PER=10
FAASMSPEED=faasmspeed
PARALLELISM=2
WARMUP_TIME=2
TIMEOUT=45000
HOST=http://127.0.0.1:8080
MONITOR_HOSTS='localhost:8125;luna:8125'
#FAASMSPEED=echo

for NDP in ${NDP_LIST}
do

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount NDP=${NDP} RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 4 -f ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} -r ${RPS} -p ${PARALLELISM} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -h ${HOST} -u ndp -n ${NDP} -N 4 -f ${FS_ARGS} -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 35

done
