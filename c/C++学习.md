# C++  数据类型 & 类 

## 1. 基本数据类型

基本数据类型就这些  .... char*代表的是 char数组 -> 字符串 -> 数组一个值的指针 

```c++
#include <iostream>
#include <string.h>

// 这个是一个空间, 类似于Java类的静态方法可以直接使用, import static java.util.Arrays.*; 在调用Array静态方法时可以省略写Array,比如 Arrays.sort(arr); 可以直接写 sort(arr) ; 
// C++中不是.调用是 :: 称为 `作用域运算符` , 所以可以将 std::cout 改成 cout ,编译器会从左边的std的作用域找cout
// 还有就是他还有其他作用, 比如限制scope.
using namespace std;

void change(char *name)
{
    // 字符串赋值... c中的string.h库
    strcpy(name, "changed !");
}

int main(int argc, char const *argv[])
{
    char arr[] = "original !";
    cout << arr << endl; /// original !
    change(arr);
    cout << arr << endl; // changed !
    return 0;
}
```





| 类型      | 16位系统/字节 | 32位系统/字节 | 64位系统/字节 |
| --------- | ------------- | ------------- | ------------- |
| char      | 1             | 1             | 1             |
| char*     | 2             | 4             | 4             |
| short     | 2             | 2             | 2             |
| int       | 2             | 4             | 4             |
| long      | 4             | 4             | 8             |
| long long | 8             | 8             | 8             |
| double    |               |               | 8             |



## 2. 