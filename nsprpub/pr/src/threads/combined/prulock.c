/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 * 
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "primpl.h"


void _PR_InitLocks(void)
{
	_PR_MD_INIT_LOCKS();
}

/*
** Deal with delayed interrupts/requested reschedule during interrupt
** re-enables.
*/
void _PR_IntsOn(_PRCPU *cpu)
{
    PRUintn missed, pri, i;
    _PRInterruptTable *it;
    PRThread *me;

    PR_ASSERT(cpu);   /* Global threads don't have CPUs */
    PR_ASSERT(_PR_MD_GET_INTSOFF() > 0);
	me = _PR_MD_CURRENT_THREAD();
#if !defined(XP_MAC)
    PR_ASSERT(!(me->flags & _PR_IDLE_THREAD));
#endif

    /*
    ** Process delayed interrupts. This logic is kinda scary because we
    ** need to avoid losing an interrupt (it's ok to delay an interrupt
    ** until later).
    **
    ** There are two missed state words. _pr_ints.where indicates to the
    ** interrupt handler which state word is currently safe for
    ** modification.
    **
    ** This code scans both interrupt state words, using the where flag
    ** to indicate to the interrupt which state word is safe for writing.
    ** If an interrupt comes in during a scan the other word will be
    ** modified. This modification will be noticed during the next
    ** iteration of the loop or during the next call to this routine.
    */
    for (i = 0; i < 2; i++) {
        cpu->where = (1 - i);
        missed = cpu->u.missed[i];
        if (missed != 0) {
            cpu->u.missed[i] = 0;
            for (it = _pr_interruptTable; it->name; it++) {
                if (missed & it->missed_bit) {
#ifndef XP_MAC
                    PR_LOG(_pr_sched_lm, PR_LOG_MIN,
                           ("IntsOn[0]: %s intr", it->name));
#endif
                    (*it->handler)();
                }
            }
        }
    }

    if (cpu->u.missed[3] != 0) {
        _PRCPU *cpu;

		_PR_THREAD_LOCK(me);
        me->state = _PR_RUNNABLE;
        pri = me->priority;

        cpu = me->cpu;
		_PR_RUNQ_LOCK(cpu);
        _PR_ADD_RUNQ(me, cpu, pri);
		_PR_RUNQ_UNLOCK(cpu);
		_PR_THREAD_UNLOCK(me);
        _PR_MD_SWITCH_CONTEXT(me);
    }
}

/*
** Assign an idle lock to the first runnable waiting thread. Skip over
** threads that are trying to be suspended
** Note: Caller must hold _PR_LOCK_LOCK()
*/
PRThread * _PR_AssignLock(PRLock *lock)
{
    PRThread *t = NULL;
    PRThread *me;
    PRCList *q;
    PRThreadPriority pri;

    q = lock->waitQ.next;
    PR_ASSERT(q != &lock->waitQ);
    while (q != &lock->waitQ) {
        /* Assign lock to first waiter */
        t = _PR_THREAD_CONDQ_PTR(lock->waitQ.next);

		/* 
		** We are about to change the thread's state to runnable and for local
		** threads, we are going to assign a cpu to it.  So, protect thread's
		** data structure.
		*/
        _PR_THREAD_LOCK(t);

        if (t->flags & _PR_SUSPENDING) {
            q = q->next;
            _PR_THREAD_UNLOCK(t);
	    	t = NULL;
            continue;
        }

        /* Found a thread to give the lock to */
	    PR_ASSERT(t->state == _PR_LOCK_WAIT);
	    PR_ASSERT(t->wait.lock == lock);
        pri = t->priority;
        t->wait.lock = 0;
        PR_REMOVE_LINK(&t->waitQLinks);         /* take it off lock's waitQ */

		/* Lock inherits the thread's priority */
        lock->priority = pri;
        lock->boostPriority = PR_PRIORITY_LOW;
        lock->owner = t;

        /* Add the granted lock to this owning thread's lock list */
        PR_APPEND_LINK(&lock->links, &t->lockList);

		/*
		** If the new owner of the lock is a native thread, nothing else to do
		** except to wake it up by calling the machine dependent wakeup routine.
		**
		** If the new owner is a local thread, we need to assign it a cpu and
		** put the thread on that cpu's run queue.  There are two cases to take care 
		** of.  If the currently running thread is also a local thread, we just
		** assign our own cpu to that thread and put it on the cpu's run queue.
		** If the the currently running thread is a native thread, we assign the
		** primordial cpu to it (on NT, MD_WAKEUP handles the cpu assignment).  
		*/
		
        if ( !_PR_IS_NATIVE_THREAD(t) ) {

            t->state = _PR_RUNNABLE;

            me = _PR_MD_CURRENT_THREAD();

            _PR_AddThreadToRunQ(me, t);
            _PR_THREAD_UNLOCK(t);
        } else {
            t->state = _PR_RUNNING;
            _PR_THREAD_UNLOCK(t);
        }
        _PR_MD_WAKEUP_WAITER(t);
        break;
    }
    return t;
}

