#include <string>
#include <dirent.h> //closedir
#include <dlfcn.h>
#define OUT_STRING "[monitor]" //default output
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
//IF can get filename ,print it . if can't print memory adress

using namespace std;
static void openfile(void) __attribute__((constructor));
static void closefile(void)  __attribute__((destructor));

static FILE *output;
static char *(*old_getenv)(const char *name)=NULL;
static void openfile(void)//we dont have main so we need this
{
    if(old_getenv==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getenv = (char * (*)(const char *))dlsym(handle, "getenv");
    }
    char *outputfile=old_getenv("MONITOR_OUTPUT");
    if(outputfile)
    {
        if(strcmp(outputfile,"stderr"))
        {
            output=stderr;
        }
        else
        {
            output=fopen(outputfile,"w");
        }
    }
    else
    {
        output=fopen("monitor.out","w");
    }
}
static void closefile(void)
{
    fclose(output);
}
static ssize_t (*old_readlink)(const char *path, char *buf, size_t bufsiz)=NULL;
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
            old_readlink = (ssize_t(*)(const char*, char *, size_t))dlsym(handle, "readlink");//
    }
    int n = old_readlink(path, filename, sizeof(filename));
    return filename;
}

static int (*old_closedir)(DIR *dirp) = NULL;
int closedir(DIR *dirp)
{
    string  dirname;
    if(dirp)
    {
        dirname = getnamebyfd(dirfd(dirp));
    }
    if(old_closedir == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_closedir = (int (*)(DIR *))dlsym(handle, "closedir");
    }
    int retvalue = old_closedir(dirp);
    if(!dirname.empty())
    {
        fprintf(output, "%s %s('%s') = %d\n", OUT_STRING,"closedir",dirname.c_str(),retvalue);
    }
    else
    {
        fprintf(output,"%s %s(%p) = %d\n",OUT_STRING,"closedir",dirp,retvalue);//print memory address
    }
    return retvalue;
}

static DIR * (*old_fdopendir)(int fd)= NULL;
DIR* fdopendir(int fd)
{
    if(old_fdopendir == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fdopendir = (DIR* (*)(int))dlsym(handle, "fdopendir");
    }
    DIR* retvalue = old_fdopendir(fd);
    if(retvalue)
    {
        string dirname = getnamebyfd(fd);
        fprintf(output,"%s %s(%d) = %s\n",OUT_STRING,"fdopendir",fd,dirname.c_str());
    }
    else
        fprintf(output,"%s %s(%d) = %p\n", OUT_STRING, "fdopendir", fd, retvalue);

    return retvalue;
}

static DIR * (*old_opendir)(const char *name)= NULL;
DIR* opendir(const char *name)
{
    if(old_opendir == NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_opendir = (DIR*(*)(const char*))dlsym(handle, "opendir");
    }
    DIR * retvalue = old_opendir(name);
    if(retvalue)
    {
        string dirname = getnamebyfd(dirfd(retvalue));
        fprintf(output,"%s %s('%s') = %s\n",OUT_STRING,"opendir",name,dirname.c_str());
    }
    else
    {
        fprintf(output,"%s %s('%s') = %p\n", OUT_STRING, "opendir", name, retvalue);
    }
    return retvalue;
}
static struct dirent *(*old_readdir)(DIR *dirp)=NULL;
struct dirent* readdir(DIR *dirp)//ask type **************************************
{
    if(old_readdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_readdir = (struct dirent*(*)(DIR *))dlsym(handle, "readdir");
    }
    string dirname= getnamebyfd(dirfd(dirp));
    struct dirent *retvalue = old_readdir(dirp);
    if(retvalue)
    {
        ino_t d_ino = retvalue->d_ino; //indoe number
        off_t d_off = retvalue->d_off;// offset to next dirent
        unsigned short d_reclen = retvalue->d_reclen;//length of this record
        unsigned char d_type = retvalue->d_type;//type of file
        char *d_name = retvalue->d_name;//filename
        fprintf(output,"%s %s('%s') = (inode=%zu, offset=%jd, reclen=%u, type=%u, name=%s)\n",OUT_STRING,"readdir",dirname.c_str(),d_ino,d_off,d_reclen,d_type,d_name);

    }
    else
    {
        fprintf(output,"%s %s('%s') = %p\n",OUT_STRING,"readdir",dirname.c_str(), retvalue);
    }
    return retvalue;
}
static int (*old_readdir_r)(DIR *dirp, struct dirent *entry, struct dirent **result)=NULL;
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
    if(old_readdir_r==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_readdir_r = (int (*)(DIR *,struct dirent*,struct dirent **))dlsym(handle, "readdir_r");
    }
    string dirname= getnamebyfd(dirfd(dirp));
    int retvalue = old_readdir_r(dirp,entry,result);
    if(!retvalue)
    {
        ino_t d_ino = entry->d_ino; //indoe number
        off_t d_off = entry->d_off;// offset to next dirent
        unsigned short d_reclen = entry->d_reclen;//length of this record
        unsigned char d_type = entry->d_type;//type of file
        char *d_name = entry->d_name;//filename
        fprintf(output,"%s %s('%s', (inode=%zu, offset=%jd, reclen=%u, type=%u, name=%s), %p) = %d\n",OUT_STRING,"readdir_r",dirname.c_str(),d_ino,d_off,d_reclen,d_type,d_name,result,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%s', %p, %p) = %d\n",OUT_STRING,"readdir_r",dirname.c_str(),entry,result,retvalue);
    }
    return retvalue;
}

