/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid) {
	STATWORD ps;
	struct pentry *pptr; /* points to proc. table for pid*/
	int i, dev;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return (SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (!isbaddev(dev))
		close(dev);
	dev = pptr->pdevs[1];
	if (!isbaddev(dev))
		close(dev);
	dev = pptr->ppagedev;
	if (!isbaddev(dev))
		close(dev);

	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:
		for (i = 0; i < NLOCKS; i++) {
			if (proctab[pid].locktype[i] != LNONE)
				release(pid,i);
		}
		pptr->pstate = PRFREE; /* suicide */
		resched();

	case PRWAIT:
		semaph[pptr->psem].semcnt++;

	case PRREADY:
		for (i = 0; i < NLOCKS; i++) {
			if (proctab[pid].locktype[i] != LNONE)
				release(pid,i);
		}
		dequeue(pid);
		pptr->pstate = PRFREE;
		break;

	case PRSLEEP:
	case PRTRECV:
		unsleep(pid);
		/* fall through	*/
	default:
		pptr->pstate = PRFREE;
		for (i = 0; i < NLOCKS; i++) {
			if (proctab[pid].locktype[i] != LNONE)
				release(pid,i);
		}
	}
	restore(ps);
	return (OK);
}
