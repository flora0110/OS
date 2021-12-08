#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

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
void output_redirection(char** args){
	//printf("in output_redirection!!!\n");
	char *cp;
    	char *ifile;
    	char *ofile;
	char **args2;
    	int i;
    	int j;
    	int err;
	int pid;
	int app=0;
	args2 = malloc(80 * sizeof(char *));
	// assume no redirections
        ofile = NULL;
        ifile = NULL;

        // split off the redirections
        j = 0;
        i = 0;
        err = 0;
        while (1) {
	    //printf("i=%d\n",i);
            cp = args[i++];
	    //printf("cp %s\n",cp);
            if (cp == NULL)
                break;

            switch (*cp) {
            case '<':
                if (cp[1] == 0)
                    cp = args[i++];
                else
                    ++cp;
                ifile = cp;
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            case '>':
		//printf("is > !!\n");
		//printf("cp[1] is %c\n",cp[1]);
                if (cp[1] == 0){//only one char(only >)
		    //printf("if\n");
                    cp = args[i++];}
		else if(cp[1]=='>'){
			//printf("find app\n");
			app=1;
			//++cp;
			cp = args[i++];
			if(cp[2]!=0){++cp;}
		}
                else{//between > and file no block
		    //printf("else\n");
                    ++cp;}
                ofile = cp;
		//printf("ofile %s\n",ofile);
                if (cp == NULL)
                    err = 1;
                else
                    if (cp[0] == 0)
                        err = 1;
                break;

            default:
                args2[j++] = cp;
                break;
            }
        }
        args2[j] = NULL;

        // we got something like "cat <"
        if (err)
            return;
        // no child arguments
        if (j == 0)
            return;

        switch (pid = fork()) {
        case 0:
            // open stdin
            if (ifile != NULL) {
                int fd = open(ifile, O_RDONLY);

                if (dup2(fd, STDIN_FILENO) == -1) {
                    fprintf(stderr, "dup2 failed");
                }

                close(fd);
            }

            // open stdout
            if (ofile != NULL) {
		int fd2;
		if(app==1){
			//printf("append way to open\n");
			if ((fd2 = open(ofile, O_APPEND | O_WRONLY, 0644)) < 0) {
		            perror("couldn't open output file.");
		            exit(0);
		        }
		}
                else{
		        if ((fd2 = open(ofile, O_WRONLY | O_CREAT, 0644)) < 0) {
		            perror("couldn't open output file.");
		            exit(0);
		        }
		}
		//printf("ofile3 %s %ld\n",ofile,strlen(ofile));
		//printf("kkk\n");
		//printf("okay\n");
                // args+=2;
                //printf("okay");
                dup2(fd2, STDOUT_FILENO);
                close(fd2);
            }

            execvp(args2[0], args2);        /* child */
            signal(SIGINT, SIG_DFL);
            fprintf(stderr, "ERROR s no such programn");
            exit(1);
            break;

        case -1:
            /* unlikely but possible if hit a limit */
            fprintf(stderr, "ERROR can't create child process!n");
            break;

        default:
            //printf("am I here");
            wait(NULL);
            //waitpid(pid, 0, 0);
        }
}

// Function where the system command is executed
int backpid[MAXLIST];
int backpid_index=0;
char* cmd[MAXLIST];
char* input_copy;
int background,status;

