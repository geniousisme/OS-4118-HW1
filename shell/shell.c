#include "shell.h"

char *history[MAX_HIST_SIZE + 1];
char **path_list;

void init_path(void)
{
	path_list = malloc(sizeof(char *) * MAX_BUFF_SIZE);
};

char *builtin_str[] = {
	"cd",
	"pwd",
	"path",
	"history",
	"exit"
};

int (*builtin_func[])(char **) = {
	&cmd_cd,
	&cmd_pwd,
	&cmd_path,
	&cmd_history,
	&cmd_exit
};

int builtin_str_leng(void)
{
	return sizeof(builtin_str) / sizeof(char *);
};

int cmd_cd(char **args)
{
	if (args[1] == NULL)
		fprintf(stderr, "error: cd cmd needs direcoty to move\n");
	else if (chdir(args[1]) == -1)
		perror("error");
	return 1;
};

/* show the current working path, ignore all args */
int cmd_pwd(char **args)
{
	char *buffer = malloc(64 * sizeof(char));
	/* set size = -1 to make getcwd malloc automatically */
	if (getcwd(buffer, -1) == NULL)
		perror("error");
	else {
		printf("%s\n", buffer);
	};
	free(buffer);
	return 1;
};

char *string_concat(char *str1, char *str2)
{
	int length = strlen(str1) + strlen(str2) + 1;
	char *result = (char *)malloc(sizeof(char) * length);

	strcpy(result, str1);
	strcat(result, str2);
	return result;
};

void add_history(char *line)
{
	char *line_copy = malloc(sizeof(char) * (strlen(line) + 1));
	char *cmd       = malloc(sizeof(char) * (strlen(line) + 1));

	strcpy(line_copy, line);
	strcpy(cmd, line);
	cmd = strtok(cmd, TOKEN_DELIM);
	if (strcmp(cmd, "history") == 0 || strcmp(cmd, "exit") == 0) {
		free(cmd);
		free(line_copy);
		return;
	};

	int pos = 0;

	while (history[pos] != NULL)
		pos++;

	history[pos] = line_copy;

	/* update the history list  */
	if (pos >= MAX_HIST_SIZE) {
		int i = 0;

		free(history[i]);
		for (i = 1; i < MAX_HIST_SIZE + 1; i++)
			history[i - 1] = history[i];
		history[i - 1] = NULL;
	};
	free(cmd);
	return;
};

char **tokenizer(char *line, char *delim)
{
	int buffer_size = MAX_TOK_BUFF_SIZE, pos = 0;
	char **tokens   = malloc(buffer_size * sizeof(char *));
	char *token;

	token = strtok(line, delim);

	while (token != NULL) {
		tokens[pos] = token;
		pos++;
		/* if token exceed the max token buffer size, realloc */
		if (pos >= buffer_size) {
			buffer_size += MAX_TOK_BUFF_SIZE;
			tokens = realloc(tokens, buffer_size * sizeof(char *));
			if (!tokens) {
				fprintf(stderr, "allocation error!!\n");
				free(tokens);
				free(line);
				exit(EXIT_FAILURE);
			};
		};
		token = strtok(NULL, delim);
	};
	tokens[pos] = NULL;
	return tokens;
};

int path_len(void)
{
	int i = 0, count = 0;

	while (path_list[i] != NULL) {
		count++;
		i++;
	};
	return count;
};

int path_str_len(void)
{
	int length = 0, i = 0;

	while (path_list[i] != NULL) {
		length += strlen(path_list[i]);
		i++;
	};
	/* return the true final display length of path, include ':' numbers */
	return length + path_len() - 1;
};

void show_path(void)
{
	int curr_path_len = path_len(), i;

	for (i = 0; i < curr_path_len; i++) {
		if (i == 0)
			printf("%s", path_list[i]);
		else {
			printf(":");
			printf("%s", path_list[i]);
		};
	};
	printf("\n");
};

void add_path(char *path_to_add)
{
	if (path_to_add == NULL)
		return;

	char *new_path = malloc(sizeof(char) * strlen(path_to_add) + 1);
	int  i         = 0;

	strcpy(new_path, path_to_add);
	while (path_list[i] != NULL) {
		if (strcmp(path_list[i], path_to_add) != 0)
			i++;
		else {
			/* already in path_list, skip */
			free(new_path);
			return;
		};
	};
	path_list[i] = new_path;
};

void delete_path(char *path_to_delete)
{
	if (path_to_delete == NULL)
		return;

	int i = 0, curr_path_len = path_len();

	while (path_list[i] != 0) {
		if (strcmp(path_list[i], path_to_delete) != 0)
			i++;
		else {
			free(path_list[i]);
			for (i = i + 1; i < curr_path_len; i++)
				path_list[i - 1] = path_list[i];
			path_list[i - 1] = NULL;
			return;
		};
	};
};

int cmd_path(char **args)
{
	if (args[1] == NULL)
		show_path();
	else {
		if (strcmp(args[1], "-") == 0)
			delete_path(args[2]);
		else if (strcmp(args[1], "+") == 0)
			add_path(args[2]);
		else
			fprintf(stderr, "error: wrong params for path cmd.\n");
	};
	return 1;
};

