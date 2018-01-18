#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

int a1 = 1, b = 2, cond = 3;

void thread1(void *a) {
	cout << "thread 1 started" << endl;
	cout << "thread 1 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 1 has lock a" << endl;
	cout << "thread 1 yielding" << endl;
	thread_yield();
	cout << "thread 1 unlocking on a" << endl;
	thread_unlock(a1);
	cout << "thread 1 done" << endl;
}

void thread2(void *a) {
	cout << "thread 2 started" << endl;
	cout << "thread 2 locking on b" << endl;
	thread_lock(b);
	cout << "thread 2 has lock b" << endl;
	cout << "thread 2 yielding" << endl;
	thread_yield();
	cout << "thread 2 returns" << endl;
	cout << "thread 2 yielding" << endl;
	thread_yield();
	cout << "thread 2 returns" << endl;
	cout << "thread 2 unlocking on b" << endl;
	thread_unlock(b);
	cout << "thread 2 done" << endl;
}

void thread3(void *a) {
	cout << "thread 3 starting" << endl;
	cout << "thread 3 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 3 aquired lock a" << endl;
	cout << "thread 3 attempting to unlock b" << endl;
	thread_unlock(b);
	cout << "thread 3 done" << endl;
}

void thread4(void *a) {
	cout << "thread 4 started" << endl;
	cout << "thread 4 attempting to lock on b" << endl;
	thread_lock(b);
	cout << "thread 4 aquired lock b" << endl;
	cout << "thread 4 done" << endl;
}

void start(void *a) {
	int n;
	cout << "Start thread intialized" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread1, 0);
	thread_create((thread_startfunc_t)thread2, 0);
	thread_create((thread_startfunc_t)thread3, 0);
	thread_create((thread_startfunc_t)thread4, 0);
	cout << "Start thread completed" << endl;
}

int main() {
	if (thread_libinit((thread_startfunc_t)start, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}