#!/bin/sh

OUTFILE=results-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

RPS_LIST="1 10 20 30 40 50 60 70 80 90 100 120 140 160 180 200 250 300 400 500 600 700 800 900 1000"
TIME_PER=10
FAASMSPEED=faasmspeed
PARALLELISM=1
TIMEOUT=25000
#FAASMSPEED=echo

for RPS in ${RPS_LIST}
do
    echo "*** Hello RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u demo -f hello '' -c -x ${TIMEOUT} -t 1 -r ${RPS} -p ${PARALLELISM} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u demo -f hello '' -c -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 1

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u ndp -f wordcount frankenmod.txt -c -x ${TIMEOUT} -t 1 -r ${RPS} -p ${PARALLELISM} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u ndp -f wordcount frankenmod.txt -c -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
sleep 1

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount-NDP RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u ndp -f wordcount_manual_ndp frankenmod.txt -c -x ${TIMEOUT} -t 1 -r ${RPS} -p ${PARALLELISM} 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u ndp -f wordcount_manual_ndp frankenmod.txt -c -x ${TIMEOUT} -t ${TIME_PER} -r ${RPS} -p ${PARALLELISM} >> $OUTFILE
done
