#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <getopt.h>
using namespace std;
string parse_port(string *a,int index)
{	
	int i=0;
	while(a[index][i]!=':')
	{
		i++;
	}
	i++;	
	int len=a[index].length();
	string b;
	b=b.assign(a[index],i,len);
	for(i=i-1;i<len;i++)
	{
		a[index][i]='\0';
	}
	return b;
}
void transform_iprint(string *a,int index)
{
	a[index]="0x"+a[index];
	stringstream tmp;
	tmp << hex << a[index];
	uint32_t value;
	tmp >> value;
	struct in_addr ipv4addr;
	ipv4addr.s_addr = htonl(ntohl(value));
	char *ans = inet_ntoa(ipv4addr);
	cout << ans;
}
void transform_portprint(string a)
{
	stringstream tmp;
	tmp << hex << a;
	uint32_t value;
	tmp >> value;
	if(value==0){cout<<"*\t"<<"\t";}
	else{cout <<value<<"\t"<<"\t";}
}
void findPID(string *a, int index)
{
	DIR *dir,*dir2;
        struct dirent *ptr;
	char filepath[50];
	char filetext[50];
	FILE *fp;
	dir=opendir("/proc");
	if(NULL != dir)
	{
		while((ptr = readdir(dir)) != NULL)
		{
			if( ( isdigit(ptr->d_name[0])!=0))
			{
				sprintf(filepath,"/proc/%s/fd",ptr->d_name);
				dir2=opendir(filepath);
				struct dirent *ptr2;
				while((ptr2 = readdir(dir2))!=NULL)
				{
					sprintf(filepath,"/proc/%s/fd/%s",ptr->d_name,ptr2->d_name);
					readlink(filepath,filetext,sizeof(filetext));
					if(strstr(filetext,a[index].c_str())!=NULL)
					{
						//PID
						cout<<ptr->d_name;
						sprintf(filepath,"/proc/%s/comm",ptr->d_name);
						fp = fopen(filepath,"r");
						if(NULL != fp)
						{
							//program name
							fgets(filetext,sizeof(filetext),fp);
							cout <<"/"<<filetext;
							fclose(fp);
							
						}
						sprintf(filepath,"/proc/%s/cmdline",ptr->d_name);
						ifstream argu;
						argu.open(filepath,ios::in);
						string input;
						getline(argu,input);
						stringstream ss(input);
						string sub_str;
						getline(ss,sub_str,'\0');							
						while(getline(ss,sub_str,'\0'))
						{	
							cout <<sub_str;
						}	
						cout<<endl;
					}
				}
				closedir(dir2);				
			}	
		}
		closedir(dir);
	}
}
void tcp_proc()
{
	ifstream TCP;
	TCP.open("/proc/net/tcp",ios::in);
	string arr[100],line,port;
       	getline(TCP,line);
	int i=0;
	//TCP
	while(getline(TCP,line))	
	{
		cout <<"tcp\t";
		stringstream parse(line);
		while(parse >> arr[i])
		{
			if(i==1||i==2){
					port=parse_port(arr,i);
					transform_iprint(arr,i);
					cout<<":";
					transform_portprint(port);
					}//local
			else if(i==9)findPID(arr,i);//inode
			i++;
		}
		i=0;
	}
}
void transform_ip6print(string *a,int index)
{
	char before[32];
	strcpy(before,a[index].c_str());
	char ip_str[128];
	struct in6_addr ipv6addr;
	if(sscanf(before,"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",&ipv6addr.s6_addr[3],
		&ipv6addr.s6_addr[2],&ipv6addr.s6_addr[1],&ipv6addr.s6_addr[0],&ipv6addr.s6_addr[7],&ipv6addr.s6_addr[6],&ipv6addr.s6_addr[5],
                &ipv6addr.s6_addr[4],&ipv6addr.s6_addr[11],&ipv6addr.s6_addr[10],&ipv6addr.s6_addr[9],&ipv6addr.s6_addr[8],&ipv6addr.s6_addr[15],
                &ipv6addr.s6_addr[14],&ipv6addr.s6_addr[13],&ipv6addr.s6_addr[12])==16)
		{
			inet_ntop(AF_INET6,&ipv6addr,ip_str,sizeof (ip_str));
			printf("%s",ip_str);
		}
}
void tcp_proc6()
{	
	ifstream TCP6;
	TCP6.open("/proc/net/tcp6",ios::in);
	string arr[100],line,port;
       	getline(TCP6,line);
	int i=0;
	//TCP6
	while(getline(TCP6,line))	
	{
		cout <<"tcp6\t";
		stringstream parse(line);
		while(parse >> arr[i])
		{
			if(i==1||i==2){
					port=parse_port(arr,i);
					transform_ip6print(arr,i);
					cout<<":";
					transform_portprint(port);
					}//local
			else if(i==9)findPID(arr,i);//inode
			i++;
		}
		i=0;
	}
}
void udp_proc()
{	
	ifstream UDP;
	UDP.open("/proc/net/udp",ios::in);
	string arr[100],line,port;
       	getline(UDP,line);
	int i=0;
	//UDP
	while(getline(UDP,line))	
	{
		cout <<"udp\t";
		stringstream parse(line);
		while(parse >> arr[i])
		{
			if(i==1||i==2){
					port=parse_port(arr,i);
					transform_iprint(arr,i);
					cout<<":";
					transform_portprint(port);
					}//local
			else if(i==9)findPID(arr,i);//inode
			i++;
		}
		i=0;
	}
}
void udp_proc6()
{
	ifstream UDP6;
	UDP6.open("/proc/net/udp6",ios::in);
	string arr[100],line,port;
       	getline(UDP6,line);
	int i=0;
	//UDP6
	while(getline(UDP6,line))	
	{
		cout <<"udp6\t";
		stringstream parse(line);
		while(parse >> arr[i])
		{
			if(i==1||i==2){
					port=parse_port(arr,i);
					transform_ip6print(arr,i);
					cout<<":";
					transform_portprint(port);
					}//local
			else if(i==9)findPID(arr,i);//inode
			i++;
		}
		i=0;
	}
}
void print_tcpinit()
{
printf("List of TCP connections:\n");
cout << "Proto\t"<<"Local Address\t"<<"\t"<<"Foreign Address\t"<<"\t"<<"PID/Program name and arguments\t"<<endl;
}
void print_udpinit()
{
printf("List of UDP connections:\n");
cout << "Proto\t"<<"Local Address\t"<<"\t"<<"Foreign Address\t"<<"\t"<<"PID/Program name and arguments\t"<<endl;
}
void argu_proc(string input)
{	
	DIR *dir,*dir2;
        struct dirent *ptr,*ptr2;
	char filepath[50];
	char filetext[50];
	dir=opendir("/proc");
	int flag=0;
	if(NULL != dir)
	{
		while((ptr = readdir(dir)) != NULL)
		{	
	           if( ( isdigit(ptr->d_name[0])!=0))
		   {		
			sprintf(filepath,"/proc/%s/comm",ptr->d_name);
			ifstream prog;
			prog.open(filepath,ios::in);
			string readfrompro;
			getline(prog,readfrompro);
			if(readfrompro==input)
			{
				sprintf(filepath,"/proc/%s/fd",ptr->d_name);//cout<<ptr2->d_name<<endl;
				dir2=opendir(filepath);
				while((ptr2 = readdir(dir2))!= NULL)
				{
			       		sprintf(filepath,"/proc/%s/fd/%s",ptr->d_name,ptr2->d_name);
					readlink(filepath,filetext,sizeof(filetext));
					ifstream getip;
					string readfromip;
					getip.open("/proc/net/tcp",ios::in);
					//cout <<"this is proname"<<readfrompro<<endl;
					while(getline(getip,readfromip))
					{	
						//cout<<"first loop"<<endl;
						stringstream parse(readfromip);
						string arr[20];
						int i=0;
						while(parse >>arr[i])
						{
							if(i==9 && (strstr(filetext,arr[9].c_str())!=NULL))
							{
								//cout <<filetext <<"\t"<<arr[9]<<endl;						
								string port;
								cout<<"tcp\t";
								port=parse_port(arr,1);
								transform_iprint(arr,1);
								cout<<":";
								transform_portprint(port);
								port=parse_port(arr,2);
								transform_iprint(arr,2);
								cout<<":";
								transform_portprint(port);
								flag=1;		
							}
							else if(i>9)
							{break;}
							i++;				
						}
						i=0;
					}			   
				}
				closedir(dir2);
				if(flag==1){	
				cout<<ptr->d_name<<"/"<<input<<" ";
				sprintf(filepath,"/proc/%s/cmdline",ptr->d_name);
				ifstream argu;
				argu.open(filepath,ios::in);
				string input;
				getline(argu,input);
				stringstream ss(input);
				string sub_str;
				getline(ss,sub_str,'\0');							
				while(getline(ss,sub_str,'\0'))
				{	
					cout <<sub_str;
				}	
			   	cout<<endl;
			      }
			    }
			}	
		}		
	}
	closedir(dir);			
}

