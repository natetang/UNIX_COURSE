#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
//#include <string>
#include <dirent.h> //closedir
#define GLOLIB "libc.so.6" //open this library
#define OUT_STRING "[monitor]" //default output

static uid_t (*old_getuid)(void) = NULL; /* function pointer */
uid_t getuid(void)
{
    if(old_getuid == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getuid = dlsym(handle, "getuid");
    }
    uid_t retvalue = old_getuid();
    if(old_getuid != NULL)
        fprintf(stderr, "%s %s() = %d\n", OUT_STRING,"getuid",retvalue);
    return retvalue;
}
