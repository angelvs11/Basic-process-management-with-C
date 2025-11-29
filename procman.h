#ifndef PROCMAN_H
#define PROCMAN_H


#include <sys/types.h>
#include <time.h>


#define MAX_PROCESSES 10


typedef struct {
pid_t pid; // PID del proceso hijo
char command[256]; // Comando completo (forma textual)
time_t start_time; // Hora de inicio
int status; // 0=running, 1=terminated, -1=error
} process_info_t;


extern process_info_t process_table[MAX_PROCESSES];
extern int process_count;


int create_process(const char *command, char *args[]);
int check_process_status(pid_t pid);
int terminate_process(pid_t pid, int force);
void list_processes(void);
void wait_all_processes(void);
void print_process_tree(pid_t root_pid, int depth);


#endif // PROCMAN_H