static void (*old_rewinddir)(DIR *dirp)=NULL;
void rewinddir(DIR *dirp)//resets the position of the directory stream dirp to the beginning
{
    if(old_rewinddir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_rewinddir = (void(*)(DIR *))dlsym(handle, "rewinddir");
    }
    string dirname= getnamebyfd(dirfd(dirp));
    fprintf(output, "%s %s('%s')\n",OUT_STRING,"rewinddir",dirname.c_str());
}
static void(*old_seekdir)(DIR *dirp,long offset)=NULL;
void seekdir(DIR *dirp,long offset)//The seekdir() function sets the location in the directory stream from which the next readdir(2) call will start
{
    if(old_seekdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_seekdir = (void(*)(DIR *,long))dlsym(handle, "seekdir");
    }
    string dirname= getnamebyfd(dirfd(dirp));
    fprintf(output, "%s %s('%s', %ld)\n",OUT_STRING,"seekdir",dirname.c_str(),offset);
}
static long(*old_telldir)(DIR *dirp)=NULL;
long telldir(DIR *dirp)
{
    if(old_telldir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_telldir = (long(*)(DIR *))dlsym(handle, "telldir");
    }
    long retvalue=old_telldir(dirp);
    string dirname= getnamebyfd(dirfd(dirp));
    fprintf(output, "%s %s('%s') = %ld\n",OUT_STRING,"telldir",dirname.c_str(),retvalue);
}

static int (*old_creat)(const char *pathname, mode_t mode)=NULL;
int creat(const char *pathname, mode_t mode) //open and possibly create a file or device
{
    if(old_creat==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_creat = (int(*)(const char *,mode_t))dlsym(handle, "creat");
    }
    int retvalue = old_creat(pathname, mode);
    fprintf(output,"%s %s('%s',%o) = %d\n",OUT_STRING,"creat",pathname,mode,retvalue);
    return retvalue;
}
static int (*old_open)(const char* pahtname, int flags)=NULL;
extern "C" int open(const char *pathname, int flags)
{
    if(old_open==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_open = (int(*)(const char *,int))dlsym(handle, "open");
    }
    int retvalue = old_open(pathname, flags);
    fprintf(output,"%s %s('%s',%d) = %d\n",OUT_STRING,"open",pathname,flags,retvalue);
    return retvalue;
}

static int (*old_remove)(const char *pathname)=NULL;
int remove(const char *pathname)//remove a file or directory
{
    if(old_remove==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_remove = (int(*)(const char *))dlsym(handle, "remove");
    }
    int retvalue = old_remove(pathname);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"remove",pathname,retvalue);
    return retvalue;
}
static int (*old_rename)(const char *oldpath,const char *newpath)=NULL;
int rename(const char *oldpath,const char *newpath)//change the name or location of a file
{
    if(old_rename==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_rename = (int(*)(const char *,const char *))dlsym(handle, "rename");
    }
    int retvalue = old_rename(oldpath,newpath);
    fprintf(output,"%s %s('%s','%s') = %d\n",OUT_STRING,"rename",oldpath,newpath,retvalue);
    return retvalue;
}
static void (*old_setbuf)(FILE *stream, char *buf)=NULL;
void setbuf(FILE *stream, char *buf)// stream buffering operations
{
    if(old_setbuf==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setbuf = (void(*)(FILE *,char *))dlsym(handle, "setbuf");
    }
    old_setbuf(stream,buf);
    string filename;
    if(stream!=NULL)
    {
        int fd=fileno(stream);
        filename=getnamebyfd(fd);
    }
    fprintf(output,"%s %s('%s',%p)\n",OUT_STRING,"setbuf",filename.c_str(),buf);// %p  print memory address
}
static int (*old_setvbuf)(FILE *stream, char *buf,int mode,size_t size)=NULL;
int setvbuf(FILE *stream, char *buf, int mode, size_t size)//any open stream to change its buffer
{
    if(old_setvbuf==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setvbuf = (int(*)(FILE *,char *,int,size_t))dlsym(handle, "setvbuf");
    }
    int retvalue=old_setvbuf(stream,buf,mode,size);
    string filename;
    if(stream!=NULL)
    {
        int fd=fileno(stream);
        filename=getnamebyfd(fd);
    }
    fprintf(output,"%s %s('%s',%p,%d,%zu) = %d\n",OUT_STRING,"setbuf",filename.c_str(),buf,mode,size,retvalue);//%zu print size_t
    return retvalue;
}
static char *(*old_tempnam)(const char *dir, const char *pfx)=NULL;
char *tempnam(const char *dir, const char *pfx)//create a name for a temporary file
{
    if(old_tempnam==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_tempnam = (char* (*)(const char *,const char *))dlsym(handle, "tempnam");
    }
    char * retvalue=old_tempnam(dir,pfx);
    fprintf(output,"%s %s('%s', '%s') = %s\n",OUT_STRING,"tempnam",dir,pfx,retvalue);
    return retvalue;
}
static FILE *(*old_tmpfile)(void)=NULL;
FILE *tmpfile(void)//create a temporary file
{
    if(old_tmpfile==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_tmpfile = (FILE* (*)(void))dlsym(handle, "tmpfile");
    }
    FILE *retvalue=old_tmpfile();
    if(retvalue)
    {
        int fd=fileno(retvalue);
        string filename=getnamebyfd(fd);
        fprintf(output,"%s %s('') = %s\n",OUT_STRING,"tmpfile",filename.c_str());
    }
    else
    {
        fprintf(output,"%s %s('') = %p\n",OUT_STRING,"tmpfile",retvalue);
    }
    return retvalue;
}
static char *(*old_tmpnam)(char *s)=NULL;
char *tmpnam(char *s)//create a name for a temporary file
{
    if(old_tmpnam==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_tmpnam = (char * (*)(char *))dlsym(handle, "tmpnam");
    }
    char *retvalue = old_tmpnam(s);
    fprintf(output,"%s %s('%s') = %s\n",OUT_STRING,"tmpnam",s,retvalue);
    return retvalue;
}
static void (*old_exit)(int status)=NULL;
void exit(int status)//causes normal process termination and the value of status & 0377 is returned to the parent
{

    fprintf(output,"%s %s('%d')\n",OUT_STRING,"exit",status);
    if(old_exit==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_exit = (void (*)(int))dlsym(handle, "exit");
    }
    old_exit(status);
}

char *getenv(const char *name)//searches the environment list to find the environment variable name
{
    if(old_getenv==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getenv = (char * (*)(const char *))dlsym(handle, "getenv");
    }
    char *retvalue = old_getenv(name);
    fprintf(output,"%s %s('%s') = %s\n",OUT_STRING,"getenv",name,retvalue);
    return retvalue;
}

