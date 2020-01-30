#include  <stdio.h>

// 回调函数
// 申明 :  函数返回值 (* 函数名) (函数参数类型) = 函数地址
int max(int x,int y){
	return x>y?x:y;
}

int main(int argc, char const *argv[])
{
	int (*p)(int,int)=&max;
	int a,b,c;
	scanf("%d %d %d",&a,&b,&c);
    int d=p(p(a,b),c);
	printf("max: %d\n", d);
	return 0;
}