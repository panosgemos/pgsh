#ifndef PROCESSES_H
#define PROCESSES_H
pid_t create_child_func( void (*func)(void));
pid_t create_child_full( char *cmd, char **args );
pid_t create_child( char **args );
int wait_child(pid_t pid);
char *enter_command();
pid_t create_child_r(char **cmd, char *input, char *output, int append);
int spawn_proc (char **command, int in, int out);
int pipe_chain(char ***commands, int n, int inFd, int outFd);
int pipe_chain_r(char ***commands, int n, char *input, char *output, int append);

#endif