static char *(*old_mkdtemp)(char *atemplate)=NULL;
char *mkdtemp(char *atemplate)//generates a uniquely named temporary directory from template
{
    if(old_mkdtemp==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_mkdtemp = (char * (*)(char *))dlsym(handle, "mkdtemp");
    }
    char *retvalue = old_mkdtemp(atemplate);
    fprintf(output,"%s %s('%s') = %s\n",OUT_STRING,"mkdtemp",atemplate,retvalue);
    return retvalue;
}
static int (*old_mkstemp)(char *atemplate)=NULL;
int mkstemp(char *atemplate)//generates a unique temporary filename from template, creates and opens the file, and returns an open file descriptor for the file
{
    if(old_mkstemp==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_mkstemp = (int (*)(char *))dlsym(handle, "mkstemp");
    }
    int retvalue = old_mkstemp(atemplate);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"mkstemp",atemplate,retvalue);
    return retvalue;
}
static int (*old_putenv)(char *string)=NULL;
int putenv(char *string)//adds or changes the value of environment variables
{
    if(old_putenv==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_putenv = (int (*)(char *))dlsym(handle, "putenv");
    }
    int retvalue = old_putenv(string);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"putenv",string,retvalue);
    return retvalue;
}
static int (*old_rand)(void)=NULL;
int rand(void)// returns a pseudo-random integer in the range 0 to RAND_MAX inclusive
{
    if(old_rand==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_rand = (int (*)(void))dlsym(handle, "rand");
    }
    int retvalue = old_rand();
    fprintf(output,"%s %s() = %d\n",OUT_STRING,"rand",retvalue);
    return retvalue;
}
static int (*old_rand_r)(unsigned int *seedp)=NULL;
int rand_r(unsigned int *seedp)
{
    if(old_rand_r==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_rand_r = (int (*)(unsigned int *))dlsym(handle, "rand_r");
    }
    int retvalue = old_rand_r(seedp);
    fprintf(output,"%s %s('%u') = %d\n",OUT_STRING,"rand_r",*seedp,retvalue);
    return retvalue;
}
static int (*old_setenv)(const char *name, const char *value, int overwrite)=NULL;
int setenv(const char *name, const char *value, int overwrite)//adds the variable name to the environment with the value value
{
    if(old_setenv==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setenv = (int (*)(const char *,const char*,int))dlsym(handle, "setenv");
    }
    int retvalue = old_setenv(name,value,overwrite);
    fprintf(output,"%s %s('%s','%s','%d') = %d\n",OUT_STRING,"setenv",name,value,overwrite,retvalue);
    return retvalue;
}
static void (*old_srand)(unsigned int seed)=NULL;// sets its argument as the seed for a new sequence of pseudo-random integers to be returned by rand()
void srand(unsigned int seed)
{
    fprintf(output,"%s %s('%u')\n",OUT_STRING,"srand",seed);
    if(old_srand==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_srand = (void (*)(unsigned int))dlsym(handle, "srand");
    }
    old_srand(seed);
}
/*static int (*old_system)(const char *command)=NULL;
int system(const char *command)//executes a command specified in command by calling /bin/sh -c command
{
    if(old_system==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_system = (int (*)(const char *))dlsym(handle, "system");
    }
    int retvalue = old_system(command);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"system",command,retvalue);
    return retvalue;
}*/
static int (*old_chdir)(const char *path)=NULL;// cause the directory named by the pathname pointed to by the path argument to become the current working directory
int chdir(const char *path)
{
    if(old_chdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_chdir = (int (*)(const char *))dlsym(handle, "chdir");
    }
    int retvalue = old_chdir(path);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"chdir",path,retvalue);
    return retvalue;
}
static int (*old_chown)(const char *path, uid_t owner, gid_t group)=NULL;
int chown(const char *path, uid_t owner, gid_t group)//function shall change the user and group ownership of a file.
{
    if(old_chown==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_chown = (int (*)(const char *,uid_t,gid_t))dlsym(handle, "chown");
    }
    int retvalue = old_chown(path,owner,group);
    fprintf(output,"%s %s('%s','%d','%d') = %d\n",OUT_STRING,"chown",path,owner,group,retvalue);
    return retvalue;
}
static int (*old_close)(int fd)=NULL;
int close(int fd)//closes a file descriptor
{
    if(old_close==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_close = (int (*)(int))dlsym(handle, "close");
    }
    string filename = getnamebyfd(fd);
    int retvalue=old_close(fd);
    if(filename.empty()==0)//do have this file
    {
        fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"close",filename.c_str(),retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"close",fd,retvalue);
    }
    return retvalue;
}
static int (*old_dup)(int oldfd)=NULL;
int dup(int oldfd)//uses the lowest-numbered unused descriptor for the new descriptor.
{
    if(old_dup==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_dup = (int (*)(int))dlsym(handle, "dup");
    }
    int retvalue = old_dup(oldfd);
    fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"dup",oldfd,retvalue);
    return retvalue;
}
static int (*old_dup2)(int oldfd, int newfd)=NULL;
int dup2(int oldfd, int newfd)//makes newfd be the copy of oldfd, closing newfd first if necessary
{
    if(old_dup2==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_dup2 = (int (*)(int, int))dlsym(handle, "dup2");
    }
    int retvalue = old_dup2(oldfd,newfd);
    fprintf(output,"%s %s('%d','%d') = %d\n",OUT_STRING,"dup2",oldfd,newfd,retvalue);
    return retvalue;
}

static void (*old__exit)(int status)=NULL;//terminates the calling process "immediately". Any open file descriptors belonging to the process are closed
void _exit(int status)
{
    fprintf(output,"%s %s('%d')\n",OUT_STRING,"_exit",status);
    if(old__exit==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old__exit = (void(*)(int))dlsym(handle, "_exit");
    }

    fflush(output);
    old__exit(status);
}

