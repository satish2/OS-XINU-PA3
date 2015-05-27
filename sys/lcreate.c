/*
 * lcreate.c
 *
 *  Created on: Apr 21, 2015
 *      Author: Satish
 */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newldes();

int lcreate() {
	STATWORD ps;
	int ldes;
	disable(ps);
	if ((ldes = newldes()) == SYSERR) {
		restore(ps);
		return (SYSERR);
	}
	restore(ps);
	return (ldes);
}

LOCAL int newldes() {
	int i;

	for (i = 0; i < NLOCKS; i++) {
		if (ltable[i].lstate == DELETED)
		{
			ltable[i].lstate = LAVAILABLE;
			ltable[i].ltype = LNONE;
			return i;
		}
	}
	return SYSERR;
}

