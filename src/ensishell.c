/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include  <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "variante.h"
#include "readcmd.h"
#include <sys/time.h>
#include <signal.h>

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>



struct bgprocess{
	pid_t pid; // pid of the process
	int time;
	char ** cmd; //command executed
};

#define BACKGROUND_PROCESS_MAX 20 // Max number of background processes
struct bgprocess* listbgprocess[BACKGROUND_PROCESS_MAX] = {NULL}; // array containing processes running in background
int latest_index =0; // available index in listbgprocess



void display_cmd(char ** cmd){
    for (int j=0; cmd[j]!=0; j++) {
		printf("'%s' ", cmd[j]);
	                }
	printf("\n");
		
		
}

void addbgprocess(pid_t pid, char** cmd){
	if (latest_index==BACKGROUND_PROCESS_MAX){
		printf("All background processes are used\n");
		exit(1);
	}
	listbgprocess[latest_index] = malloc(sizeof(struct bgprocess));
	assert(listbgprocess[latest_index] != NULL);
	
	listbgprocess[latest_index]->pid = pid;

	listbgprocess[latest_index]->cmd = malloc(sizeof(char*));
	assert(listbgprocess[latest_index]->cmd != NULL);
	*(listbgprocess[latest_index]->cmd) = malloc(strlen(*cmd)+1);
	assert(*(listbgprocess[latest_index]->cmd) != NULL);
	strcpy(*(listbgprocess[latest_index]->cmd),*cmd);

	struct timeval time;
	gettimeofday(&time,NULL);
	listbgprocess[latest_index]->time = time.tv_sec;

	latest_index++;
}

void jobs(){
	for (int i=0; i<latest_index; i++){
		printf("[%d] ",listbgprocess[i]->pid);
		display_cmd(listbgprocess[i]->cmd);
	}
}


void execute_cmd(char **cmd,int bg, char* in , char* out){
	
	if (strcmp(cmd[0],"jobs")==0){
	
			jobs();
	
	} else {
		// Creating a new process
		pid_t pid = fork();
		// Checking if we are in the child process and executing if it's the case
		if (pid==0){

			if (in){
			int fd = open(in , O_RDONLY);
			if (fd == -1) { perror("open: " ); exit(EXIT_FAILURE);}
			dup2(fd, 0);
			close(fd);
			}

			if (out){
			int fd = open(out, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (fd == -1) { perror("open: " ); exit(EXIT_FAILURE);}
			ftruncate(fd,0);
			dup2(fd, 1);
			close(fd);
			}

			int i = execvp(cmd[0],cmd);
			// We exit if the process fails
			if(i==-1){
				printf("Process failed\n");
				exit(1);
			}
			
		} else {
			if(!bg){
				// Make the parent process wait until the child process is finished if it's not run in background
				waitpid(pid, NULL,0);
			} else {
				addbgprocess(pid,cmd);
			}
		}
	}
}

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */

	struct cmdline* l = parsecmd(&line);
	if (l) {
		char **cmd = l->seq[0];

		if (cmd){
			execute_cmd(cmd,l->bg,l->in,l->out);
		}
	}
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}

void get_execution_time(){
 	// Getting the pid of the process who sent the signal
 	pid_t trigerring_process_pid = waitpid(-1, NULL, WNOHANG);
 	struct timeval current_time;
 	int elapsed_time;
	for (int i =0 ; i<latest_index ; i++){
		if (trigerring_process_pid==listbgprocess[i]->pid){
 			gettimeofday(&current_time,NULL);
 			// Computing elapsed time 
 			elapsed_time = current_time.tv_sec-listbgprocess[i]->time;
 			printf("\nLe processus (%d) a pris %d secondes\nensishell>",listbgprocess[i]->pid,elapsed_time);
			
			// Removing finished background process
			struct bgprocess* finished = listbgprocess[i];
			for (int j=i+1; j<BACKGROUND_PROCESS_MAX;j++){
				if(listbgprocess[j]){
					listbgprocess[j-1]=listbgprocess[j];
				} else {
					break;
				}
			}
			latest_index--;
			free(*(finished->cmd));
			free(finished->cmd);
			free(finished);
			break;
			}
		
	}

}



/*
   Function to handle piping and redirection in command execution.
   Executes commands in a pipeline and handles input/output redirection
*/
void piping_redirection(struct cmdline* l,char* in, char* out, int nbr_pipes){

	int total_piping = 2*nbr_pipes;
	int pipes[total_piping];

	for (int i=0 ; i<total_piping; i+=2){
		pipe(pipes+i);
	}
	pid_t pid = fork(); // Forking the first child process
	pid_t pid1 = pid;
	
	int j = 0;
	while (j<=nbr_pipes){
		
		if (j > 0){
			pid1 = fork(); // Forking additional child processes for commands in the pipeline
		}
		if (pid1 ==0){
		
			if (in){
				int fd = open(in , O_RDONLY);
				if (fd == -1) { perror("open: " ); exit(EXIT_FAILURE);}
				dup2(fd, 0);
				close(fd);
			}

			if (out){
				int fd = open(out, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (fd == -1) { perror("open: " ); exit(EXIT_FAILURE);}
				ftruncate(fd,0);
				dup2(fd, 1);
				close(fd);
			}
			if(j!=0){

			dup2(pipes[2*(j-1)],0);
			}
			if(j!=nbr_pipes){

			dup2(pipes[2*j+1],1);
			}
			// Closing pipe descriptors
			for(int i = 0; i < total_piping; i++){
					close(pipes[i]);
			}

			execvp(l->seq[j][0],l->seq[j]);

		}
		
		j++;
	} 
	// Close all pipe ends in parent
    for (int i = 0; i < total_piping; i++){
        close(pipes[i]);
	}

	// Waiting for all child processes to finish if not running in background
	if (!l->bg) {
	    for (int i = 0; i <= nbr_pipes; i++) {
        	wait(NULL);
    	}
	}	

}

void count_pipe(struct cmdline* l, int* count){
	for (int i=0; l->seq[i+1]!=0; i++) {
			(*count)++;
		}
}


int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
		setbuf(stdout, NULL);
		// Managing signals
 		struct sigaction current_signal;
 		current_signal.sa_flags = SA_RESTART;
 		current_signal.sa_sigaction = get_execution_time;
 		sigaction(SIGCHLD, &current_signal, NULL);


#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line=0;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {

			terminate(0);
		}
		

		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		int count=0;
		count_pipe(l,&count);

		if (count >= 1){
			piping_redirection(l,l->in,l->out,count);
		
		} else {
			if(l->seq[0]){
				
				execute_cmd(l->seq[0],l->bg,l->in,l->out);	
			}


			// /* Display each command of the pipe */
			// int i, j;
			// for (i=0; l->seq[i]!=0; i++) {
			// 	char **cmd = l->seq[i];
			// 	printf("seq[%d]: ", i);
			// 				for (j=0; cmd[j]!=0; j++) {
			// 						printf("'%s' ", cmd[j]);
			// 				}
			// 	printf("\n");
			// }
		}
	}
}
