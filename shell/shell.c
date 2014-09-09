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
	printf("s1 -%s : s2 -%s\n", s1, s2);
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
			while (next != '\n' && next != EOF) {
				next = getchar();
			}

			return NULL;
		} else {
			str = temp_realloc;
			fgets(str + mem / 2 - 1, mem / 2 + 1, stdin);
		}
	}
	str[strlen(str) - 1] = '\0';
	return str;
}

/*Find path for executable in the path variable*/
char* handle_commands(char *cmd) {
	if (path == NULL || strcmp(path, "") == 0) {
		return NULL;
	}
	char *token, *str, *token_found;
	int flag = FALSE;
	char *temp_malloc = (char *) malloc(strlen(path) + 1);
	if (temp_malloc == NULL) {
		printf("Memory Overflow");
		return NULL;
	}
	str = temp_malloc;
	strcpy(str, path);
	token = strtok(str, ":");
	while (token != NULL) {
		if (scan_directory(token, cmd) == TRUE) {

			char *temp = (char *) malloc(strlen(token) + 1);
			if (temp == NULL) {
				printf("Memory Overflow");
				return NULL;
			}
			token_found = temp;
			strcpy(token_found, token);
			printf("token: %s\n", token);
			printf("token_found: %s\n", token_found);
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

int scan_directory(char *dir, char *target) {
	int flag = FALSE;
	if (dir == NULL || strcmp(dir, "") == 0 || target == NULL || strcmp(target, "") == 0) {
		return FALSE;
	}
	int n;
	struct dirent **entry;
	n = scandir(dir, &entry, 0, alphasort);
	if (n < 0) {
		return FALSE;
	} else {
		while (n--) {
			if (strcmp(entry[n]-> d_name, target) == 0) {
				flag = TRUE;
			}
			free(entry[n]);
		}
		free(entry);
	}
	return flag;
}

int main() {

	char *str, *token, *cmd[100], *cmd_path="", *path_token;
	pid_t pid;

	int i = 0;
	while (1) {
		i = 0;
		char *b = "";
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
		if (strcmp(cmd[0], "exit") == 0) {
			printf("EXITING shell\n");
			exit(0);
		} else if (strcmp(cmd[0], "cd") == 0) {
			printf("==>");
			if (chdir(cmd[1]) < 0) {
				printf("error : %s\n", strerror(errno));
			}
		} else if (i >= 1 && strcmp(cmd[0], "path") == 0) {
			if (i == 1) {
				printf("path: %s\n", path);
			} else if ((strcmp(cmd[1], "+") == 0)
					|| (strcmp(cmd[1], "-") == 0)) {
				if (i >= 3) {
					//	printf("error: Missing arguments");
					if (strcmp(cmd[1], "+") == 0) {
						printf("path argument: %s\n", cmd[2]);
						if (strlen(path) > 0) {
							path = concat(path, ":");
						}
						path = concat(path, cmd[2]);
					}
					if (strcmp(cmd[1], "-") == 0) {
						printf("Path subtraction\n");
						if (strcmp(path, "") != 0) {
							printf("in subtract \n");
							path_token = strtok(path, ":");

						printf("token1: %s\n", path_token);
							while (path_token) {
								if (strcmp(path_token, cmd[2]) != 0) {
									if (strlen(b) > 0) {
										b = concat(b, ":");
									}
						printf("token2: %s\n", path_token);
									b = concat(b, path_token);
								}
								path_token = strtok(NULL, ":");
							}
							printf("b---------> %s\n", b);
							if(strcmp(path, "") != 0){
								free(path);
								path = NULL;
							}
//							printf("length b: %d", strlen(b));
							path = b;
						}
					}

				}
			}
		}else {
				if (i >= 1) {
					cmd_path = handle_commands(cmd[0]);
					pid = fork();
					if (pid < 0) {
						break;
					}
					if(cmd_path == NULL){
						printf("error : Command not found\n");
					}					
					if (pid > 0) {
						int child_status;
						wait(&child_status);
					}
					if (pid == 0 && cmd_path != NULL) {
						cmd_path = concat(cmd_path, "/");
						cmd_path = concat(cmd_path, cmd[0]);
						cmd[i] = NULL;
						printf("cmd_path: %s\n", cmd_path);
						execv(cmd_path, cmd);
						printf("error : %s\n", strerror(errno));
						exit(0);
					}
				}
		}

			free(str);
			str = NULL;		

		
	}
	return(0);
}

