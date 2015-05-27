/*
 * linit.c
 *
 *  Created on: Apr 21, 2015
 *      Author: Satish
 */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

struct lentry ltable[NLOCKS];
void linit()
{
	int i,pid;
	struct lentry *lptr;
	for (i=0 ; i<NLOCKS ; i++) {
			(lptr = &ltable[i])->lstate = DELETED;
			lptr->ltype = LNONE;
			lptr->nreaders = 0;
			lptr->lqtail = 1 + (lptr->lqhead= newqueue());
			for(pid=0;pid<NPROC;pid++)
			{
				lptr->holders[pid] = LNONE;
			}
		}
}
