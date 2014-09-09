#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

extern int errno;

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
	return str;
}


/*Handle commands*/
void handleCommands(char* cmd[]){
	
}

int main(){

	char *str, *token, *cmd[100];
	int i = 0;
	while(1){
		printf("$");
		str = (char*)get_input();
		printf("string : %s\n", str);
		if(str == NULL){
			printf("error : %s\n", strerror(errno));
		}
		token = strtok(str, " \t");
		while(token != NULL){
			cmd[i++] = token;
			printf("%s\n", token);
			token = strtok(NULL, " \t");
		}
		printf("token 1: %s\n", cmd[0]);
		if(strcmp(cmd[0], "exit") == 0){
			break;	
		}
		if(strcmp(cmd[0], "cd") == 0){
			printf("==>");
			if(chdir(cmd[1]) < 0){
				printf("error : %s\n", strerror(errno));
			}
		}		
		free(str);
		str = NULL;	
	}
}