static int (*old_fchdir)(int fd)=NULL;
int fchdir(int fd)//s identical to chdir(); the only difference is that the directory is given as an open file descriptor
{
    if(old_fchdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fchdir = (int (*)(int))dlsym(handle, "fchdir");
    }
    string filename = getnamebyfd(fd);
    int retvalue=old_fchdir(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"fchdir",filename.c_str(),retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"fchdir",fd,retvalue);
    }
    return retvalue;
}
static int (*old_fchown)(int fd, uid_t owner, gid_t group)=NULL;
int fchown(int fd, uid_t owner, gid_t group)//changes the ownership of the file referred to by the open file descriptor fd.
{
    if(old_fchown==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fchown = (int (*)(int,uid_t,gid_t))dlsym(handle, "fchown");
    }
    string filename = getnamebyfd(fd);
    int retvalue=old_fchown(fd,owner,group);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%d','%d') = %d\n",OUT_STRING,"fchown",filename.c_str(),owner,group,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%d','%d') = %d\n",OUT_STRING,"fchown",fd,owner,group,retvalue);
    }
    return retvalue;
}
static pid_t (*old_fork)(void)=NULL;
pid_t fork(void)//fork() creates a new process by duplicating the calling process
{
    fflush(output);
    if(old_fork==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fork = (pid_t (*)(void))dlsym(handle, "fork");
    }
    pid_t retvalue=old_fork();

    fprintf(output,"%s %s() = %d\n",OUT_STRING,"fork",retvalue);
}
static int (*old_fsync)(int fd)=NULL;
int fsync(int fd)//transfers ("flushes") all modified in-core data of  to by the file descriptor fd to the disk device
{
    if(old_fsync==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fsync = (int (*)(int))dlsym(handle, "fsync");
    }
    int retvalue=old_fsync(fd);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"fsync",filename.c_str(),retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"fsync",fd,retvalue);
    }
    return retvalue;
}
static int (*old_ftruncate)(int fd, off_t length)=NULL;
int ftruncate(int fd, off_t length)//cause the regular file named by path or referenced by fd to be truncated to a size of precisely length bytes
{
    if(old_ftruncate==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_ftruncate = (int (*)(int,off_t))dlsym(handle, "ftruncate");
    }
    int retvalue=old_ftruncate(fd,length);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%jd') = %d\n",OUT_STRING,"ftruncate",filename.c_str(),length,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%jd') = %d\n",OUT_STRING,"ftruncate",fd,length,retvalue);
    }
    return retvalue;
}
static char *(*old_getcwd)(char *buf, size_t size)=NULL;
char *getcwd(char *buf, size_t size)//copies an absolute pathname of the current working directory to the array pointed to by buf
{
    if(old_getcwd==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getcwd = (char * (*)(char * ,size_t ))dlsym(handle, "getcwd");
    }
    char *retvalue=old_getcwd(buf,size);
    fprintf(output,"%s %s('%s','%zu') = %s\n",OUT_STRING,"getcwd",buf,size,retvalue);
    return retvalue;
}
static gid_t (*old_getegid)(void)=NULL;
gid_t getegid(void)//returns the effective group ID of the calling process.
{
    if(old_getegid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getegid = (gid_t (*)(void))dlsym(handle, "getegid");
    }
    gid_t retvalue = old_getegid();
    fprintf(output,"%s %s() = %d\n",OUT_STRING,"getegid",retvalue);
    return retvalue;
}
static uid_t (*old_geteuid)(void)=NULL;
uid_t geteuid(void)
{
    if(old_geteuid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_geteuid = (uid_t (*)(void))dlsym(handle, "geteuid");
    }
    uid_t retvalue = old_geteuid();
    fprintf(output,"%s %s() = %d\n",OUT_STRING,"geteuid",retvalue);
    return retvalue;
}
static gid_t (*old_getgid)(void)=NULL;
gid_t getgid(void)
{
    if(old_getgid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getgid = (gid_t (*)(void))dlsym(handle, "getgid");
    }
    gid_t retvalue = old_getgid();
    fprintf(output,"%s %s() = %d\n",OUT_STRING,"getgid",retvalue);
    return retvalue;
}
static uid_t (*old_getuid)(void)=NULL;
uid_t getuid(void)
{
    if(old_getuid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_getuid = (uid_t (*)(void))dlsym(handle, "getuid");
    }
    uid_t retvalue = old_getuid();
    fprintf(output,"%s %s() = %d\n",OUT_STRING,"getuid",retvalue);
    return retvalue;
}
static int (*old_link)(const char *oldpath, const char *newpath)=NULL;
int link(const char *oldpath, const char *newpath)//creates a new hard link to an existing file.
{
    if(old_link==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_link = (int (*)(const char *,const char *))dlsym(handle, "link");
    }
    int retvalue = old_link(oldpath,newpath);
    fprintf(output,"%s %s('%s','%s') = %d\n",OUT_STRING,"link",oldpath,newpath,retvalue);
    return retvalue;
}
static int (*old_pipe)(int pipefd[2])=NULL;
int pipe(int pipefd[2])//creates a pipe, a unidirectional data channel that can be used for interprocess communication
{
    if(old_pipe==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_pipe = (int (*)(int *))dlsym(handle, "pipe");
    }
    int retvalue = old_pipe(pipefd);
    fprintf(output,"%s %s('%d','%d') = %d\n",OUT_STRING,"pipe",pipefd[0],pipefd[1],retvalue);
    return retvalue;
}
static ssize_t (*old_pread)(int fd, void *buf, size_t count, off_t offset)=NULL;
ssize_t pread(int fd, void *buf, size_t count, off_t offset)//reads up to count bytes from file descriptor fd at offset offset into the buffer
{
    if(old_pread==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_pread = (ssize_t (*)(int,void *,size_t,off_t))dlsym(handle, "pread");
    }
    ssize_t retvalue=old_pread(fd,buf,count,offset);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%p','%zu','%jd') = %zd\n",OUT_STRING,"pread",filename.c_str(),buf,count,offset,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%p','%zu','%jd') = %zd\n",OUT_STRING,"pread",fd,buf,count,offset,retvalue);
    }
    return retvalue;
}
static ssize_t (*old_pwrite)(int fd, const void *buf, size_t count, off_t offset)=NULL;
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    if(old_pwrite==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_pwrite = (ssize_t (*)(int,const void *,size_t,off_t))dlsym(handle, "pwrite");
    }
    ssize_t retvalue=old_pwrite(fd,buf,count,offset);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%p','%zu','%jd') = %zd\n",OUT_STRING,"pwrite",filename.c_str(),buf,count,offset,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%p','%zu','%jd') = %zd\n",OUT_STRING,"pwrite",fd,buf,count,offset,retvalue);
    }
    return retvalue;
}
static ssize_t (*old_read)(int fd, void *buf, size_t count)=NULL;
ssize_t read(int fd, void *buf, size_t count)
{
    if(old_read==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_read = (ssize_t (*)(int,void *,size_t))dlsym(handle, "read");
    }
    ssize_t retvalue=old_read(fd,buf,count);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%p','%zu') = %zd\n",OUT_STRING,"read",filename.c_str(),buf,count,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%p','%zu','jd') = %zd\n",OUT_STRING,"read",fd,buf,count,retvalue);
    }
    return retvalue;
}

