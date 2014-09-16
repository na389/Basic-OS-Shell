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
int isspace(int argument);

/* Path variable to be used to maintain path of the
 * executables required to run the commands*/
char *path = "";
int stdin_copy, stdout_copy;

/*Function to concatenate two strings*/
char *concat(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL || (strcmp(s2, "") == 0))
		return s1;
	char *temp_malloc = (char *) malloc(strlen(s1)
			+ strlen(s2) + 1), *result;

	if (temp_malloc == NULL)
		return s1;
	result = temp_malloc;

	strcpy(result, s1);
	if (strcmp(s1, "") != 0) {
		free(s1);
		s1 = NULL;
	}
	strcat(result, s2);
	if (result == NULL) {
		printf("error: %s\n", strerror(errno));
		return s1;
	}
	return result;

}

/* Getting input from the user*/
char *get_input(void)
{
	int mem = 128;
	char *temp_malloc = (char *) malloc(mem);
	char *str, next, *temp_realloc;

	if (temp_malloc == NULL)
		return NULL;
	str = temp_malloc;

	fgets(str, mem, stdin);
	/*Reallocate memory until newline is encountered*/
	while (str[strlen(str) - 1] != '\n') {
		mem *= 2;
		temp_realloc = realloc(str, mem);
		if (temp_realloc == NULL) {
			while ((next = getchar()) != '\n' && next != EOF)
				;
			return NULL;
		}
		str = temp_realloc;
		fgets(str + mem / 2 - 1, mem / 2 + 1, stdin);
	}
	str[strlen(str) - 1] = '\0';
	return str;
}

/*Returns location of the executable for the given command*/
char *handle_commands(char *cmd)
{
	if (path == NULL || strcmp(path, "") == 0)
		return NULL;

	char *token, *str, *token_found;
	int flag = FALSE;
	char *temp_malloc = (char *) malloc(strlen(path) + 1);

	if (temp_malloc == NULL)
		return NULL;
	str = temp_malloc;
	strcpy(str, path);
	token = strtok(str, ":");
	while (token != NULL) {
		if (scan_directory(token, cmd) == TRUE) {
			char *temp = (char *) malloc(strlen(token) + 1);

			if (temp == NULL)
				return NULL;
			token_found = temp;
			strcpy(token_found, token);
			flag = TRUE;
		}
		token = strtok(NULL, ":");
	}

	if (flag == TRUE)
		return token_found;
	return NULL;

}

/* Locate if given target if present in the given
 * directory and return TRUE or FALSE */
int scan_directory(char *dir, char *target)
{
	int flag = FALSE;

	if (dir == NULL || strcmp(dir, "") == 0 || target == NULL ||
			strcmp(target, "") == 0)
		return FALSE;
	int n;
	struct dirent **entry;

	n = scandir(dir, &entry, 0, alphasort);
	if (n < 0)
		flag = FALSE;
	else {
		while (n--) {
			if (strcmp(entry[n]->d_name, target) == 0)
				flag = TRUE;
			free(entry[n]);
		}
		free(entry);
	}
	return flag;
}


/* Handling path command
 * If inappropriate input is found print relevant error message*/
void handle_path(int cmd_length, char *input[])
{
	char *path_token = "";

	if (cmd_length == 1)
		printf("%s\n", path);
	else if (cmd_length >= 3
			&& (strcmp(input[1], "+") == 0)) {
		if (strlen(path) > 0)
			path = concat(path, ":");
		path = concat(path, input[2]);
	} else if (cmd_length >= 3
			&& strcmp(input[1], "-") == 0
			&& strcmp(path, "") != 0) {
		char *b = "";

		path_token = strtok(path, ":");
		while (path_token) {
			if (strcmp(path_token, input[2]) != 0) {
				if (strlen(b) > 0)
					b = concat(b, ":");
				b = concat(b, path_token);
			}
			path_token = strtok(NULL, ":");
		}
		if (strcmp(path, "") != 0) {
			free(path);
			path = NULL;
		}
		path = b;
	} else {
		printf("error: invalid arguments path +/- expected\n");
	}

}

