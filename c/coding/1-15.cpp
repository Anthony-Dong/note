#include <iostream>

using namespace std;

class Box
{
public:
   int length;
   int breadth;
   int height;
   int getVolume(void); // 返回体积
};
int Box::getVolume(void)
{
   return length * breadth * height;
}

int main(int argc, char const *argv[])
{
   Box box1;
   box1.length = 10;
   box1.height = 10;
   box1.breadth = 10;
   cout << "breadth : " << box1.breadth << "," << box1.height << "," << box1.length << endl;
   int out = box1.getVolume();
   cout << "volume:" << out << endl;
   int x;
   cout << x << endl;
   return 0;
}
