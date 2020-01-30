#include <stdio.h>
#include <string.h>
struct Book
{
	char name[50];
};

int main(int argc, char const *argv[])
{
	struct Book b;
	strcpy(b.name,"hello world");
	printf("%s\n", b.name);
	struct Book *b2;
	b2=&b;
	printf("%s\n",b2->name);
	return 0;
}