/*Fork the process to handle pipes*/
int fork_proc_exec(int in, int out,  char *cmd[], int len)
{
	pid_t pid = fork();
	int ret_err = 0;

	if (pid > 0) {
		int status, wait_failure;

		wait_failure = wait(&status);
		if (wait_failure == -1)
			return wait_failure;
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
	} else if (pid == 0) {
		if (in != 0) {
			//ret_err = dup2(in, 0);
			dup2(in, 0);
			if (ret_err == -1) {
				printf("error: %s\n", strerror(errno));
				exit(24);
			}
			ret_err = close(in);
			if (ret_err == -1) {
				printf("error: %s\n", strerror(errno));
				exit(24);
			}
		}
		if (out != 1) {
			ret_err = dup2(out, 1);
			if (ret_err == -1) {
				printf("error: %s\n", strerror(errno));
				exit(24);
			}
			ret_err = close(out);
			if (ret_err == -1){
				printf("error: %s\n", strerror(errno));
				exit(24);
			}
		}
		int return_val = 0;

		cmd[len] = NULL;
		return_val = execv(cmd[0], cmd);
		if (return_val == -1) {
			char *cmd_path = handle_commands(cmd[0]);

			if (cmd_path == NULL)
				exit(24);
			cmd_path = concat(cmd_path, "/");
			cmd_path = concat(cmd_path, cmd[0]);
			cmd[len] = NULL;
			execv(cmd_path, cmd);
			printf("error: %s\n", strerror(errno));
			exit(24);
		}
	}
	return 0;
}

/*Handle the pipes by changing the file descriptor to stdin and stdout*/
void pipe_exec(int n, char *cmnds[])
{
	int i, k, ret_spawn;
	int in, fd[2];
	char *token, *data[100];

	in = 0;
	for (i = 0; i < n-1; i++) {
		k = 0;
		ret_spawn = pipe(fd);
		token = strtok(cmnds[i], " \t");
		while (token != NULL) {
			data[k++] = token;
			token = strtok(NULL, " \t");
		}
		ret_spawn = fork_proc_exec(in, fd[1], data, k);
		ret_spawn = close(fd[1]);
		if (ret_spawn == -1 || ret_spawn > 0)
			break;
		in = fd[0];
	}
	if (ret_spawn == -1 || ret_spawn > 0) {
		/*Restoring standard output to write the error*/
		int ret = dup2(stdout_copy, STDOUT_FILENO);
		if (ret == -1) {
			printf("error: %s\n", strerror(errno));
			return;
		}
		printf("error: command not found\n");
		return;
	}
	k = 0;
	token = strtok(cmnds[i], " \t");
	while (token != NULL) {
		data[k++] = token;
		token = strtok(NULL, " \t");
	}
	data[k] = NULL;
	if (in != 0) {
		int ret = dup2(in, 0);
		if (ret == -1) {
			printf("error: %s\n", strerror(errno));
			return;
		}
	}


	pid_t pid = fork();

	if (pid > 0) {
		int status;

		wait(&status);
		if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
			printf("error: command not found\n");
			return;
		}
	} else if (pid == 0) {
		int return_val = 0;

		data[k] = NULL;
		return_val = execv(data[0], data);
		if (return_val == -1) {
			char *cmd_path = handle_commands(data[0]);

			if (cmd_path == NULL) {
				printf("error: command not found\n");
				exit(24);
			}
			cmd_path = concat(cmd_path, "/");
			cmd_path = concat(cmd_path, data[0]);
			data[k] = NULL;
			execv(cmd_path, data);
			printf("error: %s\n", strerror(errno));
			exit(0);
		}
	}

}

char *trim_string(char *str)
{
	char *end;

	/* Trim leading space*/
	while (isspace(*str))
		str++;

	if (*str == 0)  /* All spaces*/
		return str;

	/* Trim trailing space*/
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end))
		end--;

	/*Write new null terminator*/
	*(end+1) = 0;

	return str;
}

