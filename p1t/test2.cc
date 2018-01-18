#include "thread.h"
#include <iostream>
#include <stdlib.h>

int lock = 1;

using namespace std;

void thread3(void *a) {
	thread_yield(); //want thread 1 to run, unlock
	cout << "thread 3 trying to aquire the lock" << endl;
	thread_lock(lock);
	cout << "thread 3 aquired the lock" << endl;
	thread_unlock(lock);
	cout << "thread 3 releasing lock" << endl;
}

void thread2(void *a) {
	cout << "thread 2 trying to aquire lock" << endl;
	thread_lock(lock);	//should block, thread 3 runs next
	cout << "thread 2 aquired the lock" << endl;
	thread_unlock(lock);
	cout << "thread 2 releasing lock" << endl;
}

void thread1(void *a) {
	thread_create((thread_startfunc_t)thread2, 0);
	thread_create((thread_startfunc_t)thread3, 0);
	
	thread_lock(lock);
	cout << "thread 1 aquired the lock" << endl;
	//switch to thread 2
	thread_yield();
	cout << "thread 1 releasing lock" << endl;
	thread_unlock(lock);

}


int main() {
	if (thread_libinit((thread_startfunc_t)thread1, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}