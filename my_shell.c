#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

volatile sig_atomic_t sigint_flag = 0;

static void sigint_handler(int signum) {
    //printf("Caught SIGINT signal (Ctrl+C)\n");
    // Send SIGINT signal to the process that sent this signal
	if (sigint_flag == 0) {
		    sigint_flag = 1;
            //printf("set signal to 1\n");
            // Set flag to indicate signal caught from external source
            pid_t sender_pgid = getpgid(getpid()); // Get the PID of the current process
    		killpg(sender_pgid,SIGINT);    // Send SIGINT signal to the sender process
    } else {
            // Reset flag if signal caught internally
			//printf("Set signal to 0\n");
            sigint_flag = 0;
    }
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
    
	struct sigaction sa;
    sa.sa_handler = &sigint_handler; // Set the handler function
    sigemptyset(&sa.sa_mask);       // Clear the signal mask
    sa.sa_flags = 0;                // No special flags

    // Register the signal handler for SIGINT
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();
		pid_t pid;

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
         
		int isEmpty = 1;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] != '\n' && line[i] != '\r' && line[i] != ' ') {
                isEmpty = 0;
                break;
            }
        }

        // If the line is empty, continue to prompt for input
        if (isEmpty) {
            printf("Empty line. Please enter some text.\n");
            continue;
        }
        
		
       
	   if(strcmp(tokens[0]," ") == 0){
		continue;
	   }


	   //Reap processes
        int status;
		// pid = waitpid(-1, &status, WNOHANG);
		// printf(pid);
		// fflush(stdout);
        pid = waitpid(-1, &status, WNOHANG);
		if (pid != -1 && pid != 0 )
			printf("Shell: Background process finished\n");
		

       //Checking if the process is foreground or bg
	   int i = 0;
			while (tokens[i] != NULL){
               i++;
			};
		bool bg = false;
		if(strcmp(tokens[i-1],"&") == 0){
           tokens[i-1] = NULL;
		   bg = true;
		}

		   //exit using kil()
		if(strcmp(tokens[0],"exit") == 0){
		kill(-1,SIGTERM);//pid = -1 to kill all processes
		for(i=0;tokens[i]!=NULL;i++){
				free(tokens[i]);
			}
			free(tokens);
		break;
		}

        
		if(strcmp(tokens[0],"cd") == 0){
		   const char *s2 = tokens[1];
		   chdir(tokens[1]);
		   char cwd[1024];
		   getcwd(cwd, sizeof(cwd));
		   printf("Current Directory:%s \n",cwd);
           continue;
		}
        
		

	    int child = fork();
		
		if(child < 0){
			fprintf(stderr, "fork failed\n");
			exit(1);
		}else if(child == 0){
			if(strcmp(tokens[0], "ls") == 0 || strcmp(tokens[0], "cat") == 0 || strcmp(tokens[0], "echo") == 0 || strcmp(tokens[0], "sleep") == 0
			|| strcmp(tokens[0], "cd") == 0){

			   if(!bg){
				setpgid(0,0);
			   }

               execvp(tokens[0], tokens);
			}else{
               perror("No such command");
			}
		}else{
			int p;
			int i = 0;
			while (tokens[i] != NULL){
               i++;
			};

			//printf("%s\n",tokens[i-1]);
			int status2;
			if(!bg){
				waitpid(child,&status2,0);
			}else{
				continue;
			}
			
		}
        
		
		//printf("found token %s (remove this debug output later)\n", tokens[i]);
    
		// Freeing the allocated memory	
		
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
        

	}
	return 0;
}