/*Check is string is valid for using pipe*/
int validate_string(char *str)
{
	char *res = trim_string(str);

	if (res == NULL || strcmp(res, "") == 0)
		return FALSE;

	char *temp_malloc = (char *) malloc(strlen(res) + 1), *test_input;

	if (temp_malloc == NULL) {
		printf("error: %s\n", strerror(errno));
		return 0;
	}
	test_input = temp_malloc;
	strcpy(test_input, res);
	free(str);

	int pipe_pos = 0, i = 0, len = strlen(test_input);

	if (test_input[0] == '|' || test_input[strlen(test_input) - 1] == '|')
		return 1;

	while (i < len) {
		if (pipe_pos == 0 && test_input[i] == '|')
			pipe_pos = 1;
		else if (pipe_pos == 1
				&& test_input[i] != '|'
				&& test_input[i] != ' '
				&& test_input[i] != '\t')
			pipe_pos = 0;
		else if (pipe_pos == 1 && test_input[i] == '|')
			break;
		i = i + 1;
	}

	return pipe_pos;
}



/*Function to handle pipes*/
void handle_pipes(char *input)
{
	if (input == NULL || strcmp(input, "") == 0) {
		printf("error: Invalid Command\n");
		return;
	}
	int length = 0;
	char *token, *data1, *data2;
	char *temp_malloc1 = (char *) malloc(strlen(input) + 1);
	char *temp_malloc2 = (char *) malloc(strlen(input) + 1);

	if (temp_malloc1 == NULL || temp_malloc2 == NULL)
		return;
	data1 = temp_malloc1;
	data2 = temp_malloc2;

	strcpy(data1, input);
	strcpy(data2, input);


	token = strtok(data1, "|");
	while (token != NULL) {
		length++;
		token = strtok(NULL, "|");
	}

	char *cmds[length];

	int i = 0;

	token = strtok(data2, "|");
	while (token != NULL) {
		cmds[i++] = token;
		token = strtok(NULL, "|");
	}
	free(data1);
	pipe_exec(length, cmds);
}


int main(void)
{

	char *str, *token, *cmd[100], *cmd_path = "";
	pid_t pid;
	int i = 0;
	/*Save stdin and stdout to restore after piping is handled*/
	stdin_copy = dup(STDIN_FILENO);
	stdout_copy = dup(STDOUT_FILENO);
	if (stdin_copy == -1 || stdout_copy == -1) {
		printf("error: %s\n", strerror(errno));
		exit(1);
	}
	while (1) {
		i = 0;
		printf("$");
		dup2(stdin_copy, STDIN_FILENO);
		str = (char *) get_input();
		if (str == NULL) {
			printf("error: %s\n", strerror(errno));
			exit(1);
		}
		if (strstr(str, "|") != NULL) {
			char *temp_malloc = (char *) malloc(strlen(str) + 1),
					*test_input;

			if (temp_malloc == NULL) {
				printf("error: %s\n", strerror(errno));
				continue;
			}
			test_input = temp_malloc;
			strcpy(test_input, str);
			if (validate_string(test_input) == 0) {
				handle_pipes(str);
				free(str);
				str = NULL;
			} else {
				printf("error: invalid command\n");
			}
			continue;
		}
		token = strtok(str, " \t");
		while (token != NULL) {
			cmd[i++] = token;
			token = strtok(NULL, " \t");
		}
		if (i == 0) {
			continue;
		} else if (strcmp(cmd[0], "exit") == 0) {
			free(str);
			str = NULL;
			exit(0);
		} else if (strcmp(cmd[0], "cd") == 0) {
			if (chdir(cmd[1]) < 0)
				printf("error: %s\n", strerror(errno));
		} else if (i >= 1 && strcmp(cmd[0], "path") == 0) {
			handle_path(i, cmd);
		} else {
			/*Handle the commands*/
			int return_val = 0;

			pid = fork();
			cmd[i] = NULL;
			if (pid < 0)
				continue;
			if (pid > 0) {
				int child_status;

				wait(&child_status);
			}
			if (pid == 0)
				return_val = execv(cmd[0], cmd);
			if (return_val == -1) {
				cmd_path = handle_commands(cmd[0]);
				if (cmd_path == NULL) {
					printf("error: command not found\n");
					exit(24);
				}
				cmd_path = concat(cmd_path, "/");
				cmd_path = concat(cmd_path, cmd[0]);
				cmd[i] = NULL;
				execv(cmd_path, cmd);
				printf("error: %s\n", strerror(errno));
				exit(24);
			}
		}
		free(str);
		str = NULL;
	}
	return 0;
}
