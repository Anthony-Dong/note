#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const *argv[])
{
    string str = "abc";
    for (decltype(str.size()) index = 0; index < str.size(); index++)
    {
        str[index] = toupper(str[index]);
    }
    cout << str << endl;
    return 0;
}
