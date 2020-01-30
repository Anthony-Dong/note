#include <iostream>
#include <typeinfo>
#include <string>

using namespace std;

class User
{
public:
    User(int age)
    {
        this->age = age;
    };
    int age;
};
typedef User SUser;

const SUser get()
{
    SUser user(1);
    return user;
};

int main(int argc, char const *argv[])
{

    SUser user = User(1);
    string line = "hello world";
    auto size = line.size();
    cout << typeid(size).name() << endl;
    cout << typeid(user).name() << endl;
    cout << line.c_str() << endl;
    SUser out = get();
    out = User(1);
    cout << out.age << endl;
    cout << user.age << endl;
    return 0;
}
