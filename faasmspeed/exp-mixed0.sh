#!/bin/bash

seq --format='wiki4m/frag_%06.0f' 0 20274 > /tmp/wc_dataset
WC_DATASET="@/tmp/wc_dataset"
seq --format='wiki4m/frag_%06.0f User' 0 20274 > /tmp/substr_dataset
SUB_DATASET="@/tmp/substr_dataset"
PNG_DATASET="@$(pwd)/logoset.txt"
seq --format='mnists/set%.0f' 10000 > /tmp/pcadataset
PCA_DATASET="@/tmp/pcadataset"
GET_DATASET="@/tmp/wc_dataset"

# pg:  lat    rps-mul
#  wc: 150ms  19x
# sub: 20ms   140x
# thm: 15ms   187x
# pca: 2800ms 1x

# sum: 11200ms
# total cpu: 38000ms / s
# rps up to 4

OUTFILE=results-mixed0-$(date -I).log

echo Writing results to $OUTFILE
printf "" > $OUTFILE

FS_ARGS="-f wordcount -f substr -f thumbnailer_decode -f pcakmm -f simple_get ${WC_DATASET} ${SUB_DATASET} ${PNG_DATASET} ${PCA_DATASET} ${GET_DATASET}"
RPS_LIST="0.05 $(seq 0.1 0.1 4)"
NDP_LIST="0:0:0:0:0 6:9:0:6:0 12:12:12:12:0 12:0:12:12:0"
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

NDPA=$(echo $NDP | tr ':' ' ')

for RPS in ${RPS_LIST}
do
    RPSA="-r $(bc <<< $RPS*19) -r $(bc <<< $RPS*140) -r $(bc <<< $RPS*187) -r $(bc <<< $RPS*1) -r 10"
    echo "*** Mixed NDP=${NDP} RPS=${RPSA}"
    # Warm-up burst
    ${FAASMSPEED} -h ${HOST} -u ndp ${NDPA} -N 12 ${FS_ARGS} -c -x ${TIMEOUT} -t 180000 ${RPSA} -p ${PARALLELISM} -o > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp ${NDPA} -N 12 ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} ${RPSA} -p ${PARALLELISM} > /dev/null 2>&1
    ${FAASMSPEED} -h ${HOST} -u ndp ${NDPA} -N 12 ${FS_ARGS} -c -x ${TIMEOUT} -t ${WARMUP_TIME} ${RPSA} -p ${PARALLELISM} -o > /dev/null 2>&1
    sleep 1
    ${FAASMSPEED} -h ${HOST} -u ndp ${NDPA} -N 12 ${FS_ARGS} -c -m ${MONITOR_HOSTS} -x ${TIMEOUT} -t ${TIME_PER} ${RPSA} -p ${PARALLELISM} >> $OUTFILE
    sleep 3
done
sleep 60

done
