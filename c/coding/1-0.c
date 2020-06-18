#include <stdio.h>
#include <string.h>

void trim(char *str)
{
    int len = strlen(str);
    int start, end, index = 0;
    end = len - 1;
    while (end > 0 && str[end] ==' ')
        end--;
    while (start < (len - 1) && str[start] ==' ')
        start++;
    // while (start <= end)
    // {
    //     str[index] = str[start];
    //     index++;
    //     start++;
    // }

    while (index < len - 1)
    {
        /* code */
        str[index] = ' ';
        index++;
    }
}

int main(int argc, char const *argv[])
{

    char str[] = "   12311111111   ";
    trim(str);
    printf("%s", str);
}
