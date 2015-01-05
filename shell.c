/*
Ruby "Schmooby Shmood" Boyd, Eva "the Diva Bineferg" Fineberg, & Conrad "String temp = """ Schloer

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
int num_procs;
int direct_count;
int pipe_count;
int p_count;
pid_t current_process;
char* curTok;
//This takes the newline bit off of a line of input
char* shorten(char* s){
	int len=strlen(s);
	if(s[len-1]=='\n'){
		s[len-1]='\0';
	}
	return s;


}
//paresWhitespace tokenizes our input, conrad will explain more.
char** parseWhitespace(char* s, int* l){ // parses white spaces and the characters <, >, &, and |
	int numToks = 0; // num tokens so far.
	int sizeTok = 0; // size of current token
	char** returnString = (char**) malloc(sizeof(char*)*numToks);
	curTok = (char*) malloc(sizeof(char)*sizeTok); 

	for (int i=0; s[i]!='\0'; i++){ // move through string
		if (s[i] == ' '){ // space
			if (sizeTok != 0){ // allow for multiple spaces
				returnString = realloc(returnString, sizeof(char*)*(numToks+1));
				curTok = realloc(curTok, sizeof(char*)*(sizeTok+1));
				curTok[sizeTok] = '\0';
				returnString[numToks] = curTok;
				numToks++;
				sizeTok = 0;
				curTok = (char*) malloc(sizeof(char)*sizeTok); 
			}
		}
		else if(s[i] == '<' || s[i] == '>' || s[i] == '&' || s[i] == '|'){
			if(s[i] == '|'){
				p_count++;
			}
			if (sizeTok != 0){
				returnString = realloc(returnString, sizeof(char*)*(numToks+1));
				curTok = realloc(curTok, sizeof(char*)*(sizeTok+1));
				curTok[sizeTok] = '\0';
				returnString[numToks] = curTok;
				numToks++;
				sizeTok = 0;
			}
			else free(curTok);

			returnString = realloc(returnString, sizeof(char*)*(numToks+1));
			returnString[numToks] = (char*)malloc(sizeof(char)*2);
			returnString[numToks][0] = s[i];
			returnString[numToks][1] = '\0';
			numToks++;
			sizeTok = 0;
			//free(curTok); // free here possibbly
			curTok = (char*) malloc(sizeof(char)*sizeTok); 
		}
		else {
			curTok = realloc(curTok, sizeof(char)*(sizeTok+1));
			curTok[sizeTok] = s[i];
			sizeTok++;
		}

	}
	if (sizeTok != 0){
		returnString = realloc(returnString, sizeof(char*)*(numToks+1));
		curTok = realloc(curTok, sizeof(char)*(sizeTok+1));
		curTok[sizeTok] = '\0';
		returnString[numToks] = curTok;
		numToks++;
	} else free(curTok);
	returnString = realloc(returnString, sizeof(char*)*(numToks+1));
	returnString[numToks] = NULL;
	*l = numToks;
	//checks to make sure we never run commands with incorrect piping format (errors 3 and 4)
	for(int k=0; k<numToks; k++){
		if(strcmp(returnString[k],"<")==0 || strcmp(returnString[k], ">")==0){
			direct_count++;
			if(pipe_count >=1 && pipe_count!=p_count){
				perror("The redirecting must take place at the beginning or end of piping");
				free(returnString[0]);
				returnString[0] =(char*) "eva";
				return returnString;
				//exit(3);
			}
		}
		if(strcmp(returnString[k],"|")==0){
			pipe_count++;
		}
		
	}
	return returnString;


} 

//redirect(char**s) takes in our line of commands, steps through it and 
//runs and directs the commands and files according to the input
//after encountering a ">" or "<" we set that part to null as well as
//the word after it so execvp can work properly.
//This works for multiple redirection.
void redirect(char** s){
	int i=0;
	int flag=0;
	// for (int i=0; i<numToks; i++){
	while(s[i]!=NULL){	
		if (!strcmp(s[i], ">")){ // change stdout to the next token
			errno = 0;
			s[i]=NULL;
			i++;
			FILE* out = fopen(s[i], "w+");
			if (errno != 0) perror("Could not open file for writing, please try again");
			s[i] = NULL;
			i++;
			dup2(fileno(out), STDOUT_FILENO);
			flag=1;
			break;
		}
		if (!strcmp(s[i], "<")){ // change parameters for previous token to the value of next token
			errno = 0;
			s[i]= NULL;
			i++;
			FILE* out = fopen(s[i], "r");
			if (errno != 0) perror("Could not open file for reading, please try again");
			s[i] = NULL;
			i++;
			dup2(fileno(out), STDIN_FILENO);
			flag=1;
			break;
		}
		i++;
	}
	if(flag){
		while(s[i]!=NULL){
			if (!strcmp(s[i], ">")){ // change stdout to the next token
				errno = 0;
				s[i]=NULL;
				i++;
				FILE* out = fopen(s[i], "w+");
				if (errno != 0) perror("Could not open file for writing, please try again");
				s[i] = NULL;
				i++;
				dup2(fileno(out), STDOUT_FILENO);
				flag=1;
				break;
			}
			if (!strcmp(s[i], "<")){ // change parameters for previous token to the value of next token
				errno = 0;
				s[i]=NULL;
				i++;
				FILE* out = fopen(s[i], "r");
				if (errno != 0) perror("Could not open file for reading, please try again");
				s[i] = NULL;
				i++;
				dup2(fileno(out), STDIN_FILENO);
				flag=1;
				break;
			}
			i++;

		}
	}
}
//Traps SIGINT and terminates the currently running child process of the shell.
//It does not terminate the shell itself.
//It also handles (ctrl-z), which when pressed changes the child program that is currently running
//into a background process.
//Both are registered at the beginning of main.
void signal_callback_handler(int signum){ // catch the signal
	//printf("Caught signal %d\n", signum);

	if(signum==SIGINT &&(current_process>0)){
		signal(signum, SIG_IGN);
		kill(current_process, SIGINT);
	}
	if((current_process>0) && signum==SIGTSTP){
	//	printf("i'm in this swag swag swag SWERVE");	
		setpgid(0,0);
	}

	signal(SIGINT, signal_callback_handler); // register the signal catchers `
	signal(SIGTSTP, signal_callback_handler);
}

//This checks to see if the token & is at the end of a commandline
//If it is we return 1, otherwise return 0.
int background_check(char** s, int numToks){
	if(!strcmp(s[numToks-1], "&")){
		return 1;
	}
	else{
		return 0;
	}

}
//This is called only if background_check returns true.
//This runs the process in the background and frees up space.
void background_run(char **s, int numToks){ // run in background because of & at end
	free(s[numToks-1]);
	s[numToks-1] = NULL;
	pid_t c = fork();
	if(!c){
		setpgid(0,0);
		redirect(s);
		execvp(s[0], s);
		perror("That is not a proper command, please try again");
		for (int i=0; i<numToks-1; i++){
			free(s[i]);
		}
		free(s);
		exit(3);

	}
}
//This function divides up the command tokens into an array of arrays of processes/files
//so we can use this char*** to handle piping.
char*** dividePipes(char** s, int numToks2, int* l){
	free(s[numToks2]);
	int numGroups = 1;
	int numToks = 0;
	char*** returnArray = (char***)malloc(sizeof(char**)*(numGroups));
	returnArray[numGroups-1] = malloc(sizeof(char*)*numToks);
	for (int i=0; i<numToks2; i++){
		if ( !strcmp(s[i], "|")){
			returnArray[numGroups-1] = realloc(returnArray[numGroups-1], sizeof(char*)*(numToks+1));
			returnArray[numGroups-1][numToks] = NULL ;
			
			returnArray = realloc(returnArray, sizeof(char**)*(numGroups+1));
			numToks = 0;
			numGroups++;
			returnArray[numGroups-1] = malloc(sizeof(char*)*numToks);
		}
		else {
			returnArray[numGroups-1] = realloc(returnArray[numGroups-1], sizeof(char*)*(numToks+1));
			returnArray[numGroups-1][numToks] = s[i] ;
			numToks++;
		}
	}	
		
	returnArray[numGroups-1] = realloc(returnArray[numGroups-1], sizeof(char*)*(numToks+1));
	returnArray[numGroups-1][numToks] = NULL ;
	*l = numGroups;
	return returnArray;

}

//here we use the char*** returned by divide to 
//handle multipiping sequences
void multiPipes(char*** s2, int numGroups, int top){
	if (numGroups == 0 ){ // last pipe, 
		redirect(s2[numGroups]);
		execvp(s2[numGroups][0], s2[numGroups]);	
		perror("Not correct command, try again");
		
		exit(EXIT_SUCCESS);
		return;
	}
	int descripts[2];
	int stat;
	pipe(descripts);
	int pid2 = fork();
	if(pid2>0){ // now it's the parent
		close(descripts[1]); // close the write
		dup2(descripts[0], STDIN_FILENO); // change input to the file descriptor
		redirect(s2[numGroups]);
		execvp(s2[numGroups][0], s2[numGroups]);
		perror("Not correct command, try again");
	}
	else{ // the child, which doesn't actually exec anything here. Only execs after recurisvely calling this functions
		close(descripts[0]); // close the read
		dup2(descripts[1],STDOUT_FILENO); // change output to the file descriptor
		multiPipes(s2, numGroups-1, 0);
	}
	waitpid(pid2,&stat,0);
	if (!top){
		exit(EXIT_SUCCESS);
	} // only exit if not the top, so we can free in the file

}

//Here we run in the forground, this is where we handle piping
//We also make calls to redirect() if necessary.
int foreground_run(char** s, int numToks){ // run in foreground because no & at end
	int numGroups = 0;
	pid_t pid;
	char***  s2= dividePipes(s, numToks, &numGroups);
	if (numGroups > 1) {
		pid = fork();
		if (pid == 0) { // it's a child of the main shell
			multiPipes(s2, numGroups-1, 1); 
			for (int i=0; i<numToks; i++){
				printf("%s ", s[i]);
				free(s[i]);
			}
			free(s);
			exit(EXIT_SUCCESS);
		}

	}

	else {
		pid = fork();
		if (pid == 0){
			redirect(s);
			execvp(s[0], s);
			perror("Not correct command, try again");
			exit(1);
		}
	}

	for(int i=0; i<numGroups; i++){
		free(s2[i]);
	}
	free(s2);
    return pid;
}


//This checks whether it should be run in the forground or the background
//and calls the appropriate function.
void runCommand(char** s, int numToks){
    int status;
    if(background_check(s, numToks)==1){
		background_run(s, numToks);
	}

	else {
	    pid_t p=foreground_run(s, numToks);
	    waitpid(p, &status, 0);
	}
}

int main(){
	char* input;
	int status;
	num_procs=0;
	current_process=0;
	char* user = getenv("USER");
	signal(SIGINT, signal_callback_handler); // register the signal catcher 
	signal(SIGTSTP, signal_callback_handler);//tried the extra credit thing, not sure tho
	while(1){
		direct_count=0;
		pipe_count=0;
		p_count=0;
		pid_t done;
		while((done=waitpid(-1, &status, WNOHANG))>0){
			if(WIFEXITED(status)){
				printf("\n\n%s\t%d\n\n", "Done",done); 
			}
		}
		num_procs=0;
		printf("%s@%s",user,"<ECRash>: ");
		input = (char*) malloc(sizeof(char)*1024);
		if( fgets(input, 1024, stdin) == NULL ){
			free(input);
			exit(1);
		}
		if(input[0]=='\n'){
		    free(input);
		    continue;
		}input=shorten(input);
		int numToks = 0;
		char** s = parseWhitespace(input, &numToks);
		free(input);
		if(strcmp(s[0], "eva") ==0){
			for(int j=1; j<numToks; j++){
				free(s[j]);
			}
			free(s);
			continue;
		}
		if(strcmp(s[0], "exit")==0){ // check first word for exit    
			for (int i=0; i<numToks; i++){
				free(s[i]);
			}
			free(s);
			exit(1);
		}
		runCommand(s, numToks);	
		for (int i=0; i<numToks; i++){
			free(s[i]);
		}
		free(s);
	}

	return 0;
}
< efineber@occs >
