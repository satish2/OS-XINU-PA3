/*
 * lock.c

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

extern unsigned long ctr1000;
int has_highprio_writer(int prio, int ldesc);

int lock(int ldes1, int type, int priority) {
	STATWORD ps;
	struct pentry *pptr;
	disable(ps);

	if (isbadlock(ldes1) || ltable[ldes1].lstate == DELETED || proctab[currpid].locktype[ldes1] != LNONE) {
		restore(ps);
		return (SYSERR);
	}

	proctab[currpid].plwaitret = OK;

	if (ltable[ldes1].ltype == LNONE) {
		ltable[ldes1].ltype = type;
		proctab[currpid].locktype[ldes1] = type;
		ltable[ldes1].holders[currpid] = type;

		if (type == READ) {
			ltable[ldes1].nreaders++;
		}
	} else if (ltable[ldes1].ltype == WRITE) {
		pptr = &proctab[currpid];
		pptr->pstate = PRWAIT;
		pptr->locktype[ldes1] = type;
		pptr->plreqtime = ctr1000;
		insert(currpid, ltable[ldes1].lqhead, priority);
		resched();
	} else if (ltable[ldes1].ltype == READ) {
		if (type == WRITE) {
			pptr = &proctab[currpid];
			pptr->pstate = PRWAIT;
			pptr->locktype[ldes1] = type;
			pptr->plreqtime = ctr1000;
			insert(currpid, ltable[ldes1].lqhead, priority);
			resched();
		} else if (type == READ) {
			if (has_highprio_writer(priority, ldes1) == TRUE) {
				pptr = &proctab[currpid];
				pptr->pstate = PRWAIT;
				pptr->locktype[ldes1] = type;
				pptr->plreqtime = ctr1000;
				insert(currpid, ltable[ldes1].lqhead, priority);
				resched();
			} else {
				ltable[ldes1].ltype = type;
				ltable[ldes1].nreaders++;
				proctab[currpid].locktype[ldes1] = type;
				ltable[ldes1].holders[currpid] = type;
			}
		}
	}
	restore(ps);
	return (proctab[currpid].plwaitret);
}

int has_highprio_writer(int prio, int ldesc) {
	int temp;
	temp = q[ltable[ldesc].lqtail].qprev;

	for (temp = q[ltable[ldesc].lqtail].qprev; (temp != ltable[ldesc].lqhead) && (prio < q[temp].qkey); temp = q[temp].qprev) {
		if (proctab[temp].locktype[ldesc] == WRITE) {
			return TRUE;
		}
	}
	return FALSE;
}

