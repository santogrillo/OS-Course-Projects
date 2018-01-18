#include "thread.h"
#include <iostream>
#include <stdlib.h>


using namespace std;

void thread6(void *a) {
	int n;
	cout << "Thread 6 started" << endl;
	thread_yield();
	cout << "Thread 6 completed" << endl;
}

void thread5(void *a) {
	int n;
	cout << "Thread 5 started" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread6, 0);
	thread_yield();
	cout << "Thread 5 completed" << endl;
}

void thread4(void *a) {
	int n;
	cout << "Thread 4 started" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread5, 0);
	thread_yield();
	cout << "Thread 4 completed" << endl;
}

void thread3(void *a) {
	int n;
	cout << "Thread 3 started" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread4, 0);
	thread_yield();
	cout << "Thread 3 completed" << endl;
}

void thread2(void *a) {
	int n;
	cout << "Thread 2 started" << endl;
	//cin >> n;
	thread_create((thread_startfunc_t)thread3, 0);
	thread_yield();
	cout << "Thread 2 completed" << endl;
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
	if (thread_libinit((thread_startfunc_t) thread1, 0)) {
		cout << "thread_lib_init failed" << endl;
		exit(1);
	}
}