/************************************************************************/


PR_IMPLEMENT(PRLock*) PR_NewLock(void)
{
    PRLock *lock;

    if (!_pr_initialized) _PR_ImplicitInitialization();

    lock = PR_NEWZAP(PRLock);
    if (lock) {
        if (_PR_MD_NEW_LOCK(&lock->ilock) == PR_FAILURE) {
		PR_DELETE(lock);
		return(NULL);
	}
        PR_INIT_CLIST(&lock->links);
        PR_INIT_CLIST(&lock->waitQ);
    }
    return lock;
}

/*
** Destroy the given lock "lock". There is no point in making this race
** free because if some other thread has the pointer to this lock all
** bets are off.
*/
PR_IMPLEMENT(void) PR_DestroyLock(PRLock *lock)
{
    PR_ASSERT(lock->owner == 0);
	_PR_MD_FREE_LOCK(&lock->ilock);
    PR_DELETE(lock);
}

extern PRThread *suspendAllThread;
/*
** Lock the lock.
*/
PR_IMPLEMENT(void) PR_Lock(PRLock *lock)
{
    PRThread *me = _PR_MD_CURRENT_THREAD();
    PRIntn is;
    PRThread *t;
    PRCList *q;

    PR_ASSERT(me != suspendAllThread); 
#if !defined(XP_MAC)
    PR_ASSERT(!(me->flags & _PR_IDLE_THREAD));
#endif
#ifdef _PR_GLOBAL_THREADS_ONLY 
    PR_ASSERT(lock->owner != me);
    _PR_MD_LOCK(&lock->ilock);
    lock->owner = me;
    return;
#else  /* _PR_GLOBAL_THREADS_ONLY */

    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_INTSOFF(is);

    PR_ASSERT(_PR_IS_NATIVE_THREAD(me) || _PR_MD_GET_INTSOFF() != 0);

    _PR_LOCK_LOCK(lock);
    if (lock->owner == 0) {
        /* Just got the lock */
        lock->owner = me;
        lock->priority = me->priority;
		/* Add the granted lock to this owning thread's lock list */
        PR_APPEND_LINK(&lock->links, &me->lockList);
        _PR_LOCK_UNLOCK(lock);
    	if (!_PR_IS_NATIVE_THREAD(me))
        	_PR_FAST_INTSON(is);
        return;
    }

    /* If this thread already owns this lock, then it is a deadlock */
    PR_ASSERT(lock->owner != me);

    PR_ASSERT(_PR_IS_NATIVE_THREAD(me) || _PR_MD_GET_INTSOFF() != 0);

#if 0
    if (me->priority > lock->owner->priority) {
        /*
        ** Give the lock owner a priority boost until we get the
        ** lock. Record the priority we boosted it to.
        */
        lock->boostPriority = me->priority;
        _PR_SetThreadPriority(lock->owner, me->priority);
    }
#endif

    /* 
    Add this thread to the asked for lock's list of waiting threads.  We
    add this thread thread in the right priority order so when the unlock
    occurs, the thread with the higher priority will get the lock.
    */
    q = lock->waitQ.next;
    if (q == &lock->waitQ || _PR_THREAD_CONDQ_PTR(q)->priority ==
      	_PR_THREAD_CONDQ_PTR(lock->waitQ.prev)->priority) {
		/*
		 * If all the threads in the lock waitQ have the same priority,
		 * then avoid scanning the list:  insert the element at the end.
		 */
		q = &lock->waitQ;
    } else {
		/* Sort thread into lock's waitQ at appropriate point */
		/* Now scan the list for where to insert this entry */
		while (q != &lock->waitQ) {
			t = _PR_THREAD_CONDQ_PTR(lock->waitQ.next);
			if (me->priority > t->priority) {
				/* Found a lower priority thread to insert in front of */
				break;
			}
			q = q->next;
		}
	}
    PR_INSERT_BEFORE(&me->waitQLinks, q);

	/* 
	Now grab the threadLock since we are about to change the state.  We have
	to do this since a PR_Suspend or PR_SetThreadPriority type call that takes
	a PRThread* as an argument could be changing the state of this thread from
	a thread running on a different cpu.
	*/

    _PR_THREAD_LOCK(me);
    me->state = _PR_LOCK_WAIT;
    me->wait.lock = lock;
    _PR_THREAD_UNLOCK(me);

    _PR_LOCK_UNLOCK(lock);

    _PR_MD_WAIT(me, PR_INTERVAL_NO_TIMEOUT);

	/* When we are here after the context switch, we better own the lock */
    PR_ASSERT(lock->owner == me);

    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_FAST_INTSON(is);
#endif  /* _PR_GLOBAL_THREADS_ONLY */
}

