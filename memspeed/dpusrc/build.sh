#!/bin/sh

dpu-upmem-dpurte-clang -DNR_TASKLETS=1 -DSTACK_SIZE_DEFAULT=16 -O2 -o ../dpubin/cpucopy.dpuelf cpucopy.c
dpu-upmem-dpurte-clang -DNR_TASKLETS=4 -DSTACK_SIZE_DEFAULT=64 -O2 -S -o ../dpubin/wramcopy.s wramcopy.c

for tasklets in 1 2 4 8 12 16 24
do
    dpu-upmem-dpurte-clang -DNR_TASKLETS=$tasklets -DSTACK_SIZE_DEFAULT=64 -O2 -o ../dpubin/wramcopy.$tasklets.dpuelf wramcopy.c || exit 1
    dpu-upmem-dpurte-clang -DNR_TASKLETS=$tasklets -DSTACK_SIZE_DEFAULT=64 -O2 -o ../dpubin/mramcopy.$tasklets.dpuelf mramcopy.c || exit 1
    echo $tasklets / 24
done