void free_history(void)
{
	int pos;

	for (pos = 0; pos < MAX_HIST_SIZE + 1; pos++) {
		if (history[pos] != NULL)
			free(history[pos]);
	};
};

void free_path(void)
{
	int pos;
	int curr_path_len = path_len();

	for (pos = 0; pos < curr_path_len; pos++) {
		if (path_list[pos] != NULL)
			free(path_list[pos]);
	};
	free(path_list);
};

void init_history(void)
{
	int pos = 0;

	free_history();
	for ( ; pos < MAX_HIST_SIZE + 1; pos++) {
		history[pos] = NULL;
	};
};

int cmd_history(char **args)
{
	int i = 0;

	if (args[1] == NULL) {
		while (history[i] != NULL) {
			printf("%d %s\n", i, history[i]);
			i++;
		};
		return 1;
	};
	if (strcmp(args[1], "-c") == 0)
		init_history();
	else {
		/* set errno = 0 before you use strtol first. */
		errno = 0;
		char *to_convert = args[1];
		int offset = strtol(args[1], &to_convert, 10);

		if (errno != ERANGE && errno != EINVAL
					&& to_convert != args[1]) {
			if (offset < MAX_HIST_SIZE && history[offset] != NULL) {
				printf("%d %s\n", offset, history[offset]);
				add_history(history[offset]);
			} else {
				fprintf(stderr,
				"error: no history or offset out of range\n");
			};
		} else {
			fprintf(stderr,
				"error: invalid args or index for history.\n");
		};
	};
	return 1;
};

int cmd_exit(char **args)
{
	return 0;
};

int is_all_spaces(char *buffer)
{
	int i;

	for (i = 0; i < strlen(buffer); i++) {
		if (buffer[i] != ' ')
			return 0;
	};
	return 1;
};

char *cmd_readline(void)
{
	int buffer_size = MAX_BUFF_SIZE;
	int pos         = 0;
	char *buffer    = malloc(buffer_size * sizeof(char));
	char c;

	while (1) {
		c = getchar();
		if (!(c == EOF || c == '\n')) {
			buffer[pos] = c;
		} else {
			buffer[pos] = '\0';
			if (!is_all_spaces(buffer))
				add_history(buffer);
			return buffer;
		};
		pos++;
		/* if exceed the max buffer size, realloc */
		if (pos >= buffer_size) {
			buffer_size += MAX_BUFF_SIZE;
			buffer = realloc(buffer, buffer_size);
			if (!buffer) {
				fprintf(stderr, "allocation error!!\n");
				free(buffer);
				exit(EXIT_FAILURE);
			};
		};
	};
};


int is_executable(char **args)
{
	/* for /bin/ls case */
	if (args[0][0] == '/')
		/* false(0): execvp return -1, 0 */
		/* succeed(1): execvp return NULL, 1*/
		return execvp(args[0], args) != -1;
	/* for /bin/ls case */
	char *curr_dir    = malloc(sizeof(char) * 64);
	int  main_cmd_len = strlen(args[0]) + 1;
	int  cmd_size     = MAX_CMD_SIZE;

	getcwd(curr_dir, -1);

	char *exec_cmd = malloc(sizeof(char) * cmd_size);

	while (strlen(curr_dir) + main_cmd_len >= cmd_size) {
		cmd_size += cmd_size;
		exec_cmd = realloc(exec_cmd, cmd_size);
	};
	strcpy(exec_cmd, curr_dir);
	strcat(exec_cmd, "/");
	strcat(exec_cmd, args[0]);
	if (execvp(exec_cmd, args) == -1) {
		free(curr_dir);
		/* check all path + "/" + args[0] combinations */
		int i, curr_path_len = path_len();

		for (i = 0; i < curr_path_len; i++) {
			while (strlen(path_list[i]) + main_cmd_len
							>= cmd_size) {
				cmd_size += cmd_size;
				exec_cmd = realloc(exec_cmd, cmd_size);
			};
			strcpy(exec_cmd, path_list[i]);
			strcat(exec_cmd, "/");
			strcat(exec_cmd, args[0]);
			if (execvp(exec_cmd, args) != -1) {
				free(exec_cmd);
				return 1;
			};
		};
		free(exec_cmd);
		return 0;
	};
	free(curr_dir);
	free(exec_cmd);
	return 1;
};

int cmd_launch(char **args, char *line)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (!is_executable(args)) {
			free(args);
			free(line);
			perror("error");
		};
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!(WIFEXITED(status) || WIFSIGNALED(status)));
	} else {
		perror("error");
	};
	return 1;
};

int cmd_execute(char **args, char *line)
{
	int i;

	if (args[0] == NULL) {
		/* empty command input, skip and continue */
		return 1;
	};
	for (i = 0; i < builtin_str_leng(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0)
			return (*builtin_func[i])(args);
	};
	return cmd_launch(args, line);
};

void cmd_loop(void)
{	
	char *line;
	char **args;
	int  status = 1;

	init_path();
	init_history();
	while (status) {
		printf("$");
		line   = cmd_readline();
		args   = tokenizer(line, TOKEN_DELIM);
		status = cmd_execute(args, line);
		free(line);
		free(args);
	};
	free_history();
	free_path();
};

int main(int argc, char **argv)
{
	cmd_loop();
	return EXIT_SUCCESS;
};
