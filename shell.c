#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h> 
#include <ctype.h>
#include <errno.h>

// Declaring functions to be used :-

char **get_input(char *); // Tokenizer
char **command; 
int historyCount = 0;
char history_list[5][128]; //list defined for storing most recent 5 commands
// char *input;
pid_t child_pid;
int stat_loc;
char start_cwd[PATH_MAX];	// home directory address
int sizeStart;				// home directory address size
int start();

int main(){
    start();
    return 0;
}

int start() {
    
    if (getcwd(start_cwd, sizeof(start_cwd)) != NULL) {
        // Good
    }
    else {
        perror("getcwd() error \n");
        return 1;
    }
    sizeStart = strlen(start_cwd);

    while (1) {
    	// reading the current working directory into cwd
        char cwd[PATH_MAX];
	   	if (getcwd(cwd, sizeof(cwd)) != NULL) {
	   	} else {
	       	perror("getcwd() error \n");
	       	return 1;
	   	}
	   	// is child checks if the current directory is the subdirectory of starting directory or not
        int ischild = 1;
        int temp1 = 0;
        while(temp1 < sizeStart){
            if(cwd[temp1] == start_cwd[temp1]){

            }
            else{
                ischild = 0;
                break;
            }
            temp1++;
        }
        if(ischild == 0){
            printf("MTL458:%s$ ", cwd);    
        }
        else{
            printf("MTL458:~");
            temp1 = sizeStart;
            while(temp1 < strlen(cwd)){
                printf("%c", cwd[temp1]);
                temp1++;
            }
            printf("$ ");
        }
        // Taking input commands in the form of characters

		char str[129];
        int c = 0;
        int iter = 0;
        do{
            c = getchar();
            if(c!= '\n'){
                str[iter] = c;
            }
            else{
                str[iter] = '\0';
            }
            iter++;
        }while(c != '\n');
        if(strlen(str) > 128){
            perror("character limit(128) exceeded.");
            continue;
        }
        else{
        	// Using originalStr as an input for cd command as it requires some tweaks (in case of spaces int the address).
            char originalStr[strlen(str)]; 
            strcpy(originalStr, str);
            // Implementation of history command
            if(strcmp(str, "history") == 0){
                int tempHistorycnt = 0;
                while(tempHistorycnt < 5 && tempHistorycnt < historyCount){
                    printf("%d %s\n", tempHistorycnt+1, history_list[tempHistorycnt]);
                    tempHistorycnt++;
                }

                if(historyCount == 5){
                    int temp2 = 0;
                    while(temp2 < 4){
                        strcpy(history_list[temp2], history_list[temp2 + 1]);
                        temp2++;
                    }
                    strcpy(history_list[4], originalStr);
                }
                else{
                    strcpy(history_list[historyCount], originalStr);
                    historyCount++;
                }

                continue;
            }
            // Adjust the input command so that blank spaces dont cause an issue while tokenizing
            char str1[strlen(str) + 1]; 
            int myboolean = 0;
            int ppt = 0;
            while(ppt < strlen(str)){
                if(myboolean == 0){
                    if(str[ppt] == '\"'){
                        myboolean = 1;
                        str1[ppt] = '\n';
                    }
                    else if(str[ppt] == ' '){
                        str1[ppt] = '\n';   
                    }
                    else{
                        str1[ppt] = str[ppt];
                    }
                }
                else if(myboolean == 1){
                    if(str[ppt] == '\"'){
                        myboolean = 0;
                        str1[ppt] = '\n';
                    }
                    else{
                        str1[ppt] = str[ppt];
                    }
                }
                ppt++;
            }
            str1[ppt] = '\0';
            // creating string address for cd command
            char address_to_go[strlen(str)];
            int pp = 0;
            int pp2 = 3;
            if(strlen(str) <=  2){
                // leave it
            }
            else{
                while(pp2 < strlen(str)){
                    // printf("Moshi moshi2\n");
                    while(str[pp2] == '\"'){
                        pp2++;
                        if(pp2 == strlen(str)){
                            break;
                        }
                    }
                    if(pp2 == strlen(str)){
                        break;
                    }
                    address_to_go[pp] = str[pp2];
                    pp++;
                    pp2++;
                }
                address_to_go[pp] = '\0';
            }
            // printf("Moshi moshi2\n");
            
            // printf("Moshi moshi3\n");
            command = get_input(str1);

            
            if (!command[0]) {      /* Handle empty commands */
                // free(input);
                free(command);
                continue;
            }
            // printf("Moshi moshi\n");

            //for history cmd

            if(historyCount == 5){
                int temp2 = 0;
                while(temp2 < 4){
                    strcpy(history_list[temp2], history_list[temp2 + 1]);
                    temp2++;
                }
                strcpy(history_list[4], originalStr);
            }
            else{
                strcpy(history_list[historyCount], originalStr);
                historyCount++;
            }
            if (strcmp(command[0], "cd") == 0) {
                // printf("%s\n", *address_to_go);
                if (address_to_go[0] == '~'){
                    chdir(start_cwd);
                }
                else{
                    if (chdir(address_to_go) < 0) {
                        perror(address_to_go);
                    }    
                }
                /* Skip the fork */
                continue;
            }
            // forking the process to use launchable commands in the environment

            child_pid = fork();
            if (child_pid < 0) {
                perror("Fork failed");
                exit(1);
            }

            if (child_pid == 0) {
                /* Never returns if the call is successful */
                if(command[0] == NULL){
                    perror("EmptyLine\n");
                    return 1;
                }
                else if (execvp(command[0], command) < 0) {
                    perror(command[0]);
                    exit(1);
                }
            } else {
                waitpid(child_pid, &stat_loc, WUNTRACED);
            }

            free(command);
        }
        
    }
    start();
    return 0;
}
char **get_input(char *input) {
	// Tokenizing the input
    char **command = malloc(8 * sizeof(char *));
    if (command == NULL) {
        perror("malloc failed");
        exit(1);
    }
    // printf("%s\n", input);
    // separator are the breakpoints used to tokenize the given string here I used '\n' for tokenizing
    char *separator = "\n";
    char *parsed;
    int index = 0;
    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index] = parsed;
        // printf("%s\n", parsed);
        index++;
        parsed = strtok(NULL, separator);   

        
    }

    command[index] = NULL;
    return command;
}


/*
References : 
https://indradhanush.github.io/blog/writing-a-unix-shell-part-2/
https://indradhanush.github.io/blog/writing-a-unix-shell-part-3/

*/