ssize_t readlink(const char *path, char *buf, size_t bufsiz)//readlink() places the contents of the symbolic link path in the buffer buf
{
    if(old_readlink==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_readlink = (ssize_t (*)(const char*,char *,size_t))dlsym(handle, "readlink");
    }
    ssize_t retvalue=old_readlink(path,buf,bufsiz);
    fprintf(output,"%s %s('%s','%s','%zu') = %zd\n",OUT_STRING,"readlink",path,buf,bufsiz,retvalue);
    return retvalue;
}
static int (*old_rmdir)(const char *pathname)=NULL;
int rmdir(const char *pathname)//deletes a directory, which must be empty.
{
    if(old_rmdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_rmdir = (int (*)(const char*))dlsym(handle, "rmdir");
    }
    int retvalue=old_rmdir(pathname);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"rmdir",pathname,retvalue);
    return retvalue;
}
static int (*old_setegid)(gid_t egid)=NULL;
int setegid(gid_t egid)//sets the effective user ID of the calling process
{
    if(old_setegid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setegid = (int (*)(gid_t))dlsym(handle, "setegid");
    }
    int retvalue=old_setegid(egid);
    fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"setegid",egid,retvalue);
    return retvalue;
}
static int (*old_seteuid)(uid_t euid)=NULL;
int seteuid(uid_t euid)//sets the effective user ID
{
    if(old_seteuid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_seteuid = (int (*)(uid_t))dlsym(handle, "seteuid");
    }
    int retvalue=old_seteuid(euid);
    fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"seteuid",euid,retvalue);
    return retvalue;
}

static int (*old_setgid)(gid_t gid)=NULL;
int setgid(gid_t gid)
{
    if(old_setgid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setgid = (int (*)(gid_t))dlsym(handle, "setgid");
    }
    int retvalue=old_setgid(gid);
    fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"setgid",gid,retvalue);
    return retvalue;
}
static int (*old_setuid)(uid_t uid)=NULL;
int setuid(uid_t uid)
{
    if(old_setuid==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_setuid = (int (*)(uid_t))dlsym(handle, "setuid");
    }
    int retvalue=old_setuid(uid);
    fprintf(output,"%s %s('%d') = %d\n",OUT_STRING,"setuid",uid,retvalue);
    return retvalue;
}
static unsigned int (*old_sleep)(unsigned int seconds)=NULL;
unsigned int sleep(unsigned int seconds)//makes the calling thread sleep until seconds seconds have elapsed
{
    if(old_sleep==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_sleep = (unsigned int (*)(unsigned int))dlsym(handle, "sleep");
    }
    unsigned int retvalue=old_sleep(seconds);
    fprintf(output,"%s %s('%u') = %u\n",OUT_STRING,"sleep",seconds,retvalue);
    return 0;
}
static int (*old_symlink)(const char *oldpath, const char *newpath)=NULL;
int symlink(const char *oldpath, const char *newpath)//creates a symbolic link named newpath which contains the string oldpath.
{
    if(old_symlink==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_symlink = (int (*)(const char*,const char*))dlsym(handle, "symlink");
    }
    int retvalue=old_symlink(oldpath,newpath);
    fprintf(output,"%s %s('%s','%s') = %d\n",OUT_STRING,"symlink",oldpath,newpath,retvalue);
    return retvalue;
}

static int (*old_unlink)(const char *pathname)=NULL;
int unlink(const char *pathname)//deletes a name from the file system
{
    if(old_unlink==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_unlink = (int (*)(const char*))dlsym(handle, "unlink");
    }
    int retvalue=old_unlink(pathname);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"unlink",pathname,retvalue);
    return retvalue;
}

static ssize_t (*old_write)(int fd, const void *buf, size_t nbyte)=NULL;
ssize_t write(int fd, const void *buf, size_t nbyte)//writes up to count bytes from the buffer pointed buf to the file referred to by the file descriptor fd
{
    if(old_write==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_write = (ssize_t (*)(int,const void *,size_t))dlsym(handle, "write");
    }
    ssize_t retvalue=old_write(fd,buf,nbyte);
    string filename = getnamebyfd(fd);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%p','%zu') = %zd\n",OUT_STRING,"write",filename.c_str(),buf,nbyte,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%p','%zu') = %zd\n",OUT_STRING,"write",fd,buf,nbyte,retvalue);
    }
    return retvalue;
}

static int (*old_chmod)(const char *path, mode_t mode)=NULL;
int chmod(const char *path, mode_t mode)//These system calls change the permissions of a file
{
    if(old_chmod==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_chmod = (int (*)(const char *,mode_t))dlsym(handle, "chmod");
    }
    int retvalue=old_chmod(path,mode);
    fprintf(output,"%s %s('%s','%o') = %d\n",OUT_STRING,"chmod",path,mode,retvalue);
    return retvalue;
}
static int (*old_fchmod)(int fd, mode_t mode)=NULL;
int fchmod(int fd, mode_t mode)
{
    if(old_fchmod==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fchmod = (int (*)(int ,mode_t))dlsym(handle, "fchmod");
    }
    string filename = getnamebyfd(fd);
    int retvalue = old_fchmod(fd,mode);
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s','%o') = %d\n",OUT_STRING,"fchmod",filename.c_str(),mode,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d','%o') = %d\n",OUT_STRING,"fchmod",fd,mode,retvalue);
    }
    return retvalue;
}

