#!/bin/bash

>startstop.log
 
OPP="--vmlinux=/boot/`uname -r`/vmlinux --ctr0-count=20000 --ctr0-event=CPU_CLK_UNHALTED"
while :; do
	echo "starting" >>startstop.log
	op_start $OPP >> startstop.log
	rmmod oprofile >>startstop.log 2>&1
	cat /var/lib/oprofile/oprofiled.log >>startstop.log
	rmmod oprofile >>startstop.log 2>&1
	echo "stopping" >> startstop.log
	op_stop >> startstop.log
	rmmod oprofile >>startstop.log 2>&1
	sleep 3
	echo "starting races" >> startstop.log
	op_start $OPP >>startstop.log &
	rmmod oprofile >>startstop.log 2>&1
	sleep $(( $RANDOM / 1000 ))
	rmmod oprofile >>startstop.log 2>&1
	op_dump >> startstop.log &
	cat /var/lib/oprofile/oprofiled.log >>startstop.log
	rmmod oprofile >>startstop.log 2>&1
	sleep $(( $RANDOM / 1000 ))
	op_stop >>startstop.log &
	rmmod oprofile >>startstop.log 2>&1
done
