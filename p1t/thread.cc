#include "thread.h"
#include "interrupt.h"
#include <iostream>
#include <ucontext.h>
#include <stdlib.h>

using namespace std;



struct thread {
	ucontext_t *context;
	int active; // 1 if active, 0 if not
	thread *next; // pointer to the next 
};

struct Lock {
	unsigned int key;
	int free;
	thread *queue;
	thread *tail;
	Lock *next;
	thread *holder;
};

struct sleepQ {
	unsigned int lock;
	unsigned int cond;
	thread *queue;
	thread *tail;
	sleepQ *next;
};

static thread* readyQueue;		//pointers to the head of the readylist and waitlist
static thread* readyTail;
static thread* waitList;
static thread* activeThread;

static Lock *firstLock;
static sleepQ* firstSleepQ;

/* Main library thread*/
static ucontext_t* libContext;

/* bool to track if currently inside a thread */
int inThread = 0;

static void	threadStart(void(*func)(void*), void *arg);
void nextThread();
void nextThreadFree();

static void Exit() {
	cout << "Thread library exiting.\n";
	exit(0);
}

extern int thread_libinit(thread_startfunc_t func, void *arg) {
	int n;
	interrupt_disable();

	/*
	* Initialize a context structure by copying the current thread's context.
	*/

	ucontext_t* ucontext_ptr = new ucontext_t;
	libContext = new ucontext_t;

	getcontext(ucontext_ptr); // ucontext_ptr has type (ucontext_t *)

	/*
	* Direct the new thread to use a different stack.
	* Allocate STACK_SIZE bytes for each thread's stack.
	*/
	char *stack = new char[STACK_SIZE];

	ucontext_ptr->uc_stack.ss_sp = stack;
	ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
	ucontext_ptr->uc_stack.ss_flags = 0;
	ucontext_ptr->uc_link = NULL;

	makecontext(ucontext_ptr, (void(*)()) &threadStart, 2, func, arg);

	activeThread = new thread;
	activeThread->context = ucontext_ptr;

	interrupt_enable();

	swapcontext(libContext, ucontext_ptr);		//swap to the new thread

	while (1) {
		free(activeThread->context->uc_stack.ss_sp);
		free(activeThread->context);
		nextThreadFree();
	}


}
extern int thread_create(thread_startfunc_t func, void *arg) {

	/*
	* Initialize a context structure by copying the current thread's context.
	*/
	ucontext_t *newContext = new ucontext_t;

	getcontext(newContext); // ucontext_ptr has type (ucontext_t *)

							  /*
							  * Direct the new thread to use a different stack.
							  * Allocate STACK_SIZE bytes for each thread's stack.
							  */
	char *stack = new char[STACK_SIZE];
	newContext->uc_stack.ss_sp = stack;
	newContext->uc_stack.ss_size = STACK_SIZE;
	newContext->uc_stack.ss_flags = 0;
	newContext->uc_link = NULL;
	makecontext(newContext, (void(*)()) &threadStart, 2, func, arg);

	thread *newThread = new thread;
	newThread->active = 0;
	newThread->next = NULL;
	newThread->context = newContext;

	//add the new thread to the readylist
	//check if the queue is empty first
	if (readyQueue == NULL) {
		readyQueue = newThread;
		readyTail = newThread;
	}
	else	//otherwise add it to the end
	{
		readyTail->next = newThread;
		readyTail = newThread;
	}

}
extern int thread_yield(void) {
	int n;

	interrupt_disable();
	//if ready queue is null, all threads are either complete or deadlocked
	if (readyQueue != NULL) {
		// if the thread is still active, place it at the end of the ready queue
		int active = activeThread->active;

		if (active) {
			readyTail->next = activeThread;
			readyTail = activeThread;

			//get the next thread from the ready queue
			nextThread();
			return 0;
		}
		// otherwise free it
		else {
			setcontext(libContext);
		}
	}
	// if the ready queue is empty, check if the current thread is still running, then return
	else {
		if (activeThread->active) {
			interrupt_enable();
			return 0;
		}
		else		// thread is no longer running, clean up and exit
		{
			Exit();
		}
	}
	interrupt_enable();
	return 0;
}

static void	threadStart(void(*func)(void*), void *arg) {
	int n;
	interrupt_disable();
	activeThread->active = 1;
	interrupt_enable();

	(*func)(arg);

	interrupt_disable();
	activeThread->active = 0;
	interrupt_enable();
	thread_yield();
}

Lock* getLock(unsigned int lock) {
	Lock *temp = firstLock;
	while (temp != NULL) {
		if (temp->key == lock) return temp;
		temp = temp->next;
	}
	return NULL;
}

