#include <stdio.h>


/// 枚举也是很有用处的. 第一个变量需要定义一个初始值, 默认是0
enum DAY{
    MON=1, TUE, WED, THU, FRI, SAT, SUN
} day;
int main()
{
    // 遍历枚举元素
    for (day = MON; day <= SUN;day++) {
        printf("%d \n", day);
    }

    enum DAY days;

    scanf("%d",&days);
    switch(days){
    	case SUN:
    		 printf("today is sum");
    		 break;
    	case MON:
    		printf("%s\n","today is mon" );	
    		break;
    	default:
    		printf("%s\n","today is workding" );	  
    }
    printf("%d\n", days);
    return 0;
}