void sig_handler(int signo){
	pid_t pid=waitpid(pid,&status,WNOHANG);
	//if(background==1){
		int i,j;
		for(i=0;i<backpid_index;i++){
			if(backpid[i]==pid){
				backpid[i]=-backpid[i];
				for(j=0;j<strlen(cmd[i]);j++){
					if(cmd[i][j]=='&') cmd[i][j]='\0';
				}
				break;
			}
		}
		
	//}
	
}
void execArgs(char** parsed)
{
    // Forking a childS
    if(parsed[0]==NULL) return;
    int i,status;
    background=0;
    for(i=0;i<MAXLIST;i++){
	if(parsed[i]==NULL){
		break;
	}
    }
    if(strcmp(parsed[i-1],"&")==0){
	background=1;
	parsed[i-1]=NULL;	
    }
    if(signal(SIGCHLD, sig_handler)){
	//perror("signal sigchiled error\n");
    }
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
	//printf("getpid %d\n",getpid());
	if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
	//printf("pid %d\n",pid);
	//printf("background %d\n",background);
	if(background==0) waitpid(pid,&status,0);
	else{
		
		backpid[backpid_index]=pid;
		cmd[backpid_index] = (char*)malloc(sizeof(char)*100);
		strcpy(cmd[backpid_index++],input_copy);
		printf("[%d] %d\n",(backpid_index),pid);
	}
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


extern char** environ;
char* undefined[MAXLIST];
int numOfundefined=0;
void export(char** parsed,int or,int block){
	FILE* output_file=stdout;
	if(or==1){
		//printf("file name %s\n",parsed[block+1]);
		if(parsed[block][1]=='>'){
			output_file = fopen(parsed[block+1],"a+");
		}
		else output_file = fopen(parsed[block+1],"w+");
	}
	//printf("%s",parsed[1]);
	if(parsed[1]==NULL || parsed[1][0]=='>'){
		char** var;
		for(var=environ;*var!=NULL;++var)
			//printf("%s\n",*var);
			fprintf(output_file,"%s\n",*var);
		int i;		
		for(i=0;i<numOfundefined;i++){
			//printf("%s\n",undefined[i]);
			fprintf(output_file,"%s\n",undefined[i]);
		}
		
	}
	else if(strcmp(parsed[1],"-p")==0){
		//printf("ppp\n");
		if(parsed[2]==NULL){
			char** var;
			for(var=environ;*var!=NULL;++var)
				//printf("%s\n",*var);
				fprintf(output_file,"%s\n",*var);
			int i;		
			for(i=0;i<numOfundefined;i++){
				//printf("%s\n",undefined[i]);
				fprintf(output_file,"%s\n",undefined[i]);
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
				//printf("%s\n",*var);
				fprintf(output_file,"%s\n",*var);
			int i;		
			for(i=0;i<numOfundefined;i++){
				//printf("%s\n",undefined[i]);
				fprintf(output_file,"%s\n",undefined[i]);
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
		//no =
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
		//there has =
		else{
			//check if var already in undefined
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
			//check if value include $
			check=0;int start;
			for(i=0;i<strlen(copy);i++){
				//printf("copy %c\n",copy[i]);
				if(copy[i]=='$'){
					check=1;start=i;
					i++;
					break;
				}
			}
			//printf("i%d\n",i);
			
			if(check==1){
				char* name=(char*)malloc(sizeof(char)*30);
				int q=0;
				if(copy[i]=='{') i++;
				for(i;i<strlen(copy);i++){
					//printf("2 copy %c\n",copy[i]);
					if(copy[i]=='}'){
						name[q]='\0';
						break;
					}
					if(copy[i]==':'){
						i--;
						name[q]='\0';
						break;
					}
					name[q++]=copy[i];
					//printf("value %c\n",value[i]);
				}
				//printf("name is %s, len is %ld\n",name,strlen(name));
				char* value=getenv(name);
				int new_len=strlen(copy)+strlen(value);
				char* new_copy = (char*)malloc(sizeof(char)*new_len);
				int j,k,copy_index=0;
				//printf("start %d new_copy %c\n",start,copy[start]);
				for(j=0;j<new_len;j++){
					if(j==start){
						for(k=0;k<strlen(value);k++){
							new_copy[j++]=value[k];
						}
						j--;copy_index=i+1;
						//printf("j at %d\n",j);
					}
					else new_copy[j]=copy[copy_index++];
				}
				//printf("new_copy %s\n",new_copy);
				putenv(new_copy);
			}
			else if(check==0) putenv(copy);
		}
	}
	if(or==1) {//printf("export os 1\n");
		fclose(output_file);}
}
void getvar(char* parsedBlock,FILE* output_file,int or){
	char* cp=parsedBlock;
	cp++;
	if(or==1) fprintf(output_file,"%s",getenv(cp));
	else printf("%s",getenv(cp));
}
// Function to execute builtin commands
int ownCmdHandler(char** parsed,int or,int block){

	int NoOfOwnCmds = 9, i, switchOwnArg = 0;
    	char* ListOfOwnCmds[NoOfOwnCmds];
    	char* username;

    	ListOfOwnCmds[0] = "exit";
    	ListOfOwnCmds[1] = "cd";
    	
    	//ListOfOwnCmds[3] = "hello";
	ListOfOwnCmds[2] = "export";
    	ListOfOwnCmds[3] = "echo";
    	ListOfOwnCmds[4] = "pwd";
	ListOfOwnCmds[5] = "jobs";
	ListOfOwnCmds[6] = "kill";
	ListOfOwnCmds[7] = "bg";
	ListOfOwnCmds[8] = "history";
    	for (i = 0; i < NoOfOwnCmds; i++) {
        	if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            	switchOwnArg = i + 1;
		//printf("%d\n",(i+1));
            	break;
        	}
    	}
	FILE* output_file;
	int status,done,start_to_del=-1,check_all_stop=0,p=-1,m=-1;
	char plus_minus=' ';
	HISTORY_STATE *myhist = history_get_history_state();
	HIST_ENTRY **mylist = history_list();
    	switch (switchOwnArg) {
    	case 1:
        	//printf("\nGoodbye\n");
        	exit(0);
    	case 2:
        	chdir(parsed[1]);
        	return 1;
    	/*case 4:
        	username = getenv("USER");
        	printf("\nHello %s.\nMind that this is "
            	"not a place to play around."
            	"\nUse help to know more..\n",
            	username);
        	return 1;*/
	case 3:
		export(parsed,or,block);
		return 1;
	case 4:
		//printf("or1 %d\n",or);
		//if has outputorection set out_file
		if(or==1){
			if(parsed[block][1]=='>'){
				output_file = fopen(parsed[block+1],"a+");
			}
			else output_file = fopen(parsed[block+1],"w+");
			//fprintf(output_file,"%s\n",cwd);
			//fclose(output_file);
			//return 1;
		}
		printf("\n");
		//printf("or2 %d\n",or);
		if (parsed[1] != NULL){
			//" start
			if(parsed[1][0]==34){
				int i;
				for(i=1;i<strlen(parsed[1])-1;i++){
					if(or==1) fprintf(output_file,"%c",parsed[1][i]);
					else printf("%c",parsed[1][i]);
				}
				//check first blocktail is " or not
				if(parsed[1][i]!=34){
					//printf("%c",parsed[1][i]);
					if(or==1) fprintf(output_file,"%c",parsed[1][i]);
					else printf("%c",parsed[1][i]);
				}
				//printf("before for\n");
				//check every blocktail is " or not
				for (i = 2; i < MAXLIST; i++) {
					
					if (parsed[i] == NULL || parsed[i][0]=='>'){
						break;
					}
					if(or==1) fprintf(output_file," ");
					else printf(" ");
					//printf("parsed[%d]=%s\n",i,parsed[i]);
					int j;
					for(j=0;j<strlen(parsed[i])-1;j++){
						//printf("%c",parsed[1][j]);
						if(or==1) fprintf(output_file,"%c",parsed[i][j]);
						else printf("%c",parsed[i][j]);
					}
					if(parsed[i][j]!=34){
						//printf("%c",parsed[1][j]);
						if(or==1) fprintf(output_file,"%c",parsed[i][j]);
						else printf("%c",parsed[i][j]);
					}
					
				}
				//printf("\n");
				if(or==1) {fprintf(output_file,"\n");fclose(output_file);}
				else printf("\n");
				return 1;
			}
			else if(parsed[1][0]=='$'){
				char* cp=parsed[1];
				cp++;
				if(or==1) fprintf(output_file,"%s",getenv(cp));
				else printf("%s",getenv(cp));
			}
			else{
				//printf("%s",parsed[1]);
				if(or==1) fprintf(output_file,"%s",parsed[1]);
				else printf("%s",parsed[1]);
			}
			int i;
			for (i = 2; i < MAXLIST; i++) {
				if (parsed[i] == NULL || parsed[i][0]=='>'){
					break;
				}
				if(parsed[i][0]=='$'){
					getvar(parsed[i],output_file,or);
				}
				else{
				//printf(" %s",parsed[i]);
				if(or==1) fprintf(output_file," %s",parsed[i]);
				else printf(" %s",parsed[i]);}
				    
			}
			if(or==1) {//printf("case 4 \n");
				fprintf(output_file,"\n");
				fclose(output_file);}
			else printf("\n");
		}
		
		return 1;
	case 5:
		//printf("in case 5\n");
		if(or==1){
			//FILE* output_file;
			if(parsed[block][1]=='>'){
				output_file = fopen(parsed[block+1],"a+");
			}
			else output_file = fopen(parsed[block+1],"w+");
			fprintf(output_file,"%s\n",cwd);
			fclose(output_file);
		}
		else{
			printf("\n%s\n",cwd);
			
		}
		return 1;
	case 6://[1]+  Running                 sleep 3 &
		for(i=0;i<backpid_index;i++){
			if(backpid[i]!=0) break;
		}
		if(i==backpid_index) check_all_stop=1;
		for(i;i<backpid_index;i++){
			if(backpid[i]!=0){
				m=p;
				p=i;
			}
		}
		for(i=0;i<backpid_index;i++){
			//done=waitpid(backpid[i],&status,0);
			if(backpid[i]>0) printf("[%d]+  Running                 %s\n",i+1,cmd[i]);
			else if(backpid[i]<0){ 
printf("[%d]+  Done                    %s\n",i+1,cmd[i]);	
				backpid[i]=0;	
				
			}
		}
		if(check_all_stop==1){
			backpid_index=0;
		}			
		
		return 1;
	case 8:
		if(backpid_index==0){
			printf("bash: bg: current: no such job\n");
			return 1;
		}
		for(i=0;i<backpid_index;i++){
			if(backpid[i]!=0){
				m=p;
				p=i;
			}
		}
		if(p<0) {
			check_all_stop=1;
			printf("bash: bg: job has terminated\n");
		}
		else {
			if(backpid[p]>0){
				printf("bash: bg: job %d already in background\n",(p+1));
			}
			else printf("bash: bg: job has terminated\n");
		}
		
		for(i=0;i<backpid_index;i++){
			//done=waitpid(backpid[i],&status,0);
			if(backpid[i]<0){ 
				if(i==p){
printf("[%d]+  Done                    %s\n",i+1,cmd[i]);}
				else if(i==m){
printf("[%d]-  Done                    %s\n",i+1,cmd[i]);}
				else{
printf("[%d]   Done                    %s\n",i+1,cmd[i]);}
				backpid[i]=0;	
				
			}
		}
		if(check_all_stop==1){
			backpid_index=0;
		}	
		return 1;
	case 7:
		//printf("%d\n",kill(atoi(parsed[1]),SIGABRT));
		kill(atoi(parsed[1]),SIGABRT);
		return 1;
	case 9:
		for(i=(myhist->length - atoi(parsed[1]));i < myhist->length;i++){
			printf("%s %s\n",mylist[i]->line,mylist[i]->timestamp);
			//free(mylist[i]);
		}
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
    int piped = 0,block=0;
    piped = parsePipe(str, strpiped);
    int or=0;
    if (piped) {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
	printf("pipe\n");

    } else {
	int i;
	for(int i=0;i<strlen(str);i++){
		if(str[i]==' '){
			block++;
		}
		if(str[i]=='>' || str[i]=='<'){
			or=1;
			break;
		}
	}
        parseSpace(str, parsed);
    }

    if (ownCmdHandler(parsed,or,block)){
	
        return 0;
    }
    else if(or==1){
	//printf("get or ==1\n");
	output_redirection(parsed);
    }
    else //not my own cmd
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
		input_copy = (char*)malloc(sizeof(char)*strlen(inputString));
		strcpy(input_copy,inputString);
		// process
		execFlag = processString(inputString,
		parsedArgs, parsedArgsPiped);
		// execflag returns zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe.

		// execute
		if (execFlag == 1){
		    
		    execArgs(parsedArgs);
		}
		if (execFlag == 2)//p
		    execArgsPiped(parsedArgs, parsedArgsPiped);
	}
	return 0;
}
