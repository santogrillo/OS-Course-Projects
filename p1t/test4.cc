#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

int lock = 1, cond = 2;

void thread1(void *a) {
	cout << "thread 1 started" << endl;
	thread_lock(lock);
	cout << "thread 1 wait" << endl;
	thread_wait(lock, cond);
	cout << "thread 1 awake" << endl;
	thread_unlock(lock);
	cout << "thread 1 done" << endl;
}

void thread2(void *a) {
	cout << "thread 2 started" << endl;
	thread_lock(lock);
	cout << "thread 2 yield" << endl;
	thread_yield();
	cout << "thread 2 returns" << endl;
	thread_unlock(lock);
	cout << "thread 2 done" << endl;
}

void thread3(void *a) {
	cout << "thread 3 started" << endl;
	cout << "thread 3 signals" << endl;
	thread_signal(lock, cond);
	cout << "thread 3 locking" << endl;
	thread_lock(lock);
	cout << "thread 3 gets lock" << endl;
	thread_unlock(lock);
	cout << "thread 3 done" << endl;
}

void start(void *a) {
	int n;
	cout << "Start thread intialized" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread1, 0);
	thread_create((thread_startfunc_t)thread2, 0);
	thread_create((thread_startfunc_t)thread3, 0);
	cout << "Start thread completed" << endl;
}

int main() {
	if (thread_libinit((thread_startfunc_t)start, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}
