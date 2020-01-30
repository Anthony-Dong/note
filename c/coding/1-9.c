#include <stdio.h>
#include <stdlib.h>

int random(void){
	return rand();
}

void insert(int *arr,int size,int (*rand)(void)){
	for (int i = 0; i < size; ++i)
	{
		arr[i]=rand();
	}
}

int main(int argc, char const *argv[])
{	
	int arr[20];
	insert(arr,20,random);
	for (int i = 0; i < 20; ++i)
	{
		printf("%d\t",arr[i]);
	}
	return 0;
}