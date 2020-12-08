#!/bin/sh

dpu-upmem-dpurte-clang -DNR_TASKLETS=1 -DSTACK_SIZE_DEFAULT=16 -O2 -o cpucopy.dpuelf cpucopy.c

dpu-upmem-dpurte-clang -DNR_TASKLETS=4 -DSTACK_SIZE_DEFAULT=256 -O2 -o ramcopy.dpuelf ramcopy.c

dpu-upmem-dpurte-clang -DNR_TASKLETS=4 -DSTACK_SIZE_DEFAULT=256 -O2 -S -o ramcopy.s ramcopy.c