void argu_procudp(string input)
{	
	DIR *dir,*dir2;
        struct dirent *ptr,*ptr2;
	char filepath[50];
	char filetext[50];
	dir=opendir("/proc");
	int flag=0;
	if(NULL != dir)
	{
		while((ptr = readdir(dir)) != NULL)
		{	
	           if( ( isdigit(ptr->d_name[0])!=0))
		   {		
			sprintf(filepath,"/proc/%s/comm",ptr->d_name);
			ifstream prog;
			prog.open(filepath,ios::in);
			string readfrompro;
			getline(prog,readfrompro);
			if(readfrompro==input)
			{
				sprintf(filepath,"/proc/%s/fd",ptr->d_name);//cout<<ptr2->d_name<<endl;
				dir2=opendir(filepath);
				while((ptr2 = readdir(dir2))!= NULL)
				{
			       		sprintf(filepath,"/proc/%s/fd/%s",ptr->d_name,ptr2->d_name);
					readlink(filepath,filetext,sizeof(filetext));
					ifstream getip;
					string readfromip;
					getip.open("/proc/net/udp",ios::in);
					//cout <<"this is proname"<<readfrompro<<endl;
					while(getline(getip,readfromip))
					{	
						//cout<<"first loop"<<endl;
						stringstream parse(readfromip);
						string arr[20];
						int i=0;
						while(parse >>arr[i])
						{
							if(i==9 && (strstr(filetext,arr[9].c_str())!=NULL))
							{
								//cout <<filetext <<"\t"<<arr[9]<<endl;						
								string port;
								cout<<"udp\t";
								port=parse_port(arr,1);
								transform_iprint(arr,1);
								cout<<":";
								transform_portprint(port);
								port=parse_port(arr,2);
								transform_iprint(arr,2);
								cout<<":";
								transform_portprint(port);
								//cout<<"get";	
								flag=1;		
							}
							else if(i>9)
							{break;}
							i++;				
						}
						i=0;
					}			   
				}
				closedir(dir2);	
				if(flag==1){
				cout<<ptr->d_name<<"/"<<input<<" ";
				sprintf(filepath,"/proc/%s/cmdline",ptr->d_name);
				ifstream argu;
				argu.open(filepath,ios::in);
				string input;
				getline(argu,input);
				stringstream ss(input);
				string sub_str;
				getline(ss,sub_str,'\0');							
				while(getline(ss,sub_str,'\0'))
				{	
					cout <<sub_str;
				}	
			   	cout<<endl;
			      }
			    }
			}	
		}		
	}
	closedir(dir);			
}

char  short_options[10]="tu";
struct option long_options[]=
{
	{"tcp",0,NULL,'t'},
	{"udp",0,NULL,'u'},
	{0,0,0,0},
};
int main(int argc,char *argv[])
{
	int result;
	string arguop;
	if(argc==1){
			print_tcpinit();
			tcp_proc();
			tcp_proc6();
			print_udpinit();
			udp_proc();
			udp_proc6();
		}
	if(argc==2 && argv[1][0]!='-')
	{			print_tcpinit();
				arguop=argv[optind];
				argu_proc(arguop);
				print_udpinit();
				argu_procudp(arguop);
	}
	while((result = getopt_long(argc, argv, short_options, long_options, NULL))!=-1)
	{
		switch(result)
		{
		case 't':
			if(argc==2){
					print_tcpinit();
					tcp_proc();
					tcp_proc6();}
			else{
				print_tcpinit();
				arguop=argv[optind];
				argu_proc(arguop);}
			break;
		case 'u':
			if(argc==2){
					print_udpinit();
					udp_proc();
					udp_proc6();}
			else{
				print_udpinit();
				arguop=argv[optind];
				argu_procudp(arguop);}
			break;
		}
	}
return 0;
}


