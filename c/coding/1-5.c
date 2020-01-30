#include <stdio.h>

// 所以传递一个数组其实就是传递数组的头指针. 即&arr[0]
// 但是取值依旧可以这么取 arr[n]
void fun(int *arr){
	for(int y=0;y<10;y++){
		arr[y]=y;
		printf("%d\t",arr[y]);
	}
}

int main()
{
	// 数组中 数组名是指向数组变量的第一个指针.也就是指向 &arr[0] 的指针
	int arr[10];
	for(int x=10;x<20;x++){
		arr[x-10]=x+10;
	}
	for(int y=0;y<10;y++){
		printf("%d\t",arr[y]);
	}
	printf("\n");

	int *a1;

	a1=arr;
	for(int y=0;y<10;y++){
		printf("%d\t",*(a1+y));
	}
	printf("\n");
	fun(&arr[0]);

	printf("\n");
	for(int y=0;y<10;y++){
		printf("%d\t",arr[y]);
	}     
	return 0;
}