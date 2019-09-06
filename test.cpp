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
#include <fstream>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <fstream>
#include<vector>
#include<unordered_map>
#include "input.cpp"
/*
Use these colors to print colored text on the console
*/
#define GREEN   "\x1b[32m"
#define BLUE    "\x1b[34m"

using namespace std;

#define DEL (127)
#define ASCII_ESC 27
string ps1;

//////////////////////////////////////////////////////////////////////////////////////////////////Global Variable
vector<string> res; //store etc passwrd variables
//res0 = username , x , 1000, 1000, Pratik , home , bon/bash , PATH
extern char **environ;
char *env_arr[]={NULL};
char *args[10];
char *token[10];
char *stoken[10];
char command[100];
char command2[100];
int status=1;
bool pipeinput = false;bool backgroundinput = false;
bool equalinput = false;bool nodirectcommands = false;
bool appdirect = false;bool fwdirect = false;
unordered_map<string,string> localmap;


/////////////////////////////////////////////////////////////////////////////only used in pipe
void removespace(int i){

	string s = token[i];
	stringstream ss(s);
	
	string t="";
	int j=0;
	while(ss >> t){
		args[j]= NULL;
		args[j] = (char *)malloc(t.size()+1);
		strcpy(args[j],t.c_str());
		j++;
	}
	args[j] = NULL;
}

vector<string> split (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}/////////////////////////////////////////////////////////////////////////////used in myrcfile



void tokensplit(string delimiter){
	int numTokens=0;
  			char *tokenpointer = strtok(command,delimiter.c_str());
        	while(tokenpointer != NULL){
        		token[numTokens] = (char *)malloc(sizeof(tokenpointer) + 1);
        		strcpy(token[numTokens],tokenpointer);
        		tokenpointer = strtok(NULL,delimiter.c_str());
        		//cout << stoken[numTokens] << endl;
        		numTokens++;
        		}	
        		token[numTokens] = NULL;
}

void welcomeScreen(){
    printf(BLUE "\n\t====================OS ASSignment========================\n" BLUE);
    printf("\n\n");
}

