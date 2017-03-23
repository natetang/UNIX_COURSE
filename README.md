# Implement a 'netstat -nap'-like program
## In this homework, you have to implement a 'netstat -nap' tool by yourself. You have to list all the existing TCP and UDP connections. For each identified connection (socket descriptor), find the corresponding process name and its command lines that creates the connection (socket descriptor). You have to implement all the features by yourself and cannot make calls to the system built-in netstat program nor parse output from 'netstat -nap'. Your codes must be implemented in C and/or C++.

## To provide more flexibilities, your program have to accept several predefined options, including

### -t or --tcp: list only TCP connections.
### -u or --udp: list only UDP connections.
### An optional string to list only command lines that containing the string.
### You have to handle the additional options using getopt_long function. In short, the synopsis of homework #1 would be:
### $ ./hw1 [-t|--tcp] [-u|--udp] [filter-string]
### When no argument is passed, your program should output all identified connections. You may test your program with a root account so that your program would be able to access /proc files owned by other users.

## Grading
The tentative grading policy for this homework is listed below:

[10%] List TCP and UDP connetions (IPv4).
[10%] List TCP and UDP connetions (IPv6).
[30%] Show corresponding command lines and arguments for each identified connection.
[10%] Implement -u and --udp option using getopt_long(3).
[10%] Implement -t and --tcp option using getopt_long(3).
[10%] Translate network address into user-friendly formats, e.g., from 0100007F to 127.0.0.1, and from FE01A8C0 to 192.168.1.254.
[10%] Implement basic filter string feature.
[10%] Use Makefile to manage the building process of your program.
[10%] If your filter string supports regular expression, see regex(3).
Contact GitHub API Training Shop Blog About
© 2017 GitHub, Inc. Terms Privacy Security Status Help
