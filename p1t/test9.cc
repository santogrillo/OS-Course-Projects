#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

int a1 = 1, b = 2, cond1 = 3, cond2 = 4;

void thread1(void *a) {
	cout << "thread 1 started" << endl;
	cout << "thread 1 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 1 aquired lock a" << endl;
	cout << "thread 1 waiting on lock a, cond1" << endl;
	thread_wait(a1, cond1);
	cout << "thread 1 wakes up" << endl;
	cout << "thread 1 unlocks on a" << endl;
	thread_unlock(a1);
	cout << "thread 1 complete" << endl;
}

void thread2(void *a) {
	cout << "thread 2 started" << endl;
	cout << "thread 2 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 2 aquired lock a" << endl;
	cout << "thread 2 waiting on lock a, cond1" << endl;
	thread_wait(a1, cond1);
	cout << "thread 2 wakes up" << endl;
	cout << "thread 2 unlocks on a" << endl;
	thread_unlock(a1);
	cout << "thread 2 complete" << endl;
}

void thread3(void *a) {
	cout << "thread 3 started" << endl;
	cout << "thread 3 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 3 aquired lock a" << endl;
	cout << "thread 3 waiting on lock a, cond1" << endl;
	thread_wait(a1, cond1);
	cout << "thread 3 wakes up" << endl;
	cout << "thread 3 unlocks on a" << endl;
	thread_unlock(a1);
	cout << "thread 2 complete" << endl;
}

void thread4(void *a) {
	cout << "thread 4 started" << endl;
	cout << "thread 4 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 4 aquired lock a" << endl;
	cout << "thread 4 waiting on lock a, cond1" << endl;
	thread_wait(a1, cond1);
	cout << "thread 4 wakes up" << endl;
	cout << "thread 4 unlocks on a" << endl;
	thread_unlock(a1);
	cout << "thread 4 complete" << endl;
}

void thread5(void *a) {
	cout << "thread 5 starting" << endl;
	cout << "thread 5 attempting to signal on wrong lock" << endl;
	thread_signal(b, cond1);
	cout << "thread 5 yields, should continue" << endl;
	thread_yield();
	cout << "thread 5 attempting to signal on wrong condition" << endl;
	thread_signal(a1, cond2);
	cout << "thread 5 yields, should continue" << endl;
	thread_yield();
	cout << "thread 5 signals on correct lock/condition" << endl;
	thread_signal(a1, cond1);
	cout << "thread 5 yields, should run thread1" << endl;
	thread_yield();
	cout << "thread 5 signals on correct lock/condition" << endl;
	thread_signal(a1, cond1);
	cout << "thread 5 yields, should run thread2" << endl;
	thread_yield();
	cout << "thread 5 broadcasts on correct lock/condition" << endl;
	thread_signal(a1, cond1);
	cout << "thread 5 yields, should run thread3 and thread4" << endl;
	thread_yield();
	cout << "thread 5 complete" << endl;
}

void start(void *a) {
	int n;
	cout << "Start thread intialized" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread1, 0);
	thread_create((thread_startfunc_t)thread2, 0);
	thread_create((thread_startfunc_t)thread3, 0);
	thread_create((thread_startfunc_t)thread4, 0);
	thread_create((thread_startfunc_t)thread5, 0);
	cout << "Start thread completed" << endl;
}

int main() {
	if (thread_libinit((thread_startfunc_t)start, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}