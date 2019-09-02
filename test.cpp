#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sstream>
#include <sys/wait.h>
#include<iostream>
#include <unistd.h>
#include <sys/types.h>

/*
Use these colors to print colored text on the console
*/
#define GREEN   "\x1b[32m"
#define BLUE    "\x1b[34m"

using namespace std;

//----Global Variables---
bool historyread = true;

//------------------------------------------------Def------------------

static void alarmHandler(int signo);


//-----------------------------------------------------------------------

void printPrompt(){
    // We print the prompt in the form "<user>@<host> <cwd> >"
    char hostn[1204] = "";
    char cwd[1024];
    uid_t s = getuid();
    gethostname(hostn, sizeof(hostn));
    printf(GREEN "%d@%s %s ## " GREEN, s, hostn, getcwd(cwd, 1024));
}

void welcomeScreen(){
    printf(BLUE "\n\t====================OS ASSignment========================\n" BLUE);
    printf("\n\n");
}

void executeBasic(char** argv){
	pid_t pid;
	pid = fork();
	if(pid == 0){
		if((execvp(argv[0],argv) < 0)) printf("command not found");
		exit(1);
	}
	else{
		//waitpid(pid, 0, 0);
		wait(NULL);
	}
}

static void alarmHandler(int signo){
    printf("Alarm signal sent!\n");

	}


void historyfiles(char **token){
	int fd;
	//cout << s;
	char cwd[100];
	char files[100];
    if(getcwd(cwd,100) == NULL) perror("getcwd() error");
    char *path = cwd;
    strcat(path,"/commands.txt");
    int i=0;


	
	if(historyread){
		//int fd;	
    	fd = open(path , O_CREAT,S_IRUSR|S_IWUSR);
    	close(fd);
    	historyread = false;
	}
	
		//int fd;
		fd=open(path,O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
		int w;
		const char *ch = " ";
		const char *nline = "\n";
    	while(token[i] != NULL){
    	//files[i] = token[i];
    		w = write(fd,token[i],strlen(token[i]));
    		w = write(fd,ch,strlen(ch));
    		i++;
    	}
    	w = write(fd,nline,strlen(nline));
		close(fd);
	

}


void executefwd(char **token){
	
	char *temp[10];
	int i=0;
	string t = ">";

    while(token[i] != t){        		
       		temp[i] = (char *)malloc(sizeof(token[i]) + 1);
      		strcpy(temp[i],token[i]);
      		i++;
       	}
       	temp[i] = NULL;
    	string file2 = token[++i];
    
    	if(fork()>0)
    	{
    		wait(NULL);
    	}
    	else{
    		int fd = open(file2.c_str(), O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
			dup2(fd,1);
			if((execvp(temp[0],temp) < 0)) printf("command not found");
			exit(1);
    	}

	}
void executeappdirec(char **token){
	char *temp[10];
	int i=0;
	string t = ">>";
	while(token[i] != t){
		temp[i] = (char *)malloc(sizeof(token[i]) + 1);
      	strcpy(temp[i],token[i]);
      	i++;
	}
	temp[i] = NULL;  
	string file2 = token[++i];
	if(fork()>0)
    	{
    		wait(NULL);
    	}
    	else{
    		int fd = open(file2.c_str(), O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
			dup2(fd,1);
			if((execvp(temp[0],temp) < 0)) printf("command not found");
			exit(1);
    	}
}


int main()
{

	printf("\033[2J\033[1;1H");
	fflush(stdout);
	welcomeScreen();
	// global variable clear

  	int status; 
  	while(1){
		
  		char command[1024];
  		bool fwddirec = false; bool appdirec= false; bool backgroundpro = false; bool presentwork = false;

		char *token[1024];
  		int e,numTokens=0;
  		
  		printPrompt();
  		
  		fgets(command,1024,stdin);

  		if(command[0]== '\n') continue; ///nothing entere

  		
  		else{
        	char *tokenpointer = strtok(command,"  \n");
        	while(tokenpointer != NULL){
        		
        		token[numTokens] = (char *)malloc(sizeof(tokenpointer) + 1);
        		strcpy(token[numTokens],tokenpointer);
        		tokenpointer = strtok(NULL,"  \n");
        		numTokens++;
        	}
        	token[numTokens] = NULL;
        	historyfiles(token);

  			for(int i=0;i<sizeof(command);i++)
  			{

  				if(command[i] == '>' && command[i+1] == '>'){
  					appdirec = true;
  					break;
  				}
  				else if(command[i] == '>'){
  					fwddirec = true;
  					break;
  				}
  				else if(command[i] == '&'){
  					backgroundpro = true;
  					break;
  				}
  			}

  			if(fwddirec){
  				executefwd(token);

  			}
  			else if(appdirec){
  				executeappdirec(token);

  			}
  			

        	//Alarm detection
        	else if(strstr(token[0],"alarm")){
        		int seconds = atoi(token[1]);
        		alarm(seconds);
        		signal(SIGALRM, alarmHandler);
        	}
        	// cd is the first value
       		else if(strstr(token[0],"cd"))
       			{

       				chdir(token[1]);
       				}
       		else if(strstr(token[0],"pwd")){
       			char cwd[100];
       			if(getcwd(cwd,100) != NULL) printf("%s\n", cwd);
       			else perror("getcwd() error");
       		}
       				
       		else{
       			executeBasic(token);
       		}

       	}	

  	}
  return 0;
}

