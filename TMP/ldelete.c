/*
 * ldelete.c
 *
 *  Created on: Apr 21, 2015
 *      Author: Satish
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int ldelete(int ldesc) {
	STATWORD ps;
	int pid;
	struct lentry *lptr;

	disable(ps);
	if (isbadlock(ldesc) || ltable[ldesc].lstate == DELETED) {
		restore(ps);
		return (SYSERR);
	}
	lptr = &ltable[ldesc];
	lptr->lstate = DELETED;
	lptr->ltype = LNONE;
	lptr->nreaders = 0;
	if (nonempty(lptr->lqhead)) {
		while ((pid = getfirst(lptr->lqhead)) != EMPTY) {
			proctab[pid].plwaitret = DELETED;
			proctab[pid].locktype[ldesc] = DELETED;
			dequeue(pid);
			ready(pid, RESCHNO);
		}
		resched();
	}

	for (pid = 1; pid < NPROC; pid++) {
		if (ltable[ldesc].holders[pid] == READ
				|| ltable[ldesc].holders[pid] == WRITE) {
			ltable[ldesc].holders[pid] = DELETED;
		}
	}
	restore(ps);
	return (OK);
}

