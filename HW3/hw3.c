//HW3
#define shell_readline_buffer 1024
#define shell_TOK_BUFSIZE 64
#define shell_TOK_DELIM " \t\r\n\a"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <wordexp.h>

//using namespace std;
// \t = tab \r = carriage  return \n = newline \a = alert (bell)


int shell_launch(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0)
	{	//child
		int in = 0, out = 0;
		char input[64];
		char output[64];

		setpgid(0, 0);
		//redirection
		for (int pivot = 0; args[pivot] != '\0'; pivot++)
		{
			if (strcmp(args[pivot], "<") == 0)
			{
				args[pivot] = NULL;
				strcpy(input, args[pivot + 1]);
				in = 1;
			}

			else if (strcmp(args[pivot], ">") == 0)
			{

				args[pivot] = NULL;
				strcpy(output, args[pivot + 1]);
				out = 1;

			}
		}
		if (in) {
			int fd0 = open(input, O_RDONLY, 0);
			dup2(fd0, STDIN_FILENO);
			close(fd0);
			in = 0;
		}
		if (out) {

			int fd1 = creat(output, 0644);
			dup2(fd1, STDOUT_FILENO);
			out = 0;
		}
		//printf("%s %s\n",args[0], args[1]);
		//if(strcmp(args[0],"export")==0)printf("same\n");
		if (execvp(args[0], args) == -1)
		{
			perror("shell error execvp");
		}
		exit(-1);
	}
	else if (pid < 0)
	{
		perror("shell error");
	}
	else
	{
		setpgid(pid, pid);
		signal(SIGTTOU, SIG_IGN);
		tcsetpgrp(STDIN_FILENO, pid);
		//parent
		//do
		//{
		//	wpid = waitpid(pid, &status, WUNTRACED);
		//}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		// this two functions, if error return 0 , so if 0 , stop
		while (!(wait(&status) == pid)) ;
		if (WEXITSTATUS(status) == 255)
			printf("Child crash\n");
		tcsetpgrp(STDIN_FILENO, getpid());
		signal(SIGTTOU, SIG_DFL);
	}
	return 1;
}

int shell_execute(char **args)
{
	int pivot = 0;
	if (args[0] == NULL)
	{
		return 1;//empty command
	}

	else if (strcmp(args[pivot], "export") == 0)
	{
		if (args[pivot + 1] == NULL)
		{
			args[pivot] = "printenv";
			return shell_launch(args);
		}
		else
		{
			char *tmp = strstr(args[pivot + 1], "=");
			if (tmp != NULL)
			{
				pivot++;
				char *name = malloc (30 * sizeof(char)) ;
				char *value = malloc (30 * sizeof(char));
				name = strtok(args[pivot], "=");
				value = strtok(NULL, "=");
				int i = setenv(name, value, 1);
				return (i ^ 1);
			}
			else
			{
				int i = setenv(args[pivot + 1],"''", 1);
				return (i ^ 1);
			}

		}
	}
	else if (strcmp(args[pivot], "unset") == 0)
	{
		if (args[pivot + 1] == NULL)
		{
			printf("you have no enough arguments\n");
			return 1;
		}
		else
		{
			int i = unsetenv(args[pivot + 1]);
			return ( i ^ 1);
		}
	}
	else
	{
		int bufsize = 30;
		pivot=0;
		int pivot2=0;
		char **expand = malloc(bufsize * sizeof(char *));
		while(args[pivot]!=NULL)
		{
			wordexp_t p;
			char **w;
			//printf("123\n");
			wordexp(args[pivot], &p, 0);
			w = p.we_wordv;
			for (int j = 0; j < p.we_wordc; j++)
			{
				expand[pivot2] = malloc(bufsize+1 * sizeof(char));
				strcpy(expand[pivot2],w[j]);
				pivot2++;
				//printf("pivot2 : %d\n",pivot2);
			}
			pivot++;
				//cmd.push_back(strdup(w[j]));
			wordfree(&p);
		}
		expand[pivot2+1]=NULL;
		//printf("%s  %s \n",expand[0],expand[1]);
		return shell_launch(expand);
		//cmd.push_back(NULL);
	}
	return shell_launch(args);
}

char **shell_split_line(char *line)
{
	int bufsize = shell_TOK_BUFSIZE;
	int position = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if (!token)
	{
		fprintf(stderr, "shell: allocation error \n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, shell_TOK_DELIM);
	while (token != NULL)
	{
		tokens[position] = malloc((strlen(token) + 1 ) * sizeof(char));
		tokens[position] = token;
		position++;

		if (position >= bufsize)
		{
			bufsize += shell_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens)
			{
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}//exceed bufsize, allocate more space
		token = strtok(NULL, shell_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}
char *shell_read_line(void)
{
	/*int bufsize = shell_readline_buffer;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if(!buffer)
	{
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE); //EXIT_FAILURE = 1
	}
	while(1)
	{
		//read a char
		c = getchar();

		//when EOF ,replace it with a null
		if(c == EOF || c == '\n')
		{
			buffer[position] = '\0';
			return buffer;
		}
		else{
			buffer[position] = c;
		}
		position++;

		//if exceeded the buffer, reallocate

		if (position >= bufsize)
		{
			bufsize += shell_readline_buffer;
			buffer = realloc(buffer, bufsize);
			if(!buffer)
			{
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}*/
	char *line = NULL;
	ssize_t bufsize = 0; // have getline allocate a buffer for us
	getline(&line, &bufsize, stdin);
	return line;
}


void shell_loop(void)
{
	char *line;
	char **args;
	int status;
	do {
		printf("> ");
		line = shell_read_line();
		args = shell_split_line(line);
		//printf("456\n");
		status = shell_execute (args);
		//printf("123\n");
		//free(line);
		//free(args);
	} while (status);
}


int main(int argc, char **argv)
{

	shell_loop();
	return 0;
}