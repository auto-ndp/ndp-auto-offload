#!/bin/sh

OUTFILE=results-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

RPS_LIST="1 10 20 30 40 50 60 70 80 90 100 110 120 130 140"
TIME_PER=10
FAASMSPEED=faasmspeed
#FAASMSPEED=echo

for RPS in ${RPS_LIST}
do
    echo "*** Hello RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u demo -f hello '' -c -t 1 -r ${RPS} -p 2 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u demo -f hello '' -c -t ${TIME_PER} -r ${RPS} -p 2 >> $OUTFILE
done
sleep 1

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u ndp -f wordcount frankenmod.txt -c -t 1 -r ${RPS} -p 2 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u ndp -f wordcount frankenmod.txt -c -t ${TIME_PER} -r ${RPS} -p 2 >> $OUTFILE
done
sleep 1

for RPS in ${RPS_LIST}
do
    echo "*** Wordcount-NDP RPS=${RPS}"
    # Warm-up burst
    ${FAASMSPEED} -u ndp -f wordcount_manual_ndp frankenmod.txt -c -t 1 -r ${RPS} -p 2 2>&1 > /dev/null
    sleep 0.2
    ${FAASMSPEED} -u ndp -f wordcount_manual_ndp frankenmod.txt -c -t ${TIME_PER} -r ${RPS} -p 2 >> $OUTFILE
done
