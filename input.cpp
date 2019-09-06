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
#include <fstream>
#include<vector>
#include<unordered_map>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <string>

#include <fstream>
#include<vector>
#include<unordered_map>

using namespace std;


#define DEL (127)
#define ASCII_ESC 27

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

   bool isLastNode(Trie *node) 
  { 
      for(auto i = node->map.begin();i!=node->map.end();i++)
      {
          if (i->first) return 0; 
          }
          return 1; 
  } 

   void printsuggestion(Trie *node,string word,vector<string> &ss){
    if(node == NULL) return;
    if(node->isleaf) ss.push_back(word);
      if(isLastNode(node)) return;

    for(auto i = node->map.begin();i!=node->map.end();i++)
    {
      
      printsuggestion(i->second , word + (i->first),ss);
    }
   }



   vector<string> printstring(Trie *node , string word){
        vector<string> ss;
        int len = word.length();
        for(int i=0;i<len;i++)
        {
          if(node->map.find(word[i]) == node->map.end()){
            return {};////////////////////////////////////////////////prefix not present
          }
          node = node->map[word[i]];
        }

        bool leaf = node->isleaf;

        bool lastnode = isLastNode(node);

        if(leaf && lastnode){
          ss.push_back(word); return ss;}

          if(!lastnode){
            printsuggestion(node,word,ss);
          }

          return ss;
      }

void printPrompt(){
    string s= "$$";
    write(STDOUT_FILENO, s.c_str(),s.size());
}

void display_options(Trie *root,char* buffer,int top_buffer){

  string temp;
  for(int i=0;i<top_buffer;i++)
  {
    temp = temp + buffer[i];

  }
  vector<string> ss = printstring(root,temp);
  char ch[] = "\n";
  for(int i=0;i<ss.size();i++)
  {
    write(STDOUT_FILENO,ch,strlen(ch)); 
    write(STDOUT_FILENO,ss[i].c_str(),ss[i].length());
  }
  
  printPrompt();
  write(STDOUT_FILENO,buffer,top_buffer);

}

void disableRawMode(struct termios initial_state) {
  tcsetattr(STDIN_FILENO, TCSADRAIN, &initial_state);
}

struct termios enableRawMode() {
  struct termios initial_state;
  tcgetattr(STDIN_FILENO, &initial_state);
  struct termios raw = initial_state;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSADRAIN, &raw);

  return initial_state;
}

int logkey(Trie *root,char* buffer,int &top_buffer) {
  int status;
  char c;
  int flag=1;
  int enter_flag=1;
  while ((status = read(STDIN_FILENO, &c, 1)) != 1);
  switch(c){
    case DEL:{
      if(top_buffer>0){
        --top_buffer;
        write(STDOUT_FILENO, "\033[1D",4);
        write(STDOUT_FILENO, "\033[0K",4);
      }
      flag=0;
      break;
    }

    case '\t':{
      display_options(root,buffer,top_buffer);
      flag=0;
      break;
    }
    case '\n':{
      write(STDOUT_FILENO, "\n", 1);
      flag=0;
      enter_flag=0;
      break;
    }
  }

  if(!iscntrl(c) && flag){
          write(STDOUT_FILENO,&c, 1);
          buffer[top_buffer++]=c;}

  if(!enter_flag)
      buffer[top_buffer]='\0';

  return enter_flag;
}
void sendinput(Trie *root,char* buffer){
  int it=1;
  int top_buffer=0;
  printPrompt();
  while (it) {
    it=logkey(root,buffer,top_buffer);
  }
}

