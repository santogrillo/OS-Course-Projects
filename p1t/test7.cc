#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

int a1 = 1, b = 2;

void thread1(void *a) {
	cout << "thread 1 started" << endl;
	cout << "thread 1 locking on a" << endl;
	thread_lock(a1);
	cout << "thread 1 aquires lock a" << endl;
	cout << "thread 1 yielding" << endl;
	thread_yield();
	cout << "thread 1 returns" << endl;
	cout << "thread 1 attempting to aquire lock b" << endl;
	thread_lock(b);
	cout << "thread 1 aquires lock b" << endl;
	cout << "thread 1 unlocking" << endl;
	thread_unlock(a1);
	thread_unlock(b);
	cout << "thread 1 complete" << endl;
}

void thread2(void *a) {
	cout << "thread 2 starting" << endl;
	cout << "thread 2 locking on b" << endl;
	thread_lock(b);
	cout << "thread 2 yielding" << endl;
	thread_yield();
	cout << "thread 2 returns" << endl;
	cout << "thread 2 attempting to aquire lock a" << endl;
	thread_lock(a1);
	cout << "thread 2 aquires lock a" << endl;
	cout << "thread 2 unlocking" << endl;
	thread_unlock(a1);
	thread_unlock(b);
	cout << "thread 2 complete" << endl;
}
void start(void *a) {
	int n;
	cout << "Start thread intialized" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread1, 0);
	thread_create((thread_startfunc_t)thread2, 0);
	cout << "Start thread completed" << endl;
}

int main() {
	if (thread_libinit((thread_startfunc_t)start, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}