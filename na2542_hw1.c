#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(){

	char *str, *token, ;
	while(1){
		printf("$");
		str = get_input();
		if(str == NULL){
			printf("No input received");
			continue;
		}
		token = strtok(str, " ");
		while(token != NULL){
			
		}
		
	}
	printf("%s", str);
}

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