static int (*old_mkdir)(const char *pathname, mode_t mode)=NULL;
int mkdir(const char *pathname, mode_t mode)
{
    if(old_mkdir==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_mkdir = (int(*)(const char*,mode_t))dlsym(handle, "mkdir");
    }
    int retvalue = old_mkdir(pathname,mode);
    fprintf(output, "%s %s('%s', %o) = %d\n", OUT_STRING, "mkdir", pathname, mode, retvalue);
    return retvalue;
}
static int (*old_mkfifo)(const char *pathname, mode_t mode)=NULL;
int mkfifo(const char *pathname, mode_t mode)//makes a FIFO special file with name pathname
{
    if(old_mkfifo==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_mkfifo = (int(*)(const char*,mode_t))dlsym(handle, "mkfifo");
    }
    int retvalue = old_mkfifo(pathname,mode);
    fprintf(output, "%s %s('%s', %o) = %d\n", OUT_STRING, "mkfifo", pathname, mode, retvalue);
    return retvalue;
}
static mode_t (*old_umask)(mode_t mask)=NULL;
mode_t umask(mode_t mask)//sets the calling process's file mode creation mask (umask) to mas
{
    if(old_umask==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_umask = (mode_t(*)(mode_t))dlsym(handle, "umask");
    }
    mode_t retvalue = old_umask(mask);
    fprintf(output, "%s %s('%o') = %o\n", OUT_STRING, "umask", mask, retvalue);
    return retvalue;
}
static int (*old__fxstat)(int ver,int fd,struct stat *buf)=NULL;
int __fxstat(int ver,int fd ,struct stat *buf)
{
    if(old__fxstat==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old__fxstat = (int (*)(int,int,struct stat *))dlsym(handle,"__fxstat");
        }
    }
    int retvalue = old__fxstat(ver,fd,buf);
    string filename=getnamebyfd(fd);
    dev_t dev= buf->st_dev;     /* ID of device containing file */
    ino_t ino= buf->st_ino;     /* inode number */
    mode_t mode= buf->st_mode;    /* protection */
    nlink_t nlink=buf->st_nlink;   /* number of hard links */
    uid_t uid = buf->st_uid;     /* user ID of owner */
    gid_t gid = buf->st_gid;     /* group ID of owner */
    dev_t rdev = buf->st_rdev;    /* device ID (if special file) */
    off_t size = buf->st_size;    /* total size, in bytes */
    blksize_t blksize=buf->st_blksize; /* blocksize for file system I/O */
    blkcnt_t blocks=buf->st_blocks;  /* number of 512B blocks allocated */
    time_t atime=buf->st_atime;   /* time of last access */
    time_t mtime=buf->st_mtime;   /* time of last modification */
    time_t ctime=buf->st_ctime;   /* time of last status change */
    if(filename.empty()==0)
    {
        fprintf(output,"%s %s('%s',(dev=%zu,ino=%zu,mode=%o,nlink=%zu,uid=%d,gid=%d,rdev=%zu,size=%jd,blksize=%zu,blocks=%zu,atime=%zu,mtime=%zu,ctime=%zu)=%d\n)",OUT_STRING,"__fxstat",filename.c_str(),dev,ino,mode,nlink,uid,gid,rdev,size,blksize,blocks,atime,mtime,ctime,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%d',%p)=%d\n",OUT_STRING,"__fxstat",fd,buf,retvalue);
    }
    return retvalue;
}

static int (*old__lxstat)(int ver,const char *path,struct stat *buf)=NULL;
int __lxstat(int ver,const char *path  ,struct stat *buf)
{
    if(old__lxstat==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old__lxstat = (int (*)(int,const char *,struct stat *))dlsym(handle,"__lxstat");
        }
    }
    int retvalue = old__lxstat(ver,path,buf);
    dev_t dev= buf->st_dev;     /* ID of device containing file */
    ino_t ino= buf->st_ino;     /* inode number */
    mode_t mode= buf->st_mode;    /* protection */
    nlink_t nlink=buf->st_nlink;   /* number of hard links */
    uid_t uid = buf->st_uid;     /* user ID of owner */
    gid_t gid = buf->st_gid;     /* group ID of owner */
    dev_t rdev = buf->st_rdev;    /* device ID (if special file) */
    off_t size = buf->st_size;    /* total size, in bytes */
    blksize_t blksize=buf->st_blksize; /* blocksize for file system I/O */
    blkcnt_t blocks=buf->st_blocks;  /* number of 512B blocks allocated */
    time_t atime=buf->st_atime;   /* time of last access */
    time_t mtime=buf->st_mtime;   /* time of last modification */
    time_t ctime=buf->st_ctime;   /* time of last status change */
    if(retvalue>=0)
    {
        fprintf(output,"%s %s('%s',(dev=%zu,ino=%zu,mode=%o,nlink=%zu,uid=%d,gid=%d,rdev=%zu,size=%jd,blksize=%zu,blocks=%zu,atime=%zu,mtime=%zu,ctime=%zu)=%d\n)",OUT_STRING,"__lxstat",path,dev,ino,mode,nlink,uid,gid,rdev,size,blksize,blocks,atime,mtime,ctime,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%s',%p)=%d\n",OUT_STRING,"__lxstat",path,buf,retvalue);
    }
    return retvalue;
}
static int (*old__xstat)(int ver,const char *path,struct stat *buf)=NULL;
int __xstat(int ver,const char *path ,struct stat *buf)
{
    if(old__xstat==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old__xstat = (int (*)(int,const char *,struct stat *))dlsym(handle,"__xstat");
        }
    }
    int retvalue = old__xstat(ver,path,buf);
    dev_t dev= buf->st_dev;     /* ID of device containing file */
    ino_t ino= buf->st_ino;     /* inode number */
    mode_t mode= buf->st_mode;    /* protection */
    nlink_t nlink=buf->st_nlink;   /* number of hard links */
    uid_t uid = buf->st_uid;     /* user ID of owner */
    gid_t gid = buf->st_gid;     /* group ID of owner */
    dev_t rdev = buf->st_rdev;    /* device ID (if special file) */
    off_t size = buf->st_size;    /* total size, in bytes */
    blksize_t blksize=buf->st_blksize; /* blocksize for file system I/O */
    blkcnt_t blocks=buf->st_blocks;  /* number of 512B blocks allocated */
    time_t atime=buf->st_atime;   /* time of last access */
    time_t mtime=buf->st_mtime;   /* time of last modification */
    time_t ctime=buf->st_ctime;   /* time of last status change */
    if(retvalue>=0)
    {
        fprintf(output,"%s %s('%s',(dev=%zu,ino=%zu,mode=%o,nlink=%zu,uid=%d,gid=%d,rdev=%zu,size=%jd,blksize=%zu,blocks=%zu,atime=%zu,mtime=%zu,ctime=%zu)=%d\n)",OUT_STRING,"__xstat",path,dev,ino,mode,nlink,uid,gid,rdev,size,blksize,blocks,atime,mtime,ctime,retvalue);
    }
    else
    {
        fprintf(output,"%s %s('%s',%p)=%d\n",OUT_STRING,"__xstat",path,buf,retvalue);
    }
    return retvalue;
}


