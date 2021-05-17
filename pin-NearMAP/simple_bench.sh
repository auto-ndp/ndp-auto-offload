#!/bin/sh
/usr/bin/time ../third-party/pin3.18/pin -t ./obj-intel64/NearMAP.so -- sysbench --threads=1 memory run
