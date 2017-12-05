#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "util.h"

#define LIMIT 256 // max number of tokens for a command
#define MAXLINE 1024 // max number of characters from user input





void signalHandler_child(int p){
//prevent zombie

	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
	printf("\n");
}


void signalHandler_int(int p){
	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		
		no_reprint_prmpt = 1;			
	}
	
	
}


void signalHandler_quit(int p){
	kill(pid,SIGTERM);
	exit(0);
}


void pipeHandler(char * args[]){

	int filedes[2]; 
	int filedes2[2];
	
	int num_cmds = 0;
	
	char *command[256];
	
	pid_t pid;
	
	int err = -1;
	int end = 0;
	

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	

	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;
	

	while (args[j] != NULL && end != 1){
		k = 0;

		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				end = 1;
				k++;
				break;
			}
			k++;
		}

		command[k] = NULL;
		j++;		

		if (i % 2 != 0){
			pipe(filedes); 
		}else{
			pipe(filedes2); 
		}
		
		pid=fork();
		
		if(pid==-1){			
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]);
				}else{
					close(filedes2[1]); 
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){

			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}

			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){ 
					dup2(filedes[0],STDIN_FILENO);
				}else{ 
					dup2(filedes2[0],STDIN_FILENO);
				}

			}else{ 
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{ // for even i
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}		
		}
				
		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){					
				close(filedes[0]);
			}else{					
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){					
				close(filedes2[0]);
				close(filedes[1]);
			}else{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
				
		waitpid(pid,NULL,0);
				
		i++;	
	}
}

int changeDirectory(char* args[]){

	if (args[1] == NULL) {
		chdir(getenv("HOME")); 
		return 1;
	}

	else{ 
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}


void fileIO(char * args[], char* inputFile, char* outputFile, int option){
	 
	int err = -1;
	
	int fileDescriptor; 
	
	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0){

		if (option == 0){

			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 

			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);

		}else if (option == 1){

			fileDescriptor = open(inputFile, O_RDONLY, 0600);  

			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);

			if(outputFile!=NULL){
				fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
				dup2(fileDescriptor, STDOUT_FILENO);
				close(fileDescriptor);	
			}			
		}
		 
		setenv("parent",getcwd(currentDirectory, 1024),1);
		

		
		if (execvp(args[0],args)==err){
			printf("err");
			kill(getpid(),SIGTERM);
		}		 
	}
	waitpid(pid,NULL,0);
}


void launchProg(char **args, int background){	 
	 int err = -1;
	 
	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }

	if(pid==0){

		signal(SIGINT, SIG_IGN); 

		setenv("parent",getcwd(currentDirectory, 1024),1);	

		if (execvp(args[0],args)==err){
			printf("Command not found");
			kill(getpid(),SIGTERM);
		}
	 }

	 if (background == 0){
		 waitpid(pid,NULL,0);
	 }else{

		 printf("Process created with PID: %d\n",pid);
	 }	 
}


int commandHandler(char * args[]){
	int i = 0;
	int j = 0;
	
	int fileDescriptor;
	int standardOut;
	
	int aux;
	int background = 0;
	
	char *args_aux[256];

	while ( args[j] != NULL){
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0)){
			break;
		}
		args_aux[j] = args[j];
		j++;
	}
	

	if(strcmp(args[0],"exit") == 0) exit(0);
 	

	else if (strcmp(args[0],"clear") == 0) system("clear");

	else if (strcmp(args[0],"cd") == 0) changeDirectory(args);

	else{

		while (args[i] != NULL && background == 0){

			if (strcmp(args[i],"&") == 0){
				background = 1;

			}else if (strcmp(args[i],"|") == 0){
				pipeHandler(args);
				return 1;

			}else if (strcmp(args[i],"<") == 0){
				aux = i+1;
				if (args[aux] == NULL || args[aux+1] == NULL || args[aux+2] == NULL ){
					printf("Not enough input arguments\n");
					return -1;
				}else{
					if (strcmp(args[aux+1],">") != 0){
						printf("Usage: Expected '>' and found %s\n",args[aux+1]);
						return -2;
					}
				}
				fileIO(args_aux,args[i+1],args[i+3],1);
				return 1;
			}

			else if (strcmp(args[i],">") == 0){
				if (args[i+1] == NULL){
					printf("Not enough input arguments\n");
					return -1;
				}
				fileIO(args_aux,NULL,args[i+1],0);
				return 1;
			}
			i++;
		}

		args_aux[i] = NULL;
		launchProg(args_aux,background);
		
	}
return 1;
}


void shellPrompt(){

	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

int main(int argc, char *argv[], char ** envp) {
	act_child.sa_handler = signalHandler_child;
	act_int.sa_handler = signalHandler_int;		
	act_quit.sa_handler = signalHandler_quit;	
	

	sigaction(SIGCHLD, &act_child, 0);
	sigaction(SIGINT, &act_int, 0);
	sigaction(SIGQUIT, &act_quit, 0);	

  struct termios old_termios, new_termios;
  tcgetattr(0,&old_termios);


  new_termios             = old_termios;
  new_termios.c_cc[VQUIT]  = 4; // ^C
  new_termios.c_cc[VINTR] = 3; // ^D
  tcsetattr(0,TCSANOW,&new_termios);

	
	
	
	
	char line[MAXLINE]; 
	char * tokens[LIMIT]; 
	int numTokens;
		
	no_reprint_prmpt = 0; 	
							
	pid = -10;


	environ = envp;
	

	setenv("shell",getcwd(currentDirectory, 1024),1);

	while(TRUE){

		if (no_reprint_prmpt == 0) shellPrompt();
		no_reprint_prmpt = 0;
		

		memset ( line, '\0', MAXLINE );

		fgets(line, MAXLINE, stdin);

		if((tokens[0] = strtok(line," \n\t")) == NULL) continue;

		numTokens = 1;
		while((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) numTokens++;
		
		commandHandler(tokens);
		
	}          

	exit(0);
}