/*
 * releaseall.c
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
#include <sleep.h>

int release(int pid, int ldes);
int get_next_process(int ldesc, int *high_prio);
void remote_readers(int ldesc, int pid_, int high_prio);

int releaseall(int numlocks, long locks, ...)
{
	STATWORD ps;
	int ret = OK;
	int ldes;
	int i;
	disable(ps);

	for(i=0;i<numlocks;i++)
	{
		ldes = (int)*((&locks) + i);
		ret = release(currpid, ldes);
	}
	resched();
	restore(ps);
	return ret;
}

int release(int pid, int ldes)
{

	struct lentry *lptr;
	int pid_;
	int proceed = 0,nextpid = 0, max_w_prio = 0;

	lptr = &ltable[ldes];

	if(isbadlock(ldes) || ltable[ldes].lstate == DELETED || lptr->holders[pid] == LNONE || lptr->ltype == LNONE)
	{
		return SYSERR;
	}

	proctab[pid].locktype[ldes] = LNONE;
	ltable[ldes].holders[pid] = LNONE;

	if(lptr->ltype == READ)
	{
		lptr->nreaders--;
		if(lptr->nreaders > 0)
		{
			proceed = 0;
			return OK;
		}
		else
		{
			proceed = 1;
		}
	}
	else if(lptr->ltype == WRITE)
	{
		proceed = 1;
	}
	if(proceed == 1)
	{
		/* */
		nextpid = get_next_process(ldes,&max_w_prio);
		if(nextpid == -1)
		{
			ltable[ldes].ltype = LNONE;
			return OK;
		}
		if(proctab[nextpid].locktype[ldes] == READ)
		{
			pid_ = q[nextpid].qprev;
			dequeue(nextpid);
			ready(nextpid,RESCHNO);
			ltable[ldes].holders[nextpid] = READ;
			ltable[ldes].nreaders++;
			remote_readers(ldes,pid_,max_w_prio);
			ltable[ldes].ltype = READ;
		}
		else
		{
			ltable[ldes].ltype = WRITE;
			dequeue(nextpid);
			ready(nextpid,RESCHNO);
			ltable[ldes].holders[nextpid] = WRITE;
		}
	}
	return OK;
}

void remote_readers(int ldesc, int pid_, int high_prio)
{
	int pid_2;
	int pid = pid_;

	while(pid != ltable[ldesc].lqhead && q[pid].qkey >= high_prio)
	{
		if(proctab[pid].locktype[ldesc]==READ)
		{
			pid_2=q[pid].qprev;
			ltable[ldesc].nreaders++;
			dequeue(pid);
			ready(pid,RESCHNO);
			ltable[ldesc].holders[pid] = READ;
			pid=pid_2;
		}
		else
			pid = q[pid].qprev;
	}
}

int get_next_process(int ldesc, int *high_prio)
{
	if(q[ltable[ldesc].lqtail].qprev == ltable[ldesc].lqhead)
	{
		return SYSERR;
	}

	int pid_1 = q[ltable[ldesc].lqtail].qprev;
	int pid_2;
	int timediff = 0,retVal = 0;

	if(proctab[pid_1].locktype[ldesc] == WRITE)
	{
		retVal = pid_1;
		*high_prio = -1;
	}
	else
	{
		pid_2 = q[pid_1].qprev;
		if(q[pid_1].qkey > q[pid_2].qkey)
		{
			retVal = pid_1;
			*high_prio = -1;
		}
		else if((q[pid_1].qkey == q[pid_2].qkey))
		{
			while((pid_2 != ltable[ldesc].lqhead) && (q[pid_1].qkey == q[pid_2].qkey))
			{
				if(proctab[pid_2].locktype[ldesc] == READ)
					retVal = pid_1;
				else
				{
					timediff = proctab[pid_2].plreqtime - proctab[pid_1].plreqtime;
					if(timediff <= 1000 && timediff >= -1000)
						retVal = pid_2;
					else
						retVal = pid_1;
					break;
				}
				pid_2 = q[pid_2].qprev;
			}
		}
		pid_1 = q[ltable[ldesc].lqtail].qprev;
		{
			while(pid_1!=ltable[ldesc].lqhead && proctab[pid_1].locktype[ldesc]!=WRITE)
				pid_1 = q[pid_1].qprev;
			*high_prio = q[pid_1].qkey;
		}
	}
	return retVal;
}
