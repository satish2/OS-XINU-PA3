/*
 * lock.h
 *
 *  Created on: Apr 21, 2015
 *      Author: Satish
 */

#ifndef H_LOCK_H_
#define H_LOCK_H_

#include "proc.h"

#ifndef	NLOCKS				/* set the number of locks	*/
#define NLOCKS 50
#endif

/* Constants for lock state */
#define	LAVAILABLE	1		/* this lock is available */

/* Constants for lock type */
#define READ   '\01'       /* locked by a reader */
#define WRITE  '\02'		/* locked by a writer */
#define LNONE	'\03' 		/* locked by no process */
#define	isbadlock(l)	(l<0 || l>=NLOCKS)


struct lentry{
	char lstate;  /* lock state is LFREE or LUSED */
	char ltype;   /* lock type is LREAD or LWRITE or LNONE */
	int nreaders; /* to keep a track of total number of readers using the lock */
	int lqhead;   /* Head of the waiting queue */
	int lqtail;   /* Tail of the waiting queue */
	int holders[NPROC];
};

extern struct lentry ltable[NLOCKS];

void linit();
int lcreate();
int ldelete (int lockdescriptor);
int releaseall (int, long, ...);
int lock (int ldes1, int type, int priority);

#endif /* H_LOCK_H_ */