/*int fflush(FILE *stream)
{
    if(old_fflush==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_fflush = (int (*)(FILE *))dlsym(handle, "fflush");
    }
    string filename;
    if(stream)
    {
        filename=getnamebyfd(fileno(stream));
    }
    int retvalue = old_fflush(stream);
    fprintf(output,"%s %s('%s')= %d\n",OUT_STRING,"fflush",filename.c_str(),retvalue);
    return retvalue;
}*/
char **copyenvforexe(char **oldenv)
{
    int arg_count=0;
    int count=0;
    char **newenv;
    for(int i=0; oldenv[i]!=NULL; i++)
    {
        arg_count++;
    }
    newenv=(char **)malloc(arg_count *sizeof(char *)+1);
    for(int i=0; oldenv[i]!=NULL; i++)
    {
        if(strncmp("LD_PRELOAD",oldenv[i],10))//strstr also
        {
            newenv[count] = (char *) malloc(strlen(oldenv[i]) * sizeof(char) + 1);
            strcpy(newenv[count],oldenv[i]);
            count++;
        }
    }
    newenv[count]=NULL;
    return newenv;
}//credit from google
static int (*old_execve)(const char *filename,char *const argv[],char *const envp[])=NULL;//pass va to this function
int execl(const char *path,const char *arg,...)
{
    ptrdiff_t argc;//present ptr - ptr
    va_list arr;//for unknown parameters
    va_start(arr,arg);
    for(argc =1; va_arg(arr,const char *); argc++)
    {
        if(argc==INT_MAX)//count
        {
            va_end(arr);
            return -1;
        }
    }
    va_end(arr);
    char *argv[argc+1];
    va_start(arr,arg);
    argv[0]=(char *)arg;
    for(ptrdiff_t i=1; i<=argc; i++) //traverse
    {
        argv[i]=va_arg(arr,char *);
    }
    va_end(arr);
    fprintf(output,"%s %s('%s', )",OUT_STRING,"execl",path);
    for(ptrdiff_t i=0; i<=argc; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
    }
    fprintf(output," )\n");

    fflush(output);
    extern char **environ;
    char **newenv=copyenvforexe(environ);

    if(old_execve==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_execve = (int (*)(const char *,char *const argv[],char *const envp[]))dlsym(handle, "execve");
    }
    int retvalue = old_execve(path,argv,newenv);
    fseek(output,-1,SEEK_CUR);//from now position move left 1
    fprintf(output,"= %d\n",retvalue);
    return retvalue;
}

int execle(const char *path,const char *arg,...)
{
    ptrdiff_t argc;//present ptr - ptr
    va_list arr;//for unknown parameters
    va_start(arr,arg);
    for(argc =1; va_arg(arr,const char *); argc++)
    {
        if(argc==INT_MAX)//count
        {
            va_end(arr);
//errno = E2BIG;
            return -1;
        }
    }
    va_end(arr);
    char *argv[argc+1];
    va_start(arr,arg);
    argv[0]=(char *)arg;
    for(ptrdiff_t i=1; i<=argc; i++) //traverse
    {
        argv[i]=va_arg(arr,char *);
    }
    char **envp = va_arg(arr,char **);//the last argument is environment variable so we have to save it
    va_end(arr);
    for(int i=0; envp[i]!=NULL; i++) //just like copyenv but different way
    {
        if(!strncmp("LD_PRELOAD",envp[i],10))
        {
            envp[i]=NULL;
        }
    }
    fprintf(output,"%s %s('%s', )",OUT_STRING,"execle",path);
    for(ptrdiff_t i=0; i<=argc; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
        if(i==argc)
        {
            fprintf(output,"{");
            for(int j=0; j<3 && envp[j]!=NULL; j++)
            {
                fprintf(output,"'%s' , ",envp[j]);
            }
            fprintf(output,"}");
        }
    }
    fprintf(output," )\n");

    fflush(output);

    if(old_execve==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_execve = (int (*)(const char *,char *const argv[],char *const envp[]))dlsym(handle, "execve");
    }
    int retvalue = old_execve(path,argv,envp);
    fseek(output,-1,SEEK_CUR);//from now position move left 1
    fprintf(output,"= %d\n",retvalue);
    return retvalue;
}

int execlp(const char *file,const char *arg,...)
{
    ptrdiff_t argc;//present ptr - ptr
    va_list arr;//for unknown parameters
    va_start(arr,arg);
    for(argc =1; va_arg(arr,const char *); argc++)
    {
        if(argc==INT_MAX)//count
        {
            va_end(arr);
            return -1;
        }
    }
    va_end(arr);
    char *argv[argc+1];
    va_start(arr,arg);
    argv[0]=(char *)arg;
    for(ptrdiff_t i=1; i<=argc; i++) //traverse
    {
        argv[i]=va_arg(arr,char *);
    }
    va_end(arr);
    fprintf(output,"%s %s('%s', )",OUT_STRING,"execlp",file);
    for(ptrdiff_t i=0; i<=argc; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
    }
    fprintf(output," )\n");

    fflush(output);
    extern char **environ;
    char **newenv=copyenvforexe(environ);
    int retvalue = execvpe(file,argv,newenv);//directly using
    fseek(output,-1,SEEK_CUR);//from now position move left 1
    fprintf(output,"= %d\n",retvalue);
    return retvalue;
}


