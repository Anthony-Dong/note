#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>

using namespace std;

int main(int argc, char const *argv[])
{
    vector<int> arr = {1, 2, 0, 4, 5};
    arr.push_back(6);
    for (auto i = arr.begin(); i != arr.end(); i++)
    {
        cout << *i << " ";
    }
    cout << endl;

    for (int j : arr)
    {
        cout << j << " ";
    }
    cout << endl;

    // string str = "abcdefg";
    // for (auto i = str.begin(); i != str.end() && !isspace(*i); i++)
    // {
    //     cout << *i << endl;
    //     *i = toupper(*i);
    // }
    // /* code */
    // for (auto i = str.begin(); i != str.end(); i++)
    // {
    //     cout << *i << endl;
    //     *i = toupper(*i);
    // }

    return 0;
}
