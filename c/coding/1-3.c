#include <stdio.h>

// auto , register , static , external , 这些都是c的存储类 . 他们定义了c变量的可见性, 

// 定义一个局部变量,
const int count=10;
// 可以指向外部的编译文件,因为只会生产一个a.exe/a.out文件
extern void cprint();
void func(){
	// 被static修饰的方法变量, 也是自会初始化一次,
	// 所以输出 9-8-7-6-5 ...
	static int c=10;
	printf("c-1: %d , c:%d\n",c, c--);
}
int main()
{
	for (int x=0;x<5;x++){
	   func();
	}
	// 
	cprint();

	char *c=NULL;
	printf("%p\n", c);
	return 0;
}