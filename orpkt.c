#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<readline/readline.h>
char *prompt = "% ";
void or(char** args){
	char *cp;
    	char *ifile;
    	char *ofile;
	char **args2;
    	int i;
    	int j;
    	int err;
	int pid;
	args2 = malloc(80 * sizeof(char *));
	// assume no redirections
        ofile = NULL;
        ifile = NULL;

        // split off the redirections
        j = 0;
        i = 0;
        err = 0;
        while (1) {
	    printf("i=%d\n",i);
            cp = args[i++];
	    printf("cp %s\n",cp);
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
		printf("is > !!\n");
		printf("cp[1] is %c\n",cp[1]);
                if (cp[1] == 0){//only one char(only >)
		    printf("if\n");
                    cp = args[i++];}
                else{//between > and file no block
		    printf("else\n");
                    ++cp;}
                ofile = cp;
		printf("ofile %s\n",ofile);
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
                if ((fd2 = open(ofile, O_WRONLY | O_CREAT, 0644)) < 0) {
                    perror("couldn't open output file.");
                    exit(0);
                }
		printf("ofile3 %s %ld\n",ofile,strlen(ofile));
		printf("kkk\n");
		printf("okay\n");
                // args+=2;
                printf("okay");
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

int main()
{
    
    //int child_pid;
    //char line[81];
    char *line;
    char *token;
    char *separator = " ";
    char **args;
    int i;
    
    //int check;
    //char history[90];
    //typedef void (*sighandler_t) (int);

    args = malloc(80 * sizeof(char *));
    

    //signal(SIGINT, SIG_IGN);

    while (1) {
        fprintf(stderr, "%s", prompt);
        fflush(stderr);
	line=readline(NULL);
	if(line==NULL) break;
        /*if (fgets(line, 80, stdin) == NULL)
            break;*/

        // split up the line
        i = 0;
        while (1) {
            token = strtok((i == 0) ? line : NULL, separator);
            if (token == NULL)
                break;
	    printf("%s\n",token);
            args[i++] = token;              /* build command array */
        }
        args[i] = NULL;
        if (i != 0){
		or(args);
	}
	
        
    }

    exit(0);
}
