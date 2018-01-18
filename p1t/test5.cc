#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

int lock = 1;

void thread1(void *a) {
	cout << "thread 1 started" << endl;
	cout << "thread 1 locking" << endl;
	thread_lock(lock);
	cout << "thread 1 has lock" << endl;
	cout << "thread 1 yields" << endl;
	thread_yield();
	cout << "thread 1 returns" << endl;
	cout << "thread 1 unlocking" << endl;
	thread_unlock(lock);
	cout << "thread 1 done" << endl;
}

void thread2(void *a) {
	cout << "thread 2 started" << endl;
	thread_lock(lock);
	cout << "thread 2 gets the lock" << endl;
	cout << "thread 2 unlocking" << endl;
	thread_unlock(lock);
	cout << "thread 2 done" << endl;
}

void thread3(void *a) {
	cout << "thread 3 started" << endl;
	thread_lock(lock);
	cout << "thread 3 has the lock" << endl;
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
