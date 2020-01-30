#include <iostream>
#include <string>

using namespace std;

int fun(string name)
{
    cout << name << endl;
    return name.length();
}

int main(int argc, char const *argv[])
{

    int (*fun1)(string) = &fun;
    int x = fun1("hello world");
    cout << x;
    return 0;
}
