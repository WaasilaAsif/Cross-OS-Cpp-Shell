#include<iostream>
#include <string>
#include <vector>
#include <cstring>;
#include <sys/types.h> // Defines pid_t
#include <unistd.h>    // Defines fork() and execvp()
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/types.h> // Defines pid_t
#include <unistd.h>    // Defines fork() and execvp()
#include <sys/wait.h>   // Defines waitpid()
#endif

using namespace std;
void lsh_loop();
string lsh_read_line();
vector <string> lsh_split_line(string line);
bool lsh_execute(vector<string> args);
int main(int argc, char **argv){

    lsh_loop();

    return EXIT_SUCCESS;
}
void lsh_loop(){
     string line;
    vector <string> args;
    int status;

    do {
        cout<<"> ";
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
    }
    while (status);
};
string lsh_read_line(){
    string line = "";
    getline(cin, line);
    if(cin.eof()){
        exit (EXIT_SUCCESS);

    }
    else {
        perror("lsh: read line error");
        exit (EXIT_FAILURE);
    }
    return line;
}
vector <string> lsh_split_line(string line) {
    vector<string> tokens ;
    string current_token;
    while(1){
        if(line.empty()){
            cout<<"> lsh: allocation error. No command entered"<<endl<<">"
            ;
            return tokens;

        }
    
        for(int i = 0 ; i < line.length(); i++){
           char c = line[i];
           
           if (c == ' ' || c == '\t' || c == '\n' || c == '\r'){
            if (!current_token.empty()){
                tokens.push_back(current_token);
                current_token="";

            }}
            else{
                current_token += c;
            }
            
           }
            if (!current_token.empty()){
                tokens.push_back(current_token);
            }

        
    }
    return tokens;
}

int lsh_launch(vector<string> args){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0){
        //Child process is running
        
    }
}