#include "thread.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

//Getting rid of prefixes
using namespace std;

//A struct to contain information about the sandwiches on the board. Keeps track of the sandwich number and the cashier who posted the sandwich.
//Initializes the sandwichId to -1 to represent an empty order on the board.
typedef struct sandwichOrder {
  sandwichOrder(): sandwichNum(-1) {}
  int cashierId;
  int sandwichNum;
} sandwichOrder;

//Struct to pass cashier information in single argument parameter.
typedef struct cashierInfo {
  int cashierId;
  char* cashierFile;
} cashierInfo;

//The global sandwich board.
vector<sandwichOrder> board;

//How many sandwich orders the board can fit. Defined in main.
int boardSize;

//How many cashiers still have pending orders.
int activeCashiers;

//Number of orders currently on the board. Initialized to zero.
int pendingOrders = 0;

//The most recently completed order.
sandwichOrder lastCompletedOrder =  sandwichOrder();

//Lock shared by SandwichMakerThread, cashierThread
unsigned int boardLock = 0;
//CV to wake signal the sandwich maker that the board is full and it needs to make a sandwich.
unsigned int sandwichMakerCV = 1;
//CV to signal the cashiers that they no longer have an order pending on the board. Adds the cashier ID to the end of this number as to only
//wake a single cashier.
unsigned int cashierPendingOrderCV = 100000;
//CV to signal the cashiers that there is an open spot on the board.
unsigned int openSpotOnBoardCV = 3;

//Checks to see whether the board is full/ if the maximum possible number of orders is on the board.
bool isBoardFull(){
  int maxOrders;
  if (activeCashiers < boardSize){
    maxOrders = activeCashiers;
  }
  else{
    maxOrders = boardSize;
  }
  return (pendingOrders >= maxOrders);
}

//Adds a sandwich order to the board
void addSandwichToBoard(int sandwichNum, int cashierId){

  for ( int i = 0; i < boardSize; i++) {
    if(board[i].sandwichNum == -1){
      board[i].sandwichNum = sandwichNum;
      board[i].cashierId = cashierId;
      pendingOrders++;

      //Tell the sandwich maker that there is a pending order.
      thread_signal(boardLock, sandwichMakerCV);
      cout << "POSTED: cashier " << cashierId << " sandwich " << sandwichNum << endl;
      return;
    }
  }
}

//finds which sandwich is closest to the previous order and returns the index of that sandwich on the board.
int nextSandwich() {
  int closestOrder = abs(board[0].sandwichNum - lastCompletedOrder.sandwichNum);
  int closestOrderIndex = 0;
  for (int i = 1; i < boardSize; i++){
    if(board[i].sandwichNum == -1){
      continue;
    }
    int orderNumDifference = abs(lastCompletedOrder.sandwichNum - board[i].sandwichNum);
    if (orderNumDifference < closestOrder){
      closestOrder = orderNumDifference;
      closestOrderIndex = i;
    }
  }
  return closestOrderIndex;
}

//Removes the next Sandwich from the board, updates the lastCompletedOrder var, broadcasts to relevant threads.
void removeSandwichFromBoard(){
  int closestOrderIndex = nextSandwich();
  lastCompletedOrder.sandwichNum = board[closestOrderIndex].sandwichNum;
  lastCompletedOrder.cashierId = board[closestOrderIndex].cashierId;
  cout << "READY: cashier " << lastCompletedOrder.cashierId << " sandwich " << lastCompletedOrder.sandwichNum << endl;

  board[closestOrderIndex].cashierId = -1;
  board[closestOrderIndex].sandwichNum = -1;

  pendingOrders--;
  thread_broadcast(boardLock, openSpotOnBoardCV);
  thread_broadcast(boardLock, cashierPendingOrderCV + lastCompletedOrder.cashierId);
}


void sandwichMakerThread(void* arg){
  thread_lock(boardLock);
  while(activeCashiers>0){
    while(!isBoardFull()){
      thread_wait(boardLock, sandwichMakerCV);
    }
    removeSandwichFromBoard();
  }
  thread_unlock(boardLock);
}

void cashierThread(void* arg){

  thread_lock(boardLock);
  cashierInfo* args = (cashierInfo*) arg;
  ifstream file ((char*) args->cashierFile);
  string order;
  while(getline(file, order)){
    int sandwichNumber = atoi(order.c_str());
    //waits for open spot on the board
    while(isBoardFull()){
      thread_wait(boardLock, openSpotOnBoardCV);
    }
    addSandwichToBoard(sandwichNumber, (int)args->cashierId);
    //Waits for no pending sandwiches from same cashier
    thread_wait(boardLock, cashierPendingOrderCV + (int)args->cashierId);
    }
  activeCashiers--;
  if(activeCashiers > 0){
    thread_signal(boardLock, sandwichMakerCV);
  }
  file.close();
  thread_unlock(boardLock);
}

//Initializes the sandwichMakerThread and the cashier threads
void deliThreadInitializer(void* args){
  char **argv = (char **) args;
  for( int i = 2; i < 2 + activeCashiers; i++){
    cashierInfo* cashier = new cashierInfo;
    cashier->cashierId = i - 2;
    cashier->cashierFile = argv[i];
    thread_create((thread_startfunc_t)cashierThread, (void *) cashier);
  }

  thread_create((thread_startfunc_t)sandwichMakerThread, NULL);
}

int main (int argc, char* argv[]) {

  boardSize = atoi(argv[1]);
  activeCashiers = argc - 2;
  //fills the board with empty orders
  for (int i = 0 ; i<boardSize; i++){
    board.push_back(sandwichOrder());
  }
  thread_libinit((thread_startfunc_t)deliThreadInitializer, argv);
}
