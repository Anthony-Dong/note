#include <iostream>
#include <string.h>
#include <string>

// 这个是一个空间, 类似于Java类的静态方法可以直接使用, import static java.util.Arrays.*; 在调用Array静态方法时可以省略写Array
using namespace std;

void change(char *name)
{
    strcpy(name, "changed !");
}

int main(int argc, char const *argv[])
{
    char arr[] = "original";
    cout << arr << endl;
    change(arr);
    cout << arr << endl;

    int x = 1;
    int &y = x;
    cout << x << "," << y << endl;
    x = 10;
    cout << x << "," << y << endl;
    return 0;
}
