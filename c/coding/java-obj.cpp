#include <iostream>
#include <string>

using namespace std;

class A
{
public:
    string name;
};

void init(A *b)
{
    cout << "未改变的地址 : " << b << endl;
    A a = A();
    b = &a;
    cout << "改变后的地址 : " << b << endl;
}
int main(int argc, char const *argv[])
{
    A *a = new A();
    cout << "init : " << a->name << endl;
    a->name = "main";
    cout << "赋值 : " << a->name << endl;
    init(a);
    cout << "init : " << a->name << endl;
    return 0;
}