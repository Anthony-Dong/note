#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    int a = 3;
    decltype(a) c = a;
    c++;
    cout << a << endl;
    decltype((a)) d = a;
    d++;
    cout << a << endl;
    return 0;
}
