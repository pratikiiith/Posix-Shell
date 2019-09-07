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
char *env_arr[1024]={NULL};
char *args[10];
char *token[10];
char *stoken[10];
char command[100];
char command2[100];
int status=1;
bool pipeinput = false;bool backgroundinput = false;
bool equalinput = false;bool nodirectcommands = false;
bool appdirect = false;bool fwdirect = false;
bool record = false;
unordered_map<string,string> localmap;
unordered_map<string,string> exportmap;
int error;


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
    while(getline(readd,pp)){ ////////////////////////////////////////////////////////////////////storing values from rc file to local map
      if(k<=7) localmap[vv[k]] = pp;
      else if(pp[0] == '.'){
        string key = pp.substr(0,4);
        string value = pp.substr(5);
        localmap[key] = value;
      }
      else{
        others.push_back(pp);
      }
      k++;
    }


    for(int i=0;i<others.size();i++) //////////////////////////////////////////////////////////storing values of rc file to local map
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

    ////////////////////////////////////////////////////////////////////////////////////////////Check export file present or not.
    int partid = getppid();
    stringstream ppid;
    ppid << partid;
    string parentid;
    ppid >> parentid; 
    string exportpath = localmap["$HOME"] + "/" + parentid + ".txt"; 
    if(open(exportpath.c_str(),O_RDONLY,S_IRUSR|S_IWUSR) > 0){
    	string ep;
    	ifstream exportread;
    	exportread.open(exportpath);
    	while(getline(exportread,ep)){
    		string key , value;
    		key = ep.substr(0,1);
    		key = "$" + key;
    		value = ep.substr(2);
    		exportmap[key] = value;
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

  if(record == true){
      int p[2];
      pipe(p);
      if (fork() == 0)
      {
          dup2(p[1], 1);
          close(p[0]);
          close(p[1]);
          execvp(stoken[0],stoken);
          _exit(127);
          
          }
            close(p[1]);


          if (fork() == 0)
          {
          string path = localmap["$HOME"] + "/" + "log.txt";
          int logfd = open(path.c_str(), O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
          char buffer[1];
          int ssize=0;
          char ch[] ="\n";
          int i=0;
          string s= "";
          while(stoken[i] != NULL){///////////////////////////////////////////////////////////////////////input commands
            string temp = stoken[i];
            s = s + temp;
            i++;
          }
          write(logfd,ch,2);
          while(int n = read(p[0], buffer,1) > 0)
          {
           write(logfd, buffer,1);//////////////////////////////////////////////////////////////////////////////output command
           write(1,buffer,1);
          }
          write(logfd,ch,2);
          close(logfd);
          _exit(127);
          }

        close(p[0]);   
  }
  
  else{
    if(fork()==0){ execvp(stoken[0], stoken);}
    else{
      int errorlo;
        wait(&errorlo);
        if( WIFEXITED(errorlo)){
          error=  WEXITSTATUS(errorlo);
        }
    }
  }

}

void executepipe()
{ 
  int p[2],i=0,f=0;
  pipe(p);
  string s = "|";
  tokensplit(s);
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
      if(fork()>0)
      {
        wait(NULL);
      }
      else{
        int fd = open(file2.c_str(), O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
      dup2(fd,1);
      if((execvp(temp[0],temp) < 0)) printf("Command Not found");
      
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
		/////////////////////////////////////////////////////////////////////handle alias , ~ and create space delimeted tokens
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

            else if(strstr(stoken[0],"record")){ //////////////////////////////record on , record off

                if(strstr(stoken[1],"on")) record = true;
                else  record = false;
            }

            else if(strstr(stoken[0],"./a.out")){
            	cout << "here at ./a.out" << endl;
            	if(exportmap.size() != 0){
            		int id = getpid();
            		string sss = to_string(id);
            		string exportfilepath = localmap["$HOME"] + "/" + sss + ".txt";
            		int cr = open(exportfilepath.c_str(), O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
            		 char ch[] = "\n";
            		for(auto it=exportmap.begin();it!=exportmap.end();it++)
            		{
            			string s = it->first + "=" + it->second;
            			s=s.substr(1);
            			write(cr,s.c_str(),s.size());
            			write(cr,ch,strlen(ch)); 
            			//write(cr,newline.c_str(),2);
            		}
            		close(cr);	

            	}
            	executebasic();
            }
            
            else if(strstr(stoken[0],"open")){
                if(strstr(stoken[1],".mp4")){
                    stoken[0] = (char *)malloc(localmap[".mp4"].size());
                    strcpy(stoken[0],localmap[".mp4"].c_str());
                    if(fork()>0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else if(strstr(stoken[1],".txt")){
                  stoken[0] = (char *)malloc(localmap[".txt"].size());
                    strcpy(stoken[0],localmap[".txt"].c_str());
                    if(fork()>0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else if(strstr(stoken[1],".pdf")){
                  stoken[0] = (char *)malloc(localmap[".pdf"].size());
                    strcpy(stoken[0],localmap[".pdf"].c_str());
                    if(fork()>0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else if(strstr(stoken[1],".png")){
                  stoken[0] = (char *)malloc(localmap[".png"].size());
                    strcpy(stoken[0],localmap[".png"].c_str());
                    if(fork()>0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else{
                  cout << "Format Not Supported" << endl;
                }
            }

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
              exit(0);
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
                        cout <<"Command exit status is "  << error << endl;
                      }
                      else if(strchr(command2,'$')){
                        string temp = stoken[1];
                        if(exportmap.find(temp) != exportmap.end()){
                        }
                        else{
	                        cout << localmap[temp] << endl;
                        }
                      }

                      else{
                        if(fork()==0) execvp(stoken[0],stoken);
                          else{
     // printf("\033[2J\033[1;1H");
                          wait(NULL);
                          }
                        }
            }
            else if(strstr(stoken[0],"alarm")){
            //redirect to echo
            }
            else if(strstr(stoken[0],"export")){
            //redirect to eexport
            	string temp = stoken[1];
            	temp = "$" + temp;

            	exportmap[temp] = localmap[temp];

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

void recordfeature()
{

}

int main(){
     environ = env_arr;
    myrcfile();
     setenv("DISPLAY",":0",1);
    setenv("TERM","xterm-256color",1);
    setenv("DBUS_SESSION_BUS_ADDRESS","unix:path=/run/user/1000/bus",1);
    setenv("XDG_RUNTIME_DIR","/run/user/1000",1);
    setenv("XDG_SESSION_PATH","/org/freedesktop/DisplayManager/Session0",1);
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
    // historfiles();

    /////////////////////////////////////////////////////////////////////////check for pipe , & , = sign
    if(strchr(command,'=')){ ///////////////////////////////////////////////////////////format  x=5 x=$HOME
        equalinput = true;
        string s = "=";
        tokensplit(s);
        int i=1;
        if(token[0] == "$PS1"){

          localmap["$PS1"] = token[1];
        }
        else{
        	string ss="";
        	string key = "";
        	key = key + "$";
        	key = key + token[0];
        	if(token[i][0] == '$'){
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
        
        
    }
    else if(strchr(command,'|')){ ///////////////////////////////////////////////////////ls -l | grep a | sort
      pipeinput = true;
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