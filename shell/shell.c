#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<dirent.h>
#include<unistd.h>
/* Boolean constants */
#define TRUE 1
#define FALSE 0

extern int errno;

/* Declaring functions*/
int scan_directory(char *dir, char *target);
int chdir(const char *path);
pid_t wait(int *status);
pid_t fork(void);
int execv(const char *path, char *const argv[]);

/* Path variable to be used to maintain path of the executables required to run the commands*/
char *path = "";

/*Function to concatenate two strings*/
char* concat(char *s1, char *s2) {
	if (s1 == NULL || s2 == NULL || (strcmp(s2, "") == 0)) {
		printf("Invalid Strings");
		return s1;
	}
	char *temp_malloc = (char *) malloc(strlen(s1) + strlen(s2) + 1), *result;

	if (temp_malloc == NULL) {
		printf("Memory Overflow");
		return s1;
	}
	result = temp_malloc;

	strcpy(result, s1);
	if (strcmp(s1, "") != 0) {
		free(s1);
		s1 = NULL;
	}
	strcat(result, s2);
	if (result == NULL) {
		printf("error : %s\n", strerror(errno));
		return s1;
	}
	return result;

}

/* Getting input from the user*/
char* get_input() {
	int mem = 128;
	char *temp_malloc = (char*) malloc(mem);
	char *str, next, *temp_realloc;
	if (temp_malloc == NULL) {
		printf("Memory Overflow");
		return NULL;
	} else {
		str = temp_malloc;
	}
	fgets(str, mem, stdin);

	while (str[strlen(str) - 1] != '\n') {
		mem *= 2;
		temp_realloc = realloc(str, mem);
		if (temp_realloc == NULL) {
			printf("Memory Overflow");
			while ((next = getchar()) != '\n' && next != EOF);
			return NULL;
		} else {
			str = temp_realloc;
			fgets(str + mem / 2 - 1, mem / 2 + 1, stdin);
		}
	}
	str[strlen(str) - 1] = '\0';
	return str;
}

/*Returns location of the executable for the given command*/
char* handle_commands(char *cmd) {
	if (path == NULL || strcmp(path, "") == 0) {
		return NULL;
	}
	char *token, *str, *token_found;
	int flag = FALSE;
	char *temp_malloc = (char *) malloc(strlen(path) + 1);
	if (temp_malloc == NULL) {return NULL;}
	str = temp_malloc;
	strcpy(str, path);
	token = strtok(str, ":");
	while (token != NULL) {
		if (scan_directory(token, cmd) == TRUE) {
			char *temp = (char *) malloc(strlen(token) + 1);
			if (temp == NULL) {return NULL;}
			token_found = temp;
			strcpy(token_found, token);
			flag = TRUE;
		}
		token = strtok(NULL, ":");
	}
	if(strcmp(str, "") != 0){
		free(str);
		str = NULL;
	}
	if(flag == TRUE){
		return token_found;
	}else{
		return NULL;
	}
	
}

/* Locate if given target if present in the given directory and return TRUE or FALSE */
int scan_directory(char *dir, char *target) {
	int flag = FALSE;
	if (dir == NULL || strcmp(dir, "") == 0 || target == NULL || strcmp(target, "") == 0) {
		return FALSE;
	}
	int n;
	struct dirent **entry;
	n = scandir(dir, &entry, 0, alphasort);
	if (n < 0) {return FALSE;
	}else {
		while (n--) {
			if (strcmp(entry[n]-> d_name, target) == 0) flag = TRUE;
			free(entry[n]);
		}
		free(entry);
	}
	return flag;
}


/* Handling path command*/
void handle_path(int cmd_length, char *input[]){
	char *path_token="";
	if (cmd_length == 1) {
		printf("%s\n", path);
	} else if (cmd_length >= 3 && (strcmp(input[1], "+") == 0)) {
		if (strlen(path) > 0) path = concat(path, ":");
		path = concat(path, input[2]);
	}else if (cmd_length >= 3 && strcmp(input[1], "-") == 0 && strcmp(path, "") != 0) {
		char *b = "";
		path_token = strtok(path, ":");
		while (path_token) {
			if (strcmp(path_token, input[2]) != 0) {
				if (strlen(b) > 0) b = concat(b, ":");
				b = concat(b, path_token);
			}
			path_token = strtok(NULL, ":");
		}
		if(strcmp(path, "") != 0){
			free(path);
			path = NULL;
		}
		path = b;
	}

}

/*Executes commands using fork, wait and execv*/
void execute_commands(pid_t pid, char *input[], char *cmd_path, int len){
	if (pid < 0) {
		return;
	}
	if(cmd_path == NULL){
		printf("error : Command not found\n");
		return;
	}
	if (pid > 0) {
		int child_status;
		wait(&child_status);
	}
	if (pid == 0 && cmd_path != NULL) {
		cmd_path = concat(cmd_path, "/");
		cmd_path = concat(cmd_path, input[0]);
		input[len] = NULL;
		execv(cmd_path, input);
		printf("error : %s\n", strerror(errno));
		exit(0);
	}
}

int main() {

	char *str, *token, *cmd[100], *cmd_path="";
	pid_t pid;

	int i = 0;
	while (1) {
		i = 0;
		printf("$");
		str = (char*) get_input();
		if (str == NULL) {
			printf("error : %s\n", strerror(errno));
		}
		token = strtok(str, " \t");
		while (token != NULL) {
			cmd[i++] = token;
			token = strtok(NULL, " \t");
		}
		if(i<=0){
			continue;
		} else if (strcmp(cmd[0], "exit") == 0) {
			printf("EXITING shell\n");
			free(str);
			str = NULL;
			exit(0);
		} else if (strcmp(cmd[0], "cd") == 0) {
			printf("==>");
			if (chdir(cmd[1]) < 0) {printf("error : %s\n", strerror(errno));}
		} else if (i >= 1 && strcmp(cmd[0], "path") == 0) {
			handle_path(i, cmd);
		}else {
			cmd_path = handle_commands(cmd[0]);
			pid = fork();
			execute_commands(pid, cmd, cmd_path, i);
			/*if (pid < 0) {break;}
			if(cmd_path == NULL){printf("error : Command not found\n");}
			if (pid > 0) {
				int child_status;
				wait(&child_status);
			}
			if (pid == 0 && cmd_path != NULL) {
				cmd_path = concat(cmd_path, "/");
				cmd_path = concat(cmd_path, cmd[0]);
				cmd[i] = NULL;
				execv(cmd_path, cmd);
				printf("error : %s\n", strerror(errno));
				exit(0);
			}*/

		}
		free(str);
		str = NULL;
	}
	return(0);
}