/*
** Unlock the lock.
*/
PR_IMPLEMENT(PRStatus) PR_Unlock(PRLock *lock)
{
    PRCList *q;
    PRThreadPriority pri, boost;
    PRIntn is;
    PRThread *me = _PR_MD_CURRENT_THREAD();

    PR_ASSERT(lock->owner == me);
    PR_ASSERT(me != suspendAllThread); 
#if !defined(XP_MAC)
    PR_ASSERT(!(me->flags & _PR_IDLE_THREAD));
#endif
    if (lock->owner != me) {
        return PR_FAILURE;
    }

#ifdef _PR_GLOBAL_THREADS_ONLY 
    lock->owner = 0;
    _PR_MD_UNLOCK(&lock->ilock);
    return PR_SUCCESS;
#else  /* _PR_GLOBAL_THREADS_ONLY */

    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_INTSOFF(is);
    _PR_LOCK_LOCK(lock);

	/* Remove the lock from the owning thread's lock list */
    PR_REMOVE_LINK(&lock->links);
    pri = lock->priority;
    boost = lock->boostPriority;
    if (boost > pri) {
        /*
        ** We received a priority boost during the time we held the lock.
        ** We need to figure out what priority to move to by scanning
        ** down our list of lock's that we are still holding and using
        ** the highest boosted priority found.
        */
        q = me->lockList.next;
        while (q != &me->lockList) {
            PRLock *ll = _PR_LOCK_PTR(q);
            if (ll->boostPriority > pri) {
                pri = ll->boostPriority;
            }
            q = q->next;
        }
        if (pri != me->priority) {
            _PR_SetThreadPriority(me, pri);
        }
    }

    /* Assign the lock to the first waiting thread */
    q = lock->waitQ.next;
    if (q == &lock->waitQ) {
        /* Nobody wants the lock right now */
        lock->boostPriority = PR_PRIORITY_LOW;
        lock->owner = 0;
    } else {
      if (_PR_AssignLock(lock) == NULL) {
	/* no eligible thread to assign to */
        lock->boostPriority = PR_PRIORITY_LOW;
        lock->owner = 0;
      }
    }
    _PR_LOCK_UNLOCK(lock);
    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_INTSON(is);
    return PR_SUCCESS;
#endif  /* _PR_GLOBAL_THREADS_ONLY */
}

/*
** Test and then lock the lock if it's not already locked by some other
** thread. Return PR_FALSE if some other thread owned the lock at the
** time of the call.
*/
PR_IMPLEMENT(PRBool) PR_TestAndLock(PRLock *lock)
{
    PRThread *me = _PR_MD_CURRENT_THREAD();
    PRBool rv = PR_FALSE;
    PRIntn is;

#ifdef _PR_GLOBAL_THREADS_ONLY 
    is = _PR_MD_TEST_AND_LOCK(&lock->ilock);
    if (is == 0) {
        lock->owner = me;
        return PR_TRUE;
    }
    return PR_FALSE;
#else  /* _PR_GLOBAL_THREADS_ONLY */

    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_INTSOFF(is);

    _PR_LOCK_LOCK(lock);
    if (lock->owner == 0) {
        /* Just got the lock */
        lock->owner = me;
        lock->priority = me->priority;
		/* Add the granted lock to this owning thread's lock list */
        PR_APPEND_LINK(&lock->links, &me->lockList);
        rv = PR_TRUE;
    }
    _PR_LOCK_UNLOCK(lock);

    if (!_PR_IS_NATIVE_THREAD(me))
    	_PR_INTSON(is);
    return rv;
#endif  /* _PR_GLOBAL_THREADS_ONLY */
}

/************************************************************************/
/************************************************************************/
/***********************ROUTINES FOR DCE EMULATION***********************/
/************************************************************************/
/************************************************************************/
PR_IMPLEMENT(PRStatus) PRP_TryLock(PRLock *lock)
    { return (PR_TestAndLock(lock)) ? PR_SUCCESS : PR_FAILURE; }
