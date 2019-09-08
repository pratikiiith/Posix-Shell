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
#include<unordered_map>
#include <dirent.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <fstream>
#include<vector>

using namespace std;


#define DEL (127)
#define ASCII_ESC 27
extern string ps1;

class Trie {
  public:
      unordered_map<char,Trie*> map = {};
      bool isleaf = false;

  public:

      Trie(){}   
      void insert(string word) {
          Trie* node = this;
          
          for(char ch : word){  
              if(node->map.find(ch) == node->map.end()){
                  node->map[ch] = new Trie();
              }
              node=node->map[ch];
          }
          node->isleaf = true;
      }
      
      bool search(string word) {
          Trie* node= this;
          for(char ch:word){
             if(!node->map.count(ch)) return false;
              node=node->map[ch];
          }
          return node->isleaf;
      }

      
  };

   void printsuggestion(Trie *node,string word,vector<string> &ss){
    if(node == NULL) return;
    if(node->isleaf) ss.push_back(word);
    for(auto i = node->map.begin();i!=node->map.end();i++) {printsuggestion(i->second , word + (i->first),ss);}

   }

   vector<string> printstring(Trie *node , string word){
        vector<string> ss;
        int len = word.length();
        for(int i=0;i<len;i++)
        {
          if(node->map.find(word[i]) == node->map.end()) return {};
          node = node->map[word[i]];
           }
        if(node->isleaf && (node->map.size()==0)){
          ss.push_back(word); return ss;}

          if(node->map.size() != 0){
            printsuggestion(node,word,ss);
          }

          return ss;
      }


/////////////////////////////////////////////////////////////////////////////////////////Termios Key Input open
struct termios enableRawMode() {
  struct termios instate;
  tcgetattr(0, &instate);
  struct termios terminalstate = instate;
  terminalstate.c_lflag &= ~(ECHO | ICANON);
  terminalstate.c_cc[VMIN] = 0;
  terminalstate.c_cc[VTIME] = 1;
  tcsetattr(0, TCSADRAIN, &terminalstate);

  return instate;
}

void disableRawMode(struct termios instate) {
  tcsetattr(0, TCSADRAIN, &instate);
}


int keyinput(Trie *root,char* command) {

	int it=1,pos=0;
    string s= ps1;
    write(1, s.c_str(),s.size());
  	while (it) {
  		int status,flag=1;
		  char c;
		  while ((status = read(1, &c, 1)) != 1);
		  
	switch(c){

		 case '\n':{
	      	write(1, "\n", 1);
	      	flag=0;
	      	command[pos]='\0';
	      	it=0;
	      	break;}
		  
		  case DEL:{
		      if(pos>0){
		        --pos;
		        write(1, "\033[1D",4);
		        write(1, "\033[0K",4);
		      		}
		      	flag=0;
		      	break;
		    	}

	    case '\t':{
	       	string temp;
	        for(int i=0;i<pos;i++)
	        {
	          temp = temp + command[i];

	        }
	        vector<string> ss = printstring(root,temp);
	        char ch[] = "\n";
	        for(int i=0;i<ss.size();i++)
	        {
	          write(1,ch,strlen(ch)); 
	          write(1,ss[i].c_str(),ss[i].length());
	        }
	        
	        string s= ps1;
	        write(1, s.c_str(),s.size());
	        write(1,command,pos);
	      	flag=0;
	      	break;}    
	  }

	  if(!iscntrl(c) && flag){
	          write(1,&c, 1);
	          command[pos++]=c;}

  		}
	}

///////////////////////////////////////////////////////////////////////////////////////////Termios Key Input close
