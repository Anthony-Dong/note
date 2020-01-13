#include <stdio.h>
 

void release(int *x){
	x=NULL;
	printf("x 在函数内部的地址是 %x\n",x);
}

int main ()
{	
  int num=100;	
  int *x=&num;
  release(x);
  printf("x main函数的地址是 %x\n",x);
  printf("NULL 的地址是 %x\n",NULL);
  return 0;
}

