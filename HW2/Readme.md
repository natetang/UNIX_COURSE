# Homework 2
## Advanced Programming in the UNIX Environment

Due: April 17, 2017

* NOTE: We have reduced the number of monitored function calls listed in "Minimum Requirements" to reduce the workload for this homework.

* Hi! Jack ... Hijack!?
* In this homework, we are going to practice library injection and API hijacking. Please implement a "library call monitor" (LCM) program that is able to show the activities of an arbitrary binary running on a Linux operating system. You have to implement your LCM as a shared library and inject the shared library into a process using using LD_PRELOAD. You have to dump the library calls as well as a summary of passed parameters. Please monitor at least the functions listed in the section "Minimum Requirements" below. The result should be stored into a filename, e.g., "monitor.out". You may write the output to a filename specified by an environment variable "MONITOR_OUTPUT". If the value of MONITOR_OUTPUT is stderr, output the messages to standart error instead of a file.

* You have to compile your source codes and generate a shared object. You don't have to implement any monitored program by yourself. Instead, you must work with those binaries already installed in the system.

* Minimum Requirements
* The minimum list of monitored library calls is shown below. It covers almost all the functions we have introduced in the class.

* closedir fdopendir opendir readdir readdir_r rewinddir seekdir telldir creat open remove rename setbuf setvbuf tempnam tmpfile tmpnam exit getenv mkdtemp mkstemp putenv rand rand_r setenv srand system chdir chown close dup dup2 _exit execl execle execlp execv execve execvp fchdir fchown fork fsync ftruncate getcwd getegid geteuid getgid getuid link pipe pread pwrite read readlink rmdir setegid seteuid setgid setuid sleep symlink unlink write chmod fchmod fstat lstat mkdir mkfifo stat umask
* If you would like to monitor more, please read the function lists from the following manual pages: (ordered alphabetically)

* dirent.h(P)
* fcntl.h(P)
* stdio.h(P)
* stdlib.h(P)
* sys_socket.h(7POSIX) (network functions)
* unistd.h(P)
* sys_stat.h(7POSIX)
* Summarize of Function Call Parameters
* You will get a basic score if you only print out the raw value of monitored function calls. For example, the primitive data types char, int, short, long, long long, float, and double. For pointers, you can also print out its raw values. If you would like to get higher scores, here are additional requirements.

For char * data type, you can print it out as a string.
For file descriptors (passed as an int), FILE*, and DIR* pointers, you can convert them to corresponding file names.
For char * arrays, print out the first few strings in the array.
For uid and gid (also passed as an int), convert them to the corresponding user name and group name.
For struct stat or its pointer, retrieve meaningful information from the structure. For example, file type, file size, and permissions.
Grading Policy
The tentative grading policy for this homework is listed below:

* [20%] A monitored executable can work as usual. Your program cannot affect the original function of a monitored executable.
* [30%] Monitor functions listed in minimum requirements.
* [10%] Provide basic summaries for function call parameters.
* [20%] Provide comprehensive summaries for function call parameters.
* [10%] Output can be configured using MONITOR_OUTPUT environmental variable.
* [20%] Monitor more library calls than the minimum requirement (up to 5). Please describe why you choose the function and the corresponding testing binaries
* [10%] Compiled size of your codes (stripped). The smaller, the better.
* [10%] Use Makefile to manage the building process of your program. We will not grade your program if we cannot use make command to build your program.

Homework Submission
Please pack your files into a single ZIP archive and submit your homework via the E3 system. Please also provide a Makefile (used for compiling and linking your codes) and a README file (indicating what functions will be monitored).

## Hints
## Some hints that may simplify your work:

* You may need to define macros to simplify your implementation.
* You may consider working with # and ## operators in macros.
* For variable-length function parameters, consider working with strarg.h.
* You may consider working with __attribute__((constructor)). If you don't know what is that, please google for it!
* The implementation for some library functions may be different to its well-known prototypes. For example, the actual implementation for stat in GNU C library is __xstat. Therefore, you may be not able to find symbol stat in the library. In case that you are not sure about the real symbols used in C library, try to work with readelf or nm to get the symbol names.
