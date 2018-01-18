
#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

void thread2(void *a) {
	int n;
	cout << "Thread 2 yielding" << endl;
	//cin >> n;
	thread_yield();
	cout << "Thread 2 completed" << endl;
	//cin >> n;
}

void thread1(void *a) {
	int n;
	cout << "Thread 1 started" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t) thread2, 0);
	thread_yield();
	cout << "Thread 1 completed" << endl;
}



int main() {
	if (thread_libinit((thread_startfunc_t)thread1, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}