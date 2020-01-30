#include <stdio.h>
#include <string.h>

// 这里的意思是接收一个字符串.
// 跟数组一样首位是指针第一个
void fun1(char *str){
	printf("%s\n",str);
}

// void connect(char *str1,char *str2){
// 	int len1=strlen(str1);
// 	int len2=strlen(str2);
// 	for (int i = 0; i < len1+len2; ++i)
// 	{
		
// 	}
// }

// 这里的意思是接收一个字符串数组
int main(int argc, char const *argv[])
{
	char str1[]="hello world!";
	printf("%s\n",str1);
	fun1(&str1[0]);
	printf("len : %d\n",strlen(str1));
	char str2[]="I will be better!";
	strcat(str1,str2);
	printf("connect : %s\n",str1);
	return 0;
}