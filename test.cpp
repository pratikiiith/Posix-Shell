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
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <string>
#include <fstream>
#include <ctime>
#include<queue>
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
int child_pipe[2];
bool alll = true;
pid_t alarmpid;
vector<time_t> alarmvector;

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

  string etcpath = "/etc/passwd"; /////////////////////////////////////////////////////////////////Store all variables in res vector
  uid_t u = getuid();
  stringstream uu;
  uu << u;
  string userid;
  uu >> userid;
  string myhome,t,temp;
  ifstream etcpasswd;
  etcpasswd.open(etcpath);
    while(getline(etcpasswd,t)){
           if(strstr(t.c_str(),userid.c_str())){ 
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
    res.push_back(env.substr(pos+1));
    ////////////////////////////////////////////////////////Everything in res vector from Userid till PATH
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
    vector<string> vv ={"$username","$pwd","$userid","$groupid","$name","$HOME","$binbash","$PATH"};
    vector<string> others;
    int k=0;
    while(getline(readd,pp)){ 
      if(k<=7) localmap[vv[k]] = pp; ////////////////////////////////////////storing values from rc file to local map onyl env variables
      else if(pp[0] == '.'){         ////////////////////////////////////////storing .format files to local map " .mp4=/usr/bin/vlc "     
        string key = pp.substr(0,4);
        string value = pp.substr(5);
        localmap[key] = value;
      }
      else{
        others.push_back(pp); ///////////////push PS1, alias and other global variable to others vector
      }
      k++;
    }


    for(int i=0;i<others.size();i++) //////////////////////////////////////////////////////////storing values of rc file to local map
    {
      string key,value;
      if(i==0){///////////////////////////////////////////////////////////////////////////////////$PS1 Default stored in local map
        key = "$" + others[i].substr(0,3);
        cout << key << endl;
        value = others[i].substr(4);
        localmap[key] = value;
        cout << localmap[key] << endl;
      }
      else{/////////////////////////////////////////////////////alias ll ls -l //////////////x 5 //////ll->ls -l /// $x->5
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
          while(command2[i] != '\0'){
            s = s + command2[i];
            i++;
          }
          write(logfd,s.c_str(),s.size());
          write(logfd,ch,strlen(ch));
          while(int n = read(p[0], buffer,1) > 0)
          {
           write(logfd, buffer,1);//////////////////////////////////////////////////////////////////////////////output command
           write(1,buffer,1);
          }
          write(logfd,ch,strlen(ch));
          close(logfd);
          _exit(127);
          }

        close(p[0]);   
  }
  
  else{

    if(fork()==0){ if(execvp(stoken[0], stoken) < 0) cout <<"Command Not Valid\n"; exit(1); }
    else{
      int errorlo;
        wait(&errorlo);
        if( WIFEXITED(errorlo)){
          error=  WEXITSTATUS(errorlo);
        }
    }
  }

}

// void executepipe()
// { 
//   int p[2],i=0,f=0;
//   pipe(p);
//   string s = "|";
//   tokensplit(s);
//   while(token[i] != NULL){
//     auto pid =fork();
//     if(pid == 0)
//     {
//       dup2(f,0); // /////////////////////////////////////////////////initially 
//       if(token[i+1] != NULL) dup2(p[1],1); ///////////////////////////second wale ka right open kr diya
//       close(p[0]);
//       close(p[1]);
//       removespace(i);
//       ////////////////////check for fwd reverse
//       if(execvp(args[0],args) == -1){
//         cout << "Command Not Found" << endl;
//       } 
//       exit(0);
//     }
//     else{
//       wait(NULL);
//       close(p[1]);
//       f = p[0];
//       i++;

//     }
//   }
// }

void executepipe()
{
  string st1 = localmap["$HOME"] + "/pipe1";
  string st2 = localmap["$HOME"] + "/pipe2";
  int fd1 = open(st1.c_str(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
  int fd2 = open(st2.c_str(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
  close(fd1);
  close(fd2);
  string s = "|";
  tokensplit(s);
  int i=0;
  while(token[i]!=NULL){
    i++;
  }
  int countpipe = i-1;
  cout << countpipe << endl;
  for(int j=0;j<=countpipe;j++)
  {

    removespace(j);
    if(j==0)
      {
        fd1 = open(st1.c_str(),O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
        
        if(fork()==0){dup2(fd1,1);if(execvp(args[0],args) == -1){cout << "Command Not Found" << endl;}}
        else
          { wait(NULL);close(fd1);}
         
      }

    else if(j==countpipe){
        if(j%2 != 0){
          fd1 = open(st1.c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
         
          if(fork()==0){ dup2(fd1,0);if(execvp(args[0],args) == -1){cout << "Command Not Found" << endl;}}
          else
          { wait(NULL);close(fd1);}
        }
        else{
           
            fd2 = open(st2.c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
           
            if(fork()==0){ dup2(fd2,0);if(execvp(args[0],args) == -1){cout << "Command Not Found" << endl;}}
             else { wait(NULL);close(fd2);}
        }
      }

    else{
        if(j%2 != 0){
          fd1 = open(st1.c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
          fd2 = open(st2.c_str(),O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
          if(fork()==0){  dup2(fd2,1);dup2(fd1,0);if(execvp(args[0],args) == -1){cout << "Command Not Found" << endl;}}
          else{
            wait(NULL);
            close(fd1);
            close(fd2);}
          }
        else{
            fd1 = open(st1.c_str(),O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
            fd2 = open(st2.c_str(),O_RDONLY,S_IRUSR|S_IWUSR);
            if(fork()==0){ dup2(fd1,1);dup2(fd2,0);if(execvp(args[0],args) == -1){cout << "Command Not Found" << endl;}}
            
            else{
              close(fd1);
            close(fd2);}
      }
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
      exit(1);
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
                    if(fork()==0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else if(strstr(stoken[1],".txt")){
                  stoken[0] = (char *)malloc(localmap[".txt"].size());
                    strcpy(stoken[0],localmap[".txt"].c_str());
                    if(fork()==0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                    
                }
                else if(strstr(stoken[1],".pdf")){
                  stoken[0] = (char *)malloc(localmap[".pdf"].size());
                    strcpy(stoken[0],localmap[".pdf"].c_str());
                    if(fork()==0){
                      execvp(stoken[0],stoken);
                    }
                    else{
                      wait(NULL);
                    }
                }
                else if(strstr(stoken[1],".png")){
                  stoken[0] = (char *)malloc(localmap[".png"].size());
                    strcpy(stoken[0],localmap[".png"].c_str());
                    if(fork()==0){
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
                 char *arr[] = {(char *)"pwd", NULL};
                 if(fork()==0){
                  execvp(arr[0], arr);}
                  else{wait(NULL);}
                //else perror("pwd() error");
            }
            
            else if(strstr(stoken[0],"exit")){
              
              if(alarmvector.size()>0){
                time_t currt = time(nullptr);
                string path = localmap["$HOME"] + "/alarm";
                int fd = open(path.c_str(),O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
                char ch[]="\n";
                for(int i=0;i<alarmvector.size();i++)
                {
                  if(alarmvector[i] > currt){
                    string s= to_string(alarmvector[i]);

                    write(fd,s.c_str(),s.size());
                    write(fd,ch,strlen(ch));
                  }
                }         
                
              }

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
                            cout << exportmap[temp] << endl;
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
                    

                    pipe(child_pipe);
                    alarmpid = fork();
                      if(alarmpid == 0){
                          priority_queue <time_t, vector<time_t>, greater<time_t> > q;
                          int value;
                          int status=1;

                          while(status){    
                              
                              time_t result = time(nullptr);  
                              close(child_pipe[1]);
                              if(read(child_pipe[0],&value,sizeof(value))){
                                cout << result << " " << value << " " << result+value << endl;
                                q.push(result+value);
                                cout << q.top() << endl;
                            }
                            sleep(2);
                            result = time(nullptr);
                            if(!q.empty() && result >= q.top()){
                              cout << "Alarm Raised " << result << endl;
                              q.pop();
                              _exit(127);
                            }
                          }
                      }
                      else{
                      alll = false;
                      int s = atoi(stoken[1]);
                      time_t temp = time(nullptr);
                      alarmvector.push_back(temp+s);
                      write(child_pipe[1],&s,sizeof(s));
                      close(child_pipe[1]);

                      }       
            
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


void historfile()
{
  char ch[]="\n";
  string path = localmap["$HOME"] + "/historycommands.txt";
  int fd = open(path.c_str(), O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  int i=0;
  string com = "";
  while(command[i] != '\0'){
      com = com + command[i];
        i++;
  }
  write(fd,com.c_str(),com.size());
  write(fd,ch,strlen(ch));
  close(fd);
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
     string alaramhistory = localmap["$HOME"] + "/alarm"; 
     if(open(alaramhistory.c_str(),O_RDONLY,S_IRUSR|S_IWUSR)>0){

          string pp;
          ifstream readd;
          readd.open(alaramhistory);
          while(getline(readd,pp)){
            cout << "Missed alarms are " << pp << endl;
          }
          open(alaramhistory.c_str(),O_TRUNC|O_RDONLY|O_WRONLY,S_IRUSR|S_IWUSR);
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
      keyinput(root,command);
      disableRawMode(initial_state);
      strncpy(command2, command, sizeof(command));
      historfile();

      /////////////////////////////////////////////////////////////////////////check for pipe , & , = sign
      if(strchr(command,'=')){ ///////////////////////////////////////////////////////////format  x=5 x=$HOME
          equalinput = true;
          string s = "=";
          tokensplit(s);
          int i=1;
          string ps = token[0];
          string pa = token[0];
          if(ps == "$PS1"){
            localmap["$PS1"] = token[1];
          }
          else if(pa == "$PATH"){
            string path = localmap["$PATH"];
            int len = path.length();
            string newpath = token[1];
            newpath = ":" + newpath;
            path.insert(len-1,newpath);
            setenv("PATH",path.c_str(),1);
            localmap["$PATH"] = path;
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
            if(exportmap.find(key) != exportmap.end()){
              exportmap[key] = ss;
               localmap[key] = ss;
            }
            else{
              localmap[key] = ss; 
            }  
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