#include <stdio.h>
#include <stdlib.h> //for exit

#include "dmm.h"

/* 
 * To compile with your dmm.c:
 * 
 * $> make dmm.o
 * $> gcc -I. -Wall -lm -DNDEBUG -o basicdmmtest basicdmmtest.c dmm.o
 * $> ./basicdmmtest
*/
int main(int argc, char *argv[]) {
	char *array1, *array2, *array3, *array4, *array5, *array6, *array7, *array8;
  int i;

  printf("calling malloc(10)\n");
  array1 = (char*)dmalloc(10);
  print_freelist2();
  print_physlist();
  if(array1 == NULL) {
    fprintf(stderr,"call to dmalloc() failed\n");
    fflush(stderr);
    exit(1);
  }
  /*
  for(i=0; i < 9; i++) {
    array1[i] = 'a';
  }
  array1[9] = '\0';

  printf("String: %s\n",array1);
  */
  printf("calling malloc(200)\n");	
  array2 = (char*)dmalloc(200);
  print_freelist2();
  print_physlist();
  if(array2 == NULL) {
    fprintf(stderr,"call to dmalloc() failed\n");
    fflush(stderr);
    exit(1);
  }

  printf("calling malloc(300)\n");
  array3 = (char*)dmalloc(300);
  print_freelist2();
  print_physlist();
  if (array2 == NULL) {
	  fprintf(stderr, "call to dmalloc() failed\n");
	  fflush(stderr);
	  exit(1);
  }

  /*
  for(i=0; i < 99; i++) {
    array2[i] = 'b';
  }
  array2[99] = '\0';

  printf("String : %s, %s\n",array1, array2);

  */
  printf("calling free(300)\n");	
  dfree(array3);
  print_freelist2();
  print_physlist();

  printf("calling free(10)\n");
  dfree(array1);
  print_freelist2();
  print_physlist();

  printf("calling free(200)\n");
  dfree(array2);
  print_freelist2();
  print_physlist();

  printf("calling malloc(864)\n");	
  array3 = (char*)dmalloc(864);
  print_freelist2();
  print_physlist();

  if(array3 == NULL) {
    fprintf(stderr,"call to dmalloc() failed\n");
    fflush(stderr);
    exit(1);
  }
  for(i=0; i < 864; i++) {
    array3[i] = 'c';
  }
  array3[864] = '\0';

  //printf("String: %s, %s, %s\n",array1, array2, array3);

  printf("calling free(864)\n");	
  dfree(array3);
  print_freelist2();
  print_physlist();

  printf("try to free a bullshit value\n");
  dfree(array3 - 7);
  print_freelist2();
  print_physlist();

  printf("try to malloc more than available\n");
  array1 = (char*)dmalloc(2000);
  print_freelist2();
  print_physlist();

  printf("try to malloc maximum size\n");
  array1 = (char*)dmalloc(928);
  print_freelist2();
  print_physlist();

  printf("try to malloc with full slab\n");
  array2 = (char*)dmalloc(125);
  print_freelist2();
  print_physlist();

  printf("free entire maximum size\n");
  dfree(array1);
  print_freelist2();
  print_physlist();

  printf("try to malloc 0 bytes\n");
  array1 = (char*)dmalloc(0);
  print_freelist2();
  print_physlist();

  printf("try to malloc negative bytes\n");
  array2 = (char*)dmalloc(-5);
  print_freelist2();
  print_physlist();

  printf("free previous 2 mallocs\n");
  dfree(array1);
  dfree(array2);
  print_freelist2();
  print_physlist();

  printf("allocate several small arrays\n");
  array1 = dmalloc(32);
  array2 = dmalloc(64);
  array3 = dmalloc(128);
  array4 = dmalloc(128);
  array5 = dmalloc(64);
  array6 = dmalloc(32);
  array7 = dmalloc(16);
  print_freelist2();
  print_physlist();

  printf("free them in a random order\n");
  dfree(array4);
  print_freelist2();
  print_physlist();
  dfree(array1);
  print_freelist2();
  print_physlist();
  dfree(array5);
  print_freelist2();
  print_physlist();
  dfree(array3);
  print_freelist2();
  print_physlist();
  dfree(array2);
  print_freelist2();
  print_physlist();
  dfree(array7);
  print_freelist2();
  print_physlist();
  dfree(array6);
  print_freelist2();
  print_physlist();
  printf("Basic testcases passed!\n");

  return(0);
}
