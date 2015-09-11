#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int cmd_cd(char **args);
int cmd_pwd(char **args);
int cmd_path(char **args);
int cmd_history(char **args);
int cmd_exit(char **args);


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

#define MAX_BUFF_SIZE 1024;

int cmd_cd(char **args) {
    return 1;
};

int cmd_pwd(char **args) {
    return 1;
};

int cmd_path(char **args) {
    return 1;
};

int cmd_history(char **args) {
    return 1;
};

int cmd_exit(char **args) {
    return 1;
};

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
                   fprintf(stderr, "allocation error!!");
                   exit(EXIT_FAILURE);
               };
           };
     };
};

char **cmd_parse(char *line) {
     return 1;
};

int cmd_launch(char **args) {
    return 1;
};


int cmd_execute(char **args) {
    int i;
    if (args[0] == NULL) {
        // empty command input, skip and continue
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