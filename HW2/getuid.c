#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
int main()
{
    printf("UID = %d\n", getuid());
    return 0;
}
