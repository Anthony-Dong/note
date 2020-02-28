#include <iostream>
#include <string>
#include <typeinfo>

using namespace std;

template <typename T>

int compare(T a, T b)
{
    cout << "a : " << typeid(a).name() << " , b :" << typeid(b).name() << endl;
    return a > b ? 1 : a == b ? 0 : -1;
}
// int compare(double a, double b)
// {
//     cout << "a : " << typeid(a).name() << " , b :" << typeid(b).name() << endl;
//     return a > b ? 1 : a == b ? 0 : -1;
// }

int main(int argc, char const *argv[])
{
    int rest = compare(1, 2);
    cout << rest << endl;
    return 0;
}
