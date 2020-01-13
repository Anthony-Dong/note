#include <stdio.h>
int main()
{
    struct
    {
        char *name;  //姓名
        int num;     //学号
        int age;     //年龄
        char group;  //所在小组
        float score; //成绩
    } stu1 = {"Tom", 12, 18, 'A', 136.5}, *pstu = &stu1;

    printf("%s\n\r", stu1);
    (*pstu).name = "tony";
    printf("%s\n\r", (*pstu).name);

    stu1.name = "sb";
    printf("%s\n\r", stu1.name);

    printf("%s\n\r", pstu->name);

    return 0;
}
