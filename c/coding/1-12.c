#include <stdio.h>
#include <stdarg.h>

void fun(int num,...){
	va_list valist;
	va_start(valist,3);
	printf("%d\n",va_arg(valist,int));
	va_end(valist);
}
int main(int argc, char const *argv[])
{
	fun(1,2,3,4);
	return 0;
}