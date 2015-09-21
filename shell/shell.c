#include "shell.h"

char *history[MAX_HIST_SIZE + 1];

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
		free(buffer);
	};
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

void change_path_env(char *new_path)
{
	if (new_path == NULL)
		fprintf(stderr, "error: error! no value for PATH\n");
	else {
		if (strcmp(new_path, DEFAULT_PATH) == 0)
			/* change the PATH env to empty*/
			setenv("PATH", "", 1);
		else
			setenv("PATH", new_path, 1);
	};
	return;
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


void add_path(char *path_to_add)
{
	// char **curr_paths = malloc(sizeof(char *) * strlen(PATH_ENV));
	char *path_env    = malloc(sizeof(char) * strlen(PATH_ENV));

	strcpy(path_env, PATH_ENV);

	char **curr_paths = tokenizer(path_env, PATH_DELIM);;

	if (path_to_add == NULL) {
		fprintf(stderr, "error: can't add NULL path.\n");
		free(curr_paths);
		free(path_env);
		return;
	};
	char *new_path, *tmp_path = NULL;
	/* change to get path env from getenv, but not self_path */
	if (strcmp(PATH_ENV, DEFAULT_PATH) == 0)
		new_path = string_concat(PATH_ENV, path_to_add);
	else {
		

		// curr_paths = tokenizer(path_env, PATH_DELIM);
		char **path;

		for (path = curr_paths; *path; ++path) {
			if (strcmp(*path, path_to_add) == 0) {
				// printf("path_to_add: %s\n path: %s\n", path_to_add, *path);
				free(path_env);
				free(curr_paths);
				return;
			};
		};
		// new_path = string_concat(PATH_ENV, PATH_DELIM);
		// new_path = string_concat(new_path, path_to_add);
		tmp_path = string_concat(PATH_ENV, PATH_DELIM);
		new_path = string_concat(tmp_path, path_to_add);
	};

	change_path_env(new_path);
	if (tmp_path != NULL)
		free(tmp_path);
	free(new_path);
	free(path_env);
	free(curr_paths);
};

void delete_path(char *path_to_delete)
{
	if (path_to_delete == NULL)
		fprintf(stderr, "error: can't delete NULL path.\n");
	else {
		char *path_env  = malloc(sizeof(char) * strlen(PATH_ENV));
		
		strcpy(path_env, PATH_ENV);
		
		char **paths    = tokenizer(path_env, PATH_DELIM);
		// char *new_path  = "" ;//,*tmp_path = "";
		char **p;
		change_path_env(DEFAULT_PATH);
		/*for (p = paths; *p; ++p) {
			if (strcmp(*p, path_to_delete) == 0)
				continue;
			if (strcmp(new_path, "") == 0)
				new_path = string_concat(new_path, *p);
			else {
				// new_path = string_concat(new_path, PATH_DELIM);
				// new_path = string_concat(new_path, *p);
				tmp_path = string_concat(tmp_path, PATH_DELIM);
				new_path = string_concat(tmp_path, *p);
			};
		};*/
		for (p = paths; *p; ++p) {
			if (strcmp(*p, path_to_delete) == 0)
				continue;
			else {
				add_path(*p);
			};
		};
		free(paths);
		free(path_env);
		// if (strcmp(new_path, "") != 0)
		// 	free(new_path);
	};
	return;
};

int cmd_path(char **args)
{
	if (args[1] == NULL)
		printf("%s\n", PATH_ENV);
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
		int offset = strtol(args[1], NULL, 10);

		if (errno != ERANGE && errno != EINVAL) {
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

int isAllSpaces(char *buffer)
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
			if (!isAllSpaces(buffer))
				add_history(buffer);
			return buffer;
		};
		pos++;
		/* if exceed the max buffer size, realloc */
		/* TODO: need to make following functionize */
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

int cmd_launch(char **args, char *line)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) ==  -1) {
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
		free(args);
		free(line);
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

	change_path_env(DEFAULT_PATH);
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
};

int main(int argc, char **argv)
{
	cmd_loop();
	return EXIT_SUCCESS;
};
