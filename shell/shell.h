#define MAX_HIST_SIZE     100
#define MAX_BUFF_SIZE     1024
#define MAX_TOK_BUFF_SIZE 64
#define DEFAULT_PATH      ""
#define TOKEN_DELIM       " \t\n\r"
#define PATH_DELIM        ":"
#define PATH_ENV          getenv("PATH")

int cmd_cd(char **args);
int cmd_pwd(char **args);
int cmd_path(char **args);
int cmd_history(char **args);
int cmd_exit(char **args);