extern int thread_lock(unsigned int lock) {
	int n;
	interrupt_disable();
	//attempt to find the correct lock
	Lock *temp = getLock(lock);
	if (temp != NULL) {
		if (temp->free == 1) {			//if the lock is free, give it to this thread
			temp->free = 0;
			temp->holder = activeThread;
			interrupt_enable();
			return 0;
		}
		else					//otherwise add to lock queue and switch to next thread
		{
			if (temp->tail != NULL) {				//add to the end of the lock queue
				temp->tail->next = activeThread;
			}
			else						//unless the queue is empty, start a new lock queue
			{
				temp->queue = activeThread;
				temp->tail = activeThread;
			}
			//once added to queue, switch to next ready process
			nextThread();
			return 0;
		}
	}
	//lock doesn't exist, create it
	else {
		Lock* newLock = new Lock;
		newLock->key = lock;
		newLock->free = 0;
		newLock->holder = activeThread;
		newLock->queue = NULL;
		newLock->tail = NULL;
		//add new lock to the list of locks
		if (firstLock == NULL) {
			firstLock = newLock;
		}
		else {
			Lock* temp = firstLock;
			while (temp->next != NULL) {
				temp = temp->next;
			}
			temp->next = newLock;
		}
		interrupt_enable();
		return 0;
	}
	interrupt_enable();
	return -1;
}

extern int thread_unlock(unsigned int lock) {
	interrupt_disable();
	Lock* temp = getLock(lock);
	if (temp != NULL && temp->holder == activeThread) {
		temp->free = 1;
		temp->holder = NULL;
		if (temp->queue != NULL) {
			temp->free = 0;
			temp->holder = temp->queue;
			temp->queue = temp->queue->next;
			if (temp->queue == NULL) { temp->tail = NULL; }
			//if readyqueue has threads, add the waiting thread to the end of the ready queue
			if (readyQueue != NULL) {
				readyTail->next = temp->holder;
				readyTail = temp->holder;
			}
			else
			{
				readyQueue = temp->holder;
				readyTail = temp->holder;
			}
		}
		interrupt_enable();
		return 0;
	}
	interrupt_enable();
	return -1;
}

sleepQ* getSleepQ(unsigned int lock, unsigned int cond) {
	sleepQ *temp = firstSleepQ;
	while (temp != NULL) {
		if (temp->lock == lock && temp->cond == cond) return temp;
		temp = temp->next;
	}
	return NULL;
}

extern int thread_wait(unsigned int lock, unsigned int cond) {
	interrupt_disable();
	//make sure that this thread holds the lock
	Lock* l = getLock(lock);
	if (l != NULL && l->holder != activeThread) {
		interrupt_enable();
		return -1;
	}
	interrupt_enable();
	thread_unlock(lock);
	interrupt_disable();
	sleepQ* temp = getSleepQ(lock, cond);
	if (temp != NULL) {

		

		if (temp->queue != NULL) {
			temp->tail->next = activeThread;
			temp->tail = activeThread;
		}
		else
		{
			temp->queue = activeThread;
			temp->tail = activeThread;
		}
	}
	else
	{
		temp = new sleepQ;
		temp->lock = lock;
		temp->cond = cond;
		temp->queue = activeThread;
		temp->tail = activeThread;
		temp->next = NULL;
		if (firstSleepQ == NULL) { firstSleepQ = temp; }
		else
		{
			sleepQ *s = firstSleepQ;
			while (s->next != NULL) {
				s = s->next;
			}
			s->next = temp;
		}
	}
	nextThread();
	thread_lock(lock);
	return 0;
}
extern int thread_signal(unsigned int lock, unsigned int cond) {
	interrupt_disable();
	sleepQ* temp = getSleepQ(lock, cond);
	if (temp != NULL && temp->queue != NULL)
	{
		if (readyQueue != NULL) {
			readyTail->next = temp->queue;
			readyTail = temp->queue;
		}
		else {
			readyQueue = temp->queue;
			readyTail = temp->queue;
		}
		temp->queue = temp->queue->next;
		readyTail->next = NULL;
		if (temp->queue == NULL) { temp->tail = NULL; }
	}
	interrupt_enable();
	return 0;
}
extern int thread_broadcast(unsigned int lock, unsigned int cond) {
	interrupt_disable();
	sleepQ* temp = getSleepQ(lock, cond);
	while (temp != NULL && temp->queue != NULL)
	{
		if (readyQueue != NULL) {
			readyTail->next = temp->queue;
			readyTail = temp->queue;
		}
		else {
			readyQueue = temp->queue;
			readyTail = temp->queue;
		}
		temp->queue = temp->queue->next;
		readyTail->next = NULL;
		if (temp->queue == NULL) { temp->tail = NULL; }
	}
	interrupt_enable();
	return 0;
}

void nextThread() {
	if (readyQueue != NULL)
	{
		ucontext_t *current = activeThread->context;
		activeThread = readyQueue;
		readyQueue = readyQueue->next;
		if (readyQueue == NULL) { readyTail = NULL; }
		activeThread->next = NULL;
		interrupt_enable();
		swapcontext(current, activeThread->context);
	}
	else			//if ready queue is empty, exit
	{
		Exit();
	}
}

void nextThreadFree() {
	if (readyQueue != NULL)
	{
		activeThread = readyQueue;
		readyQueue = readyQueue->next;
		if (readyQueue == NULL) { readyTail = NULL; }
		activeThread->next = NULL;
		interrupt_enable();
		setcontext(activeThread->context);
		return;
	}
	else			//if ready queue is empty, exit
	{
		Exit();
	}
}

