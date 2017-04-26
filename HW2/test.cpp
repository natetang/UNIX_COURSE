#include <string>
#include <dirent.h> //closedir
#include <dlfcn.h>
#define GLOLIB "libc.so.6" //open this library
#define OUT_STRING "[monitor]" //default output
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
typedef ssize_t (*readlink_t)(const char *path, char *buf, size_t bufsiz);

static ssize_t (*old_readlink)(const char *path, char *buf, size_t bufsiz);
string getnamebyfd(int fd)
{
    char path[1024];
    char filename[1024];
    if(fd < 0)return "";
    sprintf(path,"/proc/self/fd/%d",fd);
    //readlink_t original_readlink = (readlink_t) dlsym(handle, "readlink");
    if(old_readlink == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_readlink = (ssize_t(*)(const char*, char *, size_t)) dlsym(handle, "readlink");
    }
    int n = old_readlink(path, filename, sizeof(filename));
    return filename;
}
int main()
{
    int fd = open("getuid.c", 0);
    printf("%s\n", getnamebyfd(fd).c_str());
}
