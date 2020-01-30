#include <iostream>
#include <bits/stdc++.h>

using namespace std;

int insert(int arr[3], int size)
{
    for (int i = 0; i < size; i++)
    {
        arr[i] = 10 + i;
    }
}

int main(int argc, char const *argv[])
{

    char name[1];
    cin >> name;
    cout << name << endl;

    int arr[4] = {1, 2, 3, 10};
    insert(arr, 4);
    for (int i = 0; i < 4; i++)
    {
        cout << arr[i] << endl;
    }

    return 0;
}
