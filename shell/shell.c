#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char **environ;

int cmd_cd(char **args);
int cmd_pwd(char **args);
int cmd_path(char **args);
int cmd_history(char **args);
int cmd_exit(char **args);

char *origin_path_env, *self_path = "";

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

int builtin_str_leng(void) {
        return sizeof(builtin_str) / sizeof(char *);
};

int cmd_cd(char **args) {
        if (args[1] == NULL) {
                fprintf(stderr, "error: come on, cmd needs direcoty to move\n");
        }
        else if (chdir(args[1]) == -1) {
                perror("error");
        };
        return 1;
};

/* show the current working path, ignore all args */
int cmd_pwd(char **args) {
        char *buffer = malloc(64 * sizeof(char));
        /* set size = -1 to make getcwd malloc automatically */
        if (getcwd(buffer, -1) == NULL) { 
                perror("error");        
        }
        else {
                printf("%s\n", buffer);
                free(buffer);
        };
        return 1;
};

#define DEFAULT_PATH ""
// #define ORIGIN_PATH  "ORIGIN_PATH"

char *string_concat(char *str1, char *str2) {  
        int length = strlen(str1) + strlen(str2) + 1;  
        char *result = (char *)malloc(sizeof(char) * length);  
        strcpy(result, str1);  
        strcat(result, str2);  
        return result;  
}  

void change_path_env(char *new_path) {
        if (new_path == NULL) {
                fprintf(stderr, "error: error! no value for PATH\n");
                return;
        };
        if (strcmp(new_path, DEFAULT_PATH) == 0) {
                environ[15] = "PATH=";
        }
        else {
                
                // strcpy(tmp, environ[15]);
                // strcat(tmp, new_path);
                // printf("tmp: %s\n", tmp);
                // strcat(environ[15], new_path);
                char *new_path_env = string_concat(environ[15], new_path);
                environ[15] = new_path_env;
                // printf("environ: %s\n", environ[15]);
                // free(tmp);
        };
};

void add_path(char *path_to_add) {
        if (path_to_add == NULL){
                fprintf(stderr, "error: need path to add, not NULL path.\n");
                return;
        };
        char *tmp = malloc(sizeof(char) * (strlen(path_to_add) +               \
                                           strlen(self_path))  + 1);
        if (strcmp(self_path, DEFAULT_PATH) == 0) {
                strcpy(tmp, path_to_add);
                self_path = tmp;
        }
        else {
                // printf("here1\n");
                // strcpy(tmp, self_path);
                // printf("here2\n");

                strcpy(tmp, ":");
                // printf("here3\n");

                strcat(tmp, path_to_add);
                self_path = tmp;
        };
        // printf("tmp: %s\n", tmp);
        // printf("self_path: %s\n", self_path);;
        change_path_env(self_path);
        printf("%s\n", getenv("PATH"));
        free(tmp);
};

void delete_path(char *path_to_delete) {
        if (path_to_delete == NULL){
                fprintf(stderr, "error: need path to delete, not NULL path.\n");
                return;
        };
};

int cmd_path(char **args) {
        if (args[1] == NULL) {
                printf("%s\n", getenv("PATH"));
        }
        else { // with arguments
                if (strcmp(args[1], "-") == 0) {                        
                        delete_path(args[2]);
                }
                else if (strcmp(args[1], "+") == 0) {
                        add_path(args[2]);
                };
        };
        return 1;
};


int cmd_history(char **args) {
    return 1;
};

int cmd_exit(char **args) { // exit program, return 0
    return 0;
};

#define MAX_BUFF_SIZE 1024;

char *cmd_readline(void) {
     int buffer_size = MAX_BUFF_SIZE;
     int pos         = 0;
     char* buffer    = malloc(buffer_size * sizeof(char));
     char c;
     while(1) {
           c = getchar();
           if (c == EOF || c == '\n') {
               buffer[pos] = '\0';
               // printf("test read line: %s \n", buffer);
               return buffer;
           }
           else {
               buffer[pos] = c;
           };
           pos++;
           // if exceed the max buffer size, realloc
           if (pos >= buffer_size) {
               buffer_size += MAX_BUFF_SIZE;
               buffer = realloc(buffer, buffer_size);
               if (!buffer) {
                   fprintf(stderr, "allocation error!!\n");
                   exit(EXIT_FAILURE);
               };
           };
     };
};

#define MAX_TOK_BUFF_SIZE 64 
#define TOKEN_DELIM       " \t\n\r"

char **cmd_parse(char *line) {
        int buffer_size = MAX_TOK_BUFF_SIZE, pos = 0;
        char **tokens = malloc(buffer_size * sizeof(char*));
        char *token; //, **tokens_backup;
        token = strtok(line, TOKEN_DELIM);
        while (token != NULL) {
                tokens[pos] = token;
                pos++;
                /* if token exceed the max token buffer size, relloc */
                if (pos >= buffer_size) {
                        buffer_size += MAX_TOK_BUFF_SIZE;
                        tokens = realloc(tokens, buffer_size * sizeof(char*));
                        if (!tokens) {
                                fprintf(stderr, "allocation error!!\n");
                                exit(EXIT_FAILURE);
                        };
                };
                token = strtok(NULL, TOKEN_DELIM);
        };
        tokens[pos] = NULL;
        return tokens;
};

int cmd_launch(char **args) {
        pid_t pid;
        int status;
        pid = fork();
        if (pid == 0) { // child process
                if (execvp(args[0], args) ==  -1) {
                        perror("error");
                };
                exit(EXIT_FAILURE);
        }
        else if (pid > 0) { // parent process
                do {
                        waitpid(pid, &status, WUNTRACED);
                } while (!(WIFEXITED(status) || WIFSIGNALED(status)));
        }
        else { // pid < 0, error
                perror("error");
        }
        return 1;
};

int cmd_execute(char **args) {
        int i;
        if (args[0] == NULL) {
                /* empty command input, skip and continue */
                return 1;
        };
        for (i = 0; i < builtin_str_leng(); i++) { 
                if (strcmp(args[0], builtin_str[i]) == 0) {
                        return (*builtin_func[i])(args);
                };
        };
        return cmd_launch(args);
}; 

void cmd_loop(void) {
        char *line;
        char **args;
        int  status = 1;
        change_path_env(DEFAULT_PATH);
        while(status) {
                printf("$ ");
                line   = cmd_readline();
                args   = cmd_parse(line);
                status = cmd_execute(args);
                free(line);
                free(args);
        };
};

int main (int argc, char **argv) {
        cmd_loop();
        return EXIT_SUCCESS;
};