int execv(const char *path,char *const argv[])
{
    ptrdiff_t argc;//present ptr - ptr

    fprintf(output,"%s %s('%s', )",OUT_STRING,"execv",path);
    for(ptrdiff_t i=0; argv[i]!=NULL; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
    }
    fprintf(output," )\n");
    fflush(output);
    extern char **environ;
    char **newenv=copyenvforexe(environ);
    if(old_execve==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_execve = (int (*)(const char *,char *const argv[],char *const envp[]))dlsym(handle, "execve");
    }
    int retvalue = old_execve(path,argv,newenv);
    fseek(output,-1,SEEK_CUR);//from now position move left 1
    fprintf(output,"= %d\n",retvalue);
    return retvalue;
}

int execve(const char *filename,char *const argv[],char *const envp[] )
{
    fprintf(output,"%s %s('%s', )",OUT_STRING,"execve",filename);
    for(ptrdiff_t i=0; argv[i]!=NULL; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
    }
    fprintf(output," )\n");
    //const envp so we can't call copyenvexe...
    char **newenv;
    int argcount=0;
    int count=0;
    for(ptrdiff_t i=0; envp[i]!=NULL; i++)
    {
        argcount++;
    }
    newenv= (char **)malloc(argcount * sizeof(char *)+1);
    for(ptrdiff_t i=0; envp[i]!=NULL; i++)
    {
        if(strncmp("LD_PRELOAD",envp[i],10))
        {
            newenv[count]=(char *)malloc((strlen(envp[i])*sizeof(char))+1);
            strcpy(newenv[count],envp[i]);
            count++;
        }
    }
    newenv[count]=NULL;
    fprintf(output,"{");//print env
    for(int j=0; j<3 && envp[j]!=NULL; j++)
    {
        fprintf(output,"'%s' , ",envp[j]);
    }
    fprintf(output,"}");
    fflush(output);
    if(old_execve==NULL)
    {
        void *handle = dlopen("libc.so.6", RTLD_LAZY);
        if(handle != NULL)
            old_execve = (int (*)(const char *,char *const argv[],char *const envp[]))dlsym(handle, "execve");
    }
    int retvalue=old_execve(filename,argv,newenv);
    fseek(output,-1,SEEK_CUR);
    fprintf(output," = %d\n",retvalue);
}
int execvp(const char *file,char *const argv[])
{
    fprintf(output,"%s %s('%s', )",OUT_STRING,"execvp",file);
    for(ptrdiff_t i=0; argv[i]!=NULL; i++) //print
    {
        fprintf(output,"%s, ",argv[i]);
    }
    fprintf(output," )\n");
    fflush(output);
    extern char **environ;
    char **newenv = copyenvforexe(environ);
    int retvalue = execvpe(file,argv,newenv);
    fseek(output,-1,SEEK_CUR);
    fprintf(output,"= %d\n",retvalue);
    return retvalue;
}

static size_t (*old_fread)(void *ptr, size_t size, size_t nmemb, FILE * stream );
size_t fread(void *ptr, size_t size, size_t nmemb,FILE * stream )
{
    if(old_fread==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old_fread = (size_t (*)(void *,size_t,size_t,FILE *))dlsym(handle,"fread");
        }
    }
    string filename;
    if(stream)
    {
        filename = getnamebyfd(fileno(stream));
    }
    size_t retvalue=old_fread(ptr,size,nmemb,stream);
    fprintf(output,"%s %s(%p ,%zu, %zu, '%s') = %zu\n",OUT_STRING,"fread",ptr,size,nmemb,filename.c_str(),retvalue);
    return retvalue;
}

static size_t (*old_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE * stream );
size_t fwrite(const void *ptr, size_t size, size_t nmemb,FILE * stream )
{
    if(old_fwrite==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old_fwrite = (size_t (*)(const void *,size_t,size_t,FILE *))dlsym(handle,"fwrite");
        }
    }
    string filename;
    if(stream)
    {
        filename = getnamebyfd(fileno(stream));
    }
    size_t retvalue=old_fwrite(ptr,size,nmemb,stream);
    fprintf(output,"%s %s(%p ,%zu, %zu, '%s') = %zu\n",OUT_STRING,"fwrite",ptr,size,nmemb,filename.c_str(),retvalue);
    return retvalue;
}
static int (*old_fputc)(int c, FILE *stream);
int fputc(int c, FILE *stream)
{
    if(old_fputc==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old_fputc = (int (*)(int,FILE *))dlsym(handle,"fputc");
        }
    }
    string filename;
    if(stream)
    {
        filename = getnamebyfd(fileno(stream));
    }
    int retvalue=old_fputc(c,stream);
    fprintf(output,"%s %s(%d ,'%s') = %d\n",OUT_STRING,"fputc",c,filename.c_str(),retvalue);
    return retvalue;
}

static int (*old_fputs)(const char *s, FILE *stream);
int fputs(const char *s, FILE *stream)
{
    if(old_fputs==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old_fputs = (int (*)(const char*,FILE *))dlsym(handle,"fputs");
        }
    }
    string filename;
    if(stream)
    {
        filename = getnamebyfd(fileno(stream));
    }
    int retvalue=old_fputs(s,stream);
    fprintf(output,"%s %s('%s' ,'%s') = %d\n",OUT_STRING,"fputs",s,filename.c_str(),retvalue);
    return retvalue;
}
static int (*old_fgetc)(FILE *stream);
int fgetc(FILE *stream)
{
    if(old_fgetc==NULL)
    {
        void *handle = dlopen("libc.so.6",RTLD_LAZY);
        if(handle != NULL)
        {
            old_fgetc = (int (*)(FILE *))dlsym(handle,"fgetc");
        }
    }
    string filename;
    if(stream)
    {
        filename = getnamebyfd(fileno(stream));
    }
    int retvalue=old_fgetc(stream);
    fprintf(output,"%s %s('%s') = %d\n",OUT_STRING,"fputs",filename.c_str(),retvalue);
    return retvalue;
}

