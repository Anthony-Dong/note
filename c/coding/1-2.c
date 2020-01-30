#include <stdio.h>
int x=1;
int y=2;
int add(void){
	extern int x;
	extern int y;
	printf("x : %d y: %d \n", x,y);
	return x+y;
}
static int count=10;
int main()
{	
	while(count--){
		printf("%d\n", count);
	}
	int a=add();
	printf("%d\n", a);
	printf("%s\n", "hello world");
	printf("%d\n", count);
	return 0;
}


