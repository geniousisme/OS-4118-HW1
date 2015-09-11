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

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int main (int argc, char **argv) {
    cmd_loop();
    return EXIT_SUCCESS;
};