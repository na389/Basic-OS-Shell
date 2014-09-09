#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

extern int errno;
/* Path variable to be used to maintain path of the executables required to run the commands*/
char *path="";

/*Function to concatenate two strings*/
char* concat(char *s1, char *s2){
	printf("s1 %s : s2 %s\n", s1, s2);
	if(s1 == NULL || s2  == NULL){
		printf("Invalid Strings");
		return s1;
	}
	char *temp_malloc = (char *)malloc(strlen(s1) + strlen(s2) + 1), *result;	

	if(temp_malloc == NULL){
		printf("Memory Overflow");
		return s1;
	}
	result = temp_malloc;
//	free(temp_malloc);
	
	strcpy(result, s1);
	strcat(result, s2);
	if(result == NULL){
		printf("error : %s\n", strerror(errno));
		return s1;
	}
//	printf("concat-> result: %s\n", result);
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


/*Handle commands*/
void handleCommands(char* cmd[]){
	
}

int main(){

	char *str, *token, *cmd[100];
	
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
		if(strcmp(cmd[0], "cd") == 0){
			printf("==>");
			if(chdir(cmd[1]) < 0){
				printf("error : %s\n", strerror(errno));
			}
		}		
		if(i >= 1 || strcmp(cmd[0], "path")==0){
			if(i == 1){
				printf("path: %s\n", path);
			}else if((strcmp(cmd[1], "+") == 0) || (strcmp(cmd[1],"-") == 0)){
				if(i < 3){ 
					printf("error: Missing arguments");
				}else{
					if(strcmp(cmd[1], "+") == 0){

								
				
						printf("path argument: %s\n", cmd[2]);
						if(strlen(path) > 0){
  							path = concat(path, ":");
						}
						path = concat(path, cmd[2]);
					}
				
				}
				
			}else{
				
			}
		}		

		free(str);
		str = NULL;	
	}
}


