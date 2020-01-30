#include <stdio.h>

// 输入 argc .  第一个参数表示程序名称
// 
int main(int argc, char const *argv[])
{	
	printf("%d\n", argc);
	for (int i = 0; i < argc; ++i)
	{
		printf("%s\n", argv[i]);
	}
	printf("%s\n", "hello world");
	return 0;
}