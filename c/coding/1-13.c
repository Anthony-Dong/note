#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	char *desc;
	desc=(char *)malloc(200*sizeof(char));
	strcpy(desc,"hello world");
	printf("%s\n",desc);
	free(desc);
	printf("%s\n",desc);
	return 0;
}