void myrcfile(){

	string etcpath = "/etc/passwd";
	uid_t u = getuid();
	stringstream uu;
	uu << u;
	string userid;
	uu >> userid;
	string myhome,t,temp;
	ifstream etcpasswd;
	etcpasswd.open(etcpath);
    while(getline(etcpasswd,t)){
           if(strstr(t.c_str(),userid.c_str())){ /////////////////////////////////////add string
           	temp = t;
           	break;
           }
    }
    string delimiter = ":";
    vector<string> v = split(temp, delimiter);
    myhome = v[5];
    string iduser = v[0];

    string envpath = "/etc/environment";
    string env;
    ifstream e;
    e.open(envpath);
    getline(e,env);
    int pos = env.find("=");
    res.push_back(env.substr(pos+1));////////////////////////////////////////////////////Everything in res vector from Userid till PATH

    /////////////////////////////////////////////////////////////////////////////////////create my rc file
    string path = res[5] + "/" + res[0] + ".txt";
	const char *nline = "\n";

	/////////////////////////////////////////////////////////////////////////// check if file created earlier or not
	if(open(path.c_str(), O_RDONLY,S_IRUSR|S_IWUSR)>0){
		//donot create
	}
	else{
		string ps = "PS1=Default";
		res.push_back(ps);
		int in = open(path.c_str(), O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
		for(int i=0;i<res.size();i++){
			write(in,res[i].c_str(),res[i].size());
			write(in,nline,strlen(nline));
				}
			close(in);
			setenv("PATH",env.c_str(),1);
			setenv("LOGNAME",res[0].c_str(),1);
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////file created till PS1=Default
		/////////////////////////////////////////////////////////////////////////////////////////////////Read File and store in localmap
		string pp;
		ifstream readd;
		readd.open(path);
		vector<string> vv ={"$username","$xxx","$userid","$groupid","$name","$HOME","$binbash","$PATH"};
		vector<string> others;
		int k=0;
		while(getline(readd,pp)){
			if(k<=7) localmap[vv[k]] = pp;
			else{
				others.push_back(pp);
			}
			k++;
		}


		for(int i=0;i<others.size();i++)
		{
      string key,value;
      if(i==0){
        key = "$" + others[i].substr(0,3);
        cout << key << endl;
        value = others[i].substr(4);
        localmap[key] = value;
        cout << localmap[key] << endl;
      }
      else{
        stringstream space;
        space << others[i];
        string temp;
        space >> temp;
        if(temp == "alias"){
            space >> temp;
            key = temp;
            space >> temp;
            value = temp;
            while(space >> temp){
              value = value + " " + temp;
            }
          }
        else{
            key = "$" + temp;
            space >> temp;
            value = temp;
            while(space >> temp){
              value = value + " " + temp;
            }
        }
        localmap[key] = value;
      }
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////Add Path into Envrionment
		setenv("PATH",localmap["$PATH"].c_str(),1);
 		setenv("LOGNAME",localmap["$name"].c_str(),1);////////////////////////////////////////////////////////Environment set

	} //final is File Created with system details, PS1 , and Global Variables


void clearvariables(){
    	status=1;
    	pipeinput = false;
    	backgroundinput = false;
    	equalinput = false;
    	nodirectcommands = false;
    	appdirect = false;
    	fwdirect = false;
    	for(int i=0;i<10;i++)
    	{
    		args[i] = NULL;
    		token[i] = NULL;
    		stoken[i] = NULL;
    	}
    	command[0] = '\0';
    	command2[0] = '\0';
}


void executebasic(){
	if(fork()==0){ execvp(stoken[0], stoken);}
    else{wait(NULL);}
}

void executepipe()
{	
	int p[2];
	pipe(p);
	string s = "|";
	tokensplit(s);
	int i=0;
	int f=0;
	while(token[i] != NULL){
		auto pid =fork();
		if(pid == 0)
		{
			dup2(f,0); // /////////////////////////////////////////////////initially 
			if(token[i+1] != NULL) dup2(p[1],1); ///////////////////////////second wale ka right open kr diya
			close(p[0]);
			close(p[1]);
			removespace(i);
			////////////////////check for fwd reverse
			if(execvp(args[0],args) == -1){
				cout << "Not" << endl;
			}	
			exit(0);
		}
		else{

			wait(NULL);
			close(p[1]);
			f = p[0];
			i++;

		}
	}
}

void executefwd(){
	char *temp[10];
	int i=0; 
	string t = ">";

    while(stoken[i] != t){        		
       		temp[i] = (char *)malloc(sizeof(stoken[i]) + 1);
      		strcpy(temp[i],stoken[i]);
          //cout << temp[i] << endl;
      		i++;
       	}
       	temp[i] = NULL;
    	string file2 = stoken[++i];
      cout << file2 << endl;
    	if(fork()>0)
    	{
    		wait(NULL);
    	}
    	else{
    		int fd = open(file2.c_str(), O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
			dup2(fd,1);
			if((execvp(temp[0],temp) < 0)) printf("command not found");
			
    	}

	}

void executeappdirec(){
	char *temp[10];
	int i=0;
	string t = ">>";
	while(stoken[i] != t){
		temp[i] = (char *)malloc(sizeof(stoken[i]) + 1);
      	strcpy(temp[i],stoken[i]);
      	i++;
	}
	temp[i] = NULL;  
	string file2 = stoken[++i];
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

void directcommands(){

		  	int numTokens=0;
  			char *tokenpointer = strtok(command,"  \n");
          string aliastemp = tokenpointer;
          if(localmap.find(aliastemp) != localmap.end()){
                stringstream ss;
                ss << localmap[aliastemp];
                string temp;
                while(ss >> temp){
                  stoken[numTokens] = (char *)malloc(temp.size());
                  strcpy(stoken[numTokens],temp.c_str());
                  cout << stoken[numTokens] << endl;
                  numTokens++;
                }
                tokenpointer = strtok(NULL,"  \n");
             }
             
            while(tokenpointer != NULL){
            stoken[numTokens] = (char *)malloc(sizeof(tokenpointer) + 1);
            strcpy(stoken[numTokens],tokenpointer);
            if(stoken[numTokens][0] == '~'){

              string te = "/home/pratik";
              string t = stoken[numTokens];
              t = t.substr(1);
              te.append(t);
              stoken[numTokens] = (char *)malloc(te.length()+1);
              strcpy(stoken[numTokens],te.c_str());
            }
            tokenpointer = strtok(NULL,"  \n");
            numTokens++;
            } 
            stoken[numTokens] = NULL;


          //checkfor direct commands exit,pwd,echo,cd,clear,alarm


        		if(strstr(stoken[0],"cd")) chdir(stoken[1]);
       			
            else if(strstr(stoken[0],"pwd")){
         				 char cwd[20];
         				 char *arr[] = {(char *)"pwd", NULL};
         				 if(fork()==0){
         					execvp(arr[0], arr);}
         					else{wait(NULL);}
         				//else perror("pwd() error");
       			}
       			
            else if(strstr(stoken[0],"exit")){
       				cout << "Byeeee\n";
       				status =0;
       			}
       			
            else if(strstr(stoken[0],"echo")){
       			//redirect to echo
               				if(strstr(command2,"$$")){
               					cout << getpid() << endl;
               				}
                      else if(strchr(command2,'~')){
                        cout << localmap["$HOME"] << endl;
                      }
               				else if(strstr(command2,"$?")){
               					//////////////////////////////////////////////////////////////////////////pending
               				}
               				else if(strchr(command2,'$')){
               					string temp = stoken[1];
               					cout << localmap[temp] << endl;
               		
               				}
               				else{
               					if(fork()==0) execvp(stoken[0],stoken);
               						else{
               						wait(NULL);
               						}
               					}
       			}
       			else if(strstr(stoken[0],"alarm")){
       			//redirect to echo
       			}
       			else if(strstr(stoken[0],"export")){
       			//redirect to eexport
       			}
       			else if(strstr(stoken[0],"alias")){///////////////////////////////////////////alias l ls -l
       			//redirect to alias
       				string key = stoken[1];
       				string value = "";
   					  int i=3;
   					  value = value + stoken[2];
   					  while(stoken[i] != NULL){
   						 value = value + " " + stoken[i];
   						 i++;
   					  }
   					  localmap[key] = value;
       			}
       			else{
      				//execute basic commands 	
      				nodirectcommands = true;		
       			}

      }


int main(){
	environ = env_arr;
  myrcfile();
  	setenv("TERM","xterm-256color",1);
  	printf("\033[2J\033[1;1H");
  	fflush(stdout);
  	welcomeScreen();

    for(auto i=localmap.begin();i!=localmap.end();i++)
    {
      cout << i->first << " " << i->second << endl;
    }


	while(status){
          if(localmap["$PS1"] == "Default"){
            if(localmap["$userid"] == "0") ps1 = "$$";
            else ps1 = "##";

            localmap["$PS1"] = ps1;
          }
          else{
            ps1 = localmap["$PS1"];
          }

		  clearvariables();
		  Trie *root= new Trie();
      vector<string> v = {"/usr/local/sbin","/usr/local/bin","/usr/sbin","/usr/bin","/sbin","/bin","/usr/games","/usr/local/games"};
      DIR *directory;
      for(int i=0;i<v.size();i++)
      {
          directory = opendir(v[i].c_str());
          struct dirent *dd;
      
          while(dd=readdir(directory)){
              string temp = dd->d_name;
              root->insert(temp);
        } 
      
      }
    struct termios initial_state=enableRawMode();
    sendinput(root,command);
    disableRawMode(initial_state);
    strncpy(command2, command, sizeof(command));

		/////////////////////////////////////////////////////////////////////////check for pipe , & , = sign
		if(strchr(command,'=')){ ///////////////////////////////////////////////////////////format  x=5 x=$HOME
			  equalinput = true;
				string s = "=";
				tokensplit(s);
				int i=1;
        cout << token[0] << endl;
        if(token[0] == "$PS1"){

          cout << "hi" << endl;
          localmap["$PS1"] = token[1];
        }
				string ss="";
				string key = "";
				key = key + "$";
				key = key + token[0];
        if(token[i][0] = '$'){
          localmap[key] = localmap[token[i]];

        }
        else{
          while(token[i] != NULL){
          ss = ss + token[i];
          i++;
        }
        localmap[key] = ss;
        }
				
		}
		else if(strchr(command,'|')){ ///////////////////////////////////////////////////////ls -l | grep a | sort
			pipeinput = true;
			cout << "pipe" << endl;
			executepipe();

		}
		else if(strchr(command,'&')){ 
			// backgroundinput = true;
		}

  		if(command[0]== '\0' || equalinput==true || pipeinput==true || backgroundinput == true) continue;

  		else{
  			directcommands();
  			
  			 // if(strchr(command,'$')) cout << "yes" << endl;
  			if(nodirectcommands){
  				//input in stoken for separated via space
  				//check what stoken contains > or >>
  				for(int i=0;i<sizeof(command)-1;i++){
 
  					if(command[i] == '>' && command[i+1] == '>'){
  						appdirect = true;
  						executeappdirec();
  						break;
  					}
  					else if(command[i] == '>'){
  						fwdirect = true;
  						executefwd();
  						break;
  					}	
  				}
  				if(!appdirect && !fwdirect) executebasic();

  			}
  		}


	}

return 0;

}