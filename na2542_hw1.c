#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<dirent.h>

/* Boolean constants */
#define TRUE 1
#define FALSE 0

extern int errno;
/* Declaring functions*/
int scan_directory(char *dir, char *target);
/* Path variable to be used to maintain path of the executables required to run the commands*/
char *path="";

/*Function to concatenate two strings*/
char* concat(char *s1, char *s2){
	printf("s1 %s : s2 %s\n", s1, s2);
	if(s1 == NULL || s2  == NULL || (strcmp(s2, "")==0)){
		printf("Invalid Strings");
		return s1;
	}
	char *temp_malloc = (char *)malloc(strlen(s1) + strlen(s2) + 1), *result;	

	if(temp_malloc == NULL){
		printf("Memory Overflow");
		return s1;
	}
	result = temp_malloc;
	
	strcpy(result, s1);
	if(strcmp(s1, "") != 0){
		free(s1);
	}	
	strcat(result, s2);
	if(result == NULL){
		printf("error : %s\n", strerror(errno));
		return s1;
	}
	return result;

	
}

/* Getting input from the user*/
char* get_input(){
	int mem = 128;
	char *temp_malloc = (char*) malloc(mem);
	char *str, next, *temp_realloc;
	if(temp_malloc == NULL){
		printf("Memory Overflow");
		return NULL;
	}else{
		
		str = temp_malloc;
	}
	fgets(str, mem, stdin);	
	
	while(str[strlen(str) - 1] != '\n'){
		mem *= 2;
		temp_realloc = realloc(str, mem);
		if(temp_realloc == NULL){
			printf("Memory Overflow");
			while(next!= '\n' && next !=EOF){
				next = getchar();
			}
			
			return NULL;
		}else{			
			str = temp_realloc;
			fgets(str+mem/2 - 1,mem/2 + 1, stdin); 
		}
	}
	str[strlen(str)-1] = '\0';
	return str;
}


/*Find path for executable in the path variable*/
char* handle_commands(char* str, char *cmd){
	printf("find_strings : %s, %s \n", str, cmd);
	if(str == NULL || strcmp(str, "") == 0){
		return NULL;		
	}
	char *token;
	token = strtok(str, ":");
	while(token != NULL){
		printf("Token: %s\n", token);
		if(scan_directory(token, cmd) == TRUE){
			return token;
		}		
		token = strtok(NULL, ":");
	}	

}

int scan_directory(char *dir, char *target){
//	printf("Target: %sc", target);
	if(dir == NULL || strcmp(dir, "") == 0){
		return FALSE;
	}
	DIR *d;
	int n;
	struct dirent **entry;
	n = scandir(dir, &entry, 0, alphasort);
	if(n < 0){
		printf("error : %s\n", strerror(errno));
		return FALSE;
	}else{
		while(n--){
			
			//printf("File name: %s\n", entry[n]->d_name); 
			if(strcmp(entry[n]->d_name, target) == 0){
				printf("File found: %s\n", entry[n]->d_name);
				
			}
			free(entry[n]);
		//return;		
		}
		free(entry);
		return TRUE;		
	}
}

int main(){

	char *str, *token, *cmd[100], *b, *cmd_path;
	pid_t pid;
	
	int i = 0;
	while(1){
		i = 0;
		printf("$");
		str = (char*)get_input();
		if(str == NULL){
			printf("error : %s\n", strerror(errno));
		}
		token = strtok(str, " \t");
		while(token != NULL){
			cmd[i++] = token;
			token = strtok(NULL, " \t");
		}
		if(strcmp(cmd[0], "exit") == 0){
			printf("EXITING shell\n");
			break;	
		}
		else if(strcmp(cmd[0], "cd") == 0){
			printf("==>");
			if(chdir(cmd[1]) < 0){
				printf("error : %s\n", strerror(errno));
			}
		}		
		else if(i >= 1 && strcmp(cmd[0], "path")==0){
			if(i == 1){
				printf("path: %s\n", path);
			}else if((strcmp(cmd[1], "+") == 0) || (strcmp(cmd[1],"-") == 0)){
				if(i >= 3){ 
				//	printf("error: Missing arguments");
					if(strcmp(cmd[1], "+") == 0){
						printf("path argument: %s\n", cmd[2]);
						if(strlen(path) > 0){
  							path = concat(path, ":");
						}
						path = concat(path, cmd[2]);
					}
					if(strcmp(cmd[1], "-") == 0){
						printf("Path subtraction\n");
						if(strcmp(path, "") != 0){
							token = strtok(path , ":");			
							while(token != NULL){
								if(strcmp(token, cmd[2]) > 0){
									b = concat(b, token);
								}
								token = strtok(NULL, ":");
							}
							path = b;
						}						
					}
				
				
			}
		}else{
			if(i >= 1){
				printf("Command: %s\n", cmd[0]);
				cmd_path = handle_commands(path, cmd[0]);
				/*Execute command*/
				pid = fork();
				if(pid < 0){
					break;
				}
				if(pid > 0){
					printf("Parent waiting for child \n");
					wait(pid);
				}
				if(pid == 0){
					cmd_path = concat(cmd_path, cmd[0]);
					cmd[i] = NULL;
					execv(cmd_path, cmd);
					printf("error : %s\n", strerror(errno));
					exit(0);
				}
			}
		}
	
				

		free(str);
		str = NULL;	

	}
}


