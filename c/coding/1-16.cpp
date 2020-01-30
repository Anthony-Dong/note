#include <iostream>
#include <string>
using namespace std;

class A
{
public:
    static int x;
    A(int age)
    {
        this->age = age;
    }
    //  析构造函数
    ~A(void);
    int getName(void)
    {
        return age;
    };
    static void say(string name)
    {
        cout << name << endl;
    };

protected:
    int age;
};

A::~A(void)
{
    cout << "deleted" << endl;
};

class B : A
{
public:
    int getAge(void);
};
int B::getAge(void)
{
    return age;
};
int name;

int A::x = 0;

int main(int argc, char const *argv[])
{
    A a(1);
    cout << a.getName() << endl;
    A::x = 10;
    cout << "x : " << A::x << endl;
    A::say("tom");
    return 0;
}
