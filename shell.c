#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

char cwd[1024];
// Function to print Current Directory and take input
int takeInput(char* str){
	//print Current Directory
	char* username = getenv("USER");
	char hostname[1024];
	hostname[1023]='\0';
	gethostname(hostname,1023);
	
	getcwd(cwd, sizeof(cwd));

	int root_len = 5+strlen(username);
	char *root  = (char*)malloc(sizeof(char)*root_len);
	strcpy(root,"/home/");
	strcat(root,username);

	int length = strlen(username)+strlen(hostname)+strlen(cwd)+21;
	char *result  = (char*)malloc(sizeof(char)*length);
	strcpy(result,"\n\033[32m");
	strcat(result,username);
	strcat(result,"@");
	strcat(result,hostname);
	strcat(result,"\033[m:");
	if(strcmp(cwd,root) == 0){
		char new_cwd[2] = "~";
		
		strcat(result,new_cwd);
	}
	else{
		strcat(result,cwd);
	}
	
	strcat(result,"$ ");

	//input
	char* buf;

    	buf = readline(result);
    	if (strlen(buf) != 0){
        	add_history(buf);
        	strcpy(str, buf);
        	return 0;
    	}
	else{
        	return 1;
    	}
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Suprotik Dey"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");

    return;
}

extern char** environ;
char* undefined[MAXLIST];
int numOfundefined=0;
void export(char** parsed){
	
	//printf("%s",parsed[1]);
	if(parsed[1]==NULL){
		char** var;
		for(var=environ;*var!=NULL;++var)
			printf("%s\n",*var);
		int i;		
		for(i=0;i<numOfundefined;i++){
			printf("%s\n",undefined[i]);
		}
		
	}
	else if(strcmp(parsed[1],"-p")==0){
		//printf("ppp\n");
		if(parsed[2]==NULL){
			char** var;
			for(var=environ;*var!=NULL;++var)
				printf("%s\n",*var);
			int i;		
			for(i=0;i<numOfundefined;i++){
				printf("%s\n",undefined[i]);
			}
			
		}
		else{
			putenv(parsed[1]);
			
		}
	}
	else if(strcmp(parsed[1],"-n")==0){
		//printf("nnn\n");
		if(parsed[2]!=NULL){
			int i,check=0;
			for(i=0;i<numOfundefined;i++){
				if(strcmp(undefined[i],parsed[2])==0){
					check=1;
					numOfundefined--;
				}
				if(check==1){
					undefined[i]=undefined[i+1];
				}
			}
			unsetenv(parsed[2]);
			
		}
		else{
			char** var;
			for(var=environ;*var!=NULL;++var)
				printf("%s\n",*var);
			int i;		
			for(i=0;i<numOfundefined;i++){
				printf("%s\n",undefined[i]);
			}
			
		}
	}
	else if(strcmp(parsed[1],"-f")==0 ){
		if(parsed[2]!=NULL){
			printf("bash: export: %s: not a function",parsed[2]);
			
		}
	}
	else if(parsed[1][0]=='-'){
		printf("bash: export: %s: invalid option\nexport: usage: export [-fn] [name[=value] ...] or export -p",parsed[1]);
	}
	else {
		
		char* copy = (char*)malloc(sizeof(char)*(strlen(parsed[1])+5));
		strcpy(copy,parsed[1]);
		char* exportNV[3];
		int i;
		for (i = 0; i < 3; i++) {
			exportNV[i] = strsep(&parsed[1], "=");
			if (exportNV[i] == NULL)
				break;
		}
		if(i==1){
			//printf("bash: export: `%s': not a valid identifier\n",parsed[1]);
			//printf("format: export [NAME]=[VALUE]\n");
			int i,check=0;
			for(i=0;i<numOfundefined;i++){
				if(strcmp(undefined[i],copy)==0){
					check=1;
					break;
				}
			}
			if(check==0 && getenv(copy)==NULL){
				undefined[numOfundefined++]=copy;
			}
		}
		else{
			int i,check=0;
			for(i=0;i<numOfundefined;i++){
				if(strcmp(undefined[i],exportNV[0])==0){
					check=1;
					numOfundefined--;
				}
				if(check==1){
					undefined[i]=undefined[i+1];
				}
			}
			putenv(copy);
		}
	}
}
// Function to execute builtin commands
int ownCmdHandler(char** parsed){

	int NoOfOwnCmds = 7, i, switchOwnArg = 0;
    	char* ListOfOwnCmds[NoOfOwnCmds];
    	char* username;

    	ListOfOwnCmds[0] = "exit";
    	ListOfOwnCmds[1] = "cd";
    	ListOfOwnCmds[2] = "help";
    	ListOfOwnCmds[3] = "hello";
	ListOfOwnCmds[4] = "export";
    	ListOfOwnCmds[5] = "echo";
    	ListOfOwnCmds[6] = "pwd";

    	for (i = 0; i < NoOfOwnCmds; i++) {
        	if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            	switchOwnArg = i + 1;
            	break;
        	}
    	}

    	switch (switchOwnArg) {
    	case 1:
        	printf("\nGoodbye\n");
        	exit(0);
    	case 2:
        	chdir(parsed[1]);
        	return 1;
    	case 3:
        	openHelp();
        	return 1;
    	case 4:
        	username = getenv("USER");
        	printf("\nHello %s.\nMind that this is "
            	"not a place to play around."
            	"\nUse help to know more..\n",
            	username);
        	return 1;
	case 5:
		export(parsed);
		return 1;
	case 6:
		printf("\n6\n");
		printf("\n");
		if (parsed[1] != NULL){
			printf("%s",parsed[1]);
		}
		int i;
		for (i = 2; i < MAXLIST; i++) {
			if (parsed[i] == NULL){
				break;
			}
			printf(" %s",parsed[i]);
			    
		}
		printf("\n");
		return 1;
	case 7:
		printf("\n7\n");
		printf("\n%s\n",cwd);
		return 1;

    	default:
        	break;
    	}

    	return 0;
}

// function for finding pipe
int parsePipe(char* str, char** strpiped){
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char* str, char** parsed, char** parsedpipe){

    char* strpiped[2];
    int piped = 0;

    piped = parsePipe(str, strpiped);

    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);

    } else {

        parseSpace(str, parsed);
    }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

int main(){
	char inputString[MAXCOM], *parsedArgs[MAXLIST];
	char* parsedArgsPiped[MAXLIST];
	int execFlag = 0;
	clear();
    	//init_shell();

    	while(1){
        	// print shell line and take input
		if (takeInput(inputString))
		    continue;
		// process
		execFlag = processString(inputString,
		parsedArgs, parsedArgsPiped);
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.

		// execute
		if (execFlag == 1)
		    execArgs(parsedArgs);

		if (execFlag == 2)//p
		    execArgsPiped(parsedArgs, parsedArgsPiped);
	}
	return 0;
}
