/*
 * procman.c
 * Implementación de un manejador de procesos simple.
 * Comentarios en español para explicar cada parte importante.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

#include "procman.h"

// Tabla global de procesos
process_info_t process_table[MAX_PROCESSES];
int process_count = 0;

// Mutex simple usando bloquear SIGCHLD durante secciones críticas
static void block_sigchld(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

static void unblock_sigchld(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

// Helper: encontrar índice en la tabla por pid
static int find_index_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; ++i)
        if (process_table[i].pid == pid) return i;
    return -1;
}

// Añade un proceso a la tabla
static int add_process_to_table(pid_t pid, const char *cmdline) {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (process_table[i].pid == 0) {
            process_table[i].pid = pid;
            strncpy(process_table[i].command, cmdline, sizeof(process_table[i].command)-1);
            process_table[i].command[sizeof(process_table[i].command)-1] = '\0';
            process_table[i].start_time = time(NULL);
            process_table[i].status = 0; // running
            process_count++;
            return 0;
        }
    }
    return -1; // tabla llena
}

// Remueve el proceso de la tabla
static void remove_process_from_table_index(int idx) {
    if (idx < 0 || idx >= MAX_PROCESSES) return;
    if (process_table[idx].pid != 0) {
        process_table[idx].pid = 0;
        process_table[idx].command[0] = '\0';
        process_table[idx].start_time = 0;
        process_table[idx].status = -1;
        process_count--;
        if (process_count < 0) process_count = 0;
    }
}

// ------------------- CREACIÓN DE PROCESOS -------------------
int create_process(const char *command, char *args[]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        // Proceso hijo: reset handlers
        signal(SIGINT, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        execvp(command, args);
        perror("execvp");
        _exit(127);
    }

    // Construir cadena de comando
    char cmdline[256];
    size_t off = snprintf(cmdline, sizeof(cmdline), "%s", command);
    for (int i = 1; args && args[i]; ++i) {
        off += snprintf(cmdline + off, sizeof(cmdline)-off, " %s", args[i]);
    }

    block_sigchld();
    add_process_to_table(pid, cmdline);
    unblock_sigchld();

    return pid;
}

// ------------------- TERMINAR PROCESO -------------------
int terminate_process(pid_t pid, int force) {
    int sig = force ? SIGKILL : SIGTERM;
    if (kill(pid, sig) == -1) {
        if (errno == ESRCH) return -1;
        perror("kill");
        return -1;
    }

    int status;
    pid_t r;
    do { r = waitpid(pid, &status, 0); } while (r == -1 && errno == EINTR);

    block_sigchld();
    int idx = find_index_by_pid(pid);
    if (idx != -1) remove_process_from_table_index(idx);
    unblock_sigchld();

    return 0;
}

// ------------------- CHECK STATUS -------------------
int check_process_status(pid_t pid) {
    int status;
    pid_t r = waitpid(pid, &status, WNOHANG);
    if (r == 0) return 1; // sigue corriendo
    else if (r == -1) return -1; // error
    else {
        block_sigchld();
        int idx = find_index_by_pid(pid);
        if (idx != -1) process_table[idx].status = 1; // terminated
        unblock_sigchld();

        return 0;
    }
}

// ------------------- LISTADO DE PROCESOS -------------------
static void format_runtime(time_t start, char *buf, size_t bufsz) {
    time_t diff = time(NULL) - start;
    int h = diff / 3600, m = (diff%3600)/60, s = diff%60;
    snprintf(buf, bufsz, "%02d:%02d:%02d", h, m, s);
}

void list_processes(void) {
    printf("PID\tCOMMAND\t\tRUNTIME\tSTATUS\n");
    printf("-----\t-------------\t--------\t----------\n");
    block_sigchld();
    for (int i=0;i<MAX_PROCESSES;i++) {
        if (process_table[i].pid != 0) {
            char runtime[16];
            format_runtime(process_table[i].start_time, runtime, sizeof(runtime));
            printf("%d\t%-15s\t%s\t%s\n",
                   process_table[i].pid,
                   process_table[i].command,
                   runtime,
                   process_table[i].status == 0 ? "Running" : "Terminated");
        }
    }
    unblock_sigchld();
}

// ------------------- ESPERAR TODOS -------------------
void wait_all_processes(void) {
    block_sigchld();
    for (int i=0;i<MAX_PROCESSES;i++) {
        pid_t pid = process_table[i].pid;
        if (pid != 0) {
            unblock_sigchld();
            int status;
            pid_t r;
            do { r = waitpid(pid, &status, 0); } while (r==-1 && errno==EINTR);
            if (r>0) printf("Terminated process %d\n", pid);
            block_sigchld();
            remove_process_from_table_index(i);
        }
    }
    unblock_sigchld();
    printf("All processes handled.\n");
}

// ------------------- MANEJO SEÑALES -------------------
void sigint_handler(int signum) {
    (void)signum;
    write(STDOUT_FILENO, "Shutting down gracefully...\n", 27);

    block_sigchld();
    for (int i=0;i<MAX_PROCESSES;i++)
        if (process_table[i].pid != 0) kill(process_table[i].pid, SIGTERM);
    unblock_sigchld();

    wait_all_processes();
    exit(0);
}

void sigchld_handler(int signum) {
    (void)signum;
    int saved_errno = errno;
    while(1) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) break;
        block_sigchld();
        int idx = find_index_by_pid(pid);
        if (idx != -1) process_table[idx].status = 1;
        unblock_sigchld();
    }
    errno = saved_errno;
}

// ------------------- ÁRBOL DE PROCESOS -------------------
typedef struct node { pid_t pid; pid_t ppid; char cmd[256]; } node_t;
#define MAX_NODES 1024
static node_t nodes[MAX_NODES];
static int node_count = 0;

static int read_proc_stat(pid_t pid, pid_t *ppid_out, char *comm_out, size_t comm_sz) {
    char path[256];
    snprintf(path,sizeof(path),"/proc/%d/stat",pid);
    FILE *fp = fopen(path,"r");
    if (!fp) return -1;
    int pid_read, ppid;
    char comm[256], state;
    if (fscanf(fp,"%d (%255[^)]) %c %d",&pid_read,comm,&state,&ppid)<4){
        fclose(fp); return -1;
    }
    fclose(fp);
    *ppid_out = ppid;
    strncpy(comm_out, comm, comm_sz-1);
    comm_out[comm_sz-1] = '\0';
    return 0;
}

static void build_process_list(void) {
    node_count = 0;
    DIR *d = opendir("/proc");
    if (!d) return;
    struct dirent *ent;
    while ((ent = readdir(d))) {
        if (!isdigit(ent->d_name[0])) continue;
        pid_t pid = atoi(ent->d_name);
        pid_t ppid;
        char comm[256];
        if (read_proc_stat(pid,&ppid,comm,sizeof(comm))==0) {
            if (node_count<MAX_NODES) {
                nodes[node_count].pid = pid;
                nodes[node_count].ppid = ppid;
                strncpy(nodes[node_count].cmd,comm,255);
                nodes[node_count].cmd[255]='\0';
                node_count++;
            }
        }
    }
    closedir(d);
}

static void print_tree_recursive(pid_t pid,int depth){
    for(int i=0;i<depth;i++) printf("  ");
    const char *name = "unknown";
    for(int i=0;i<node_count;i++)
        if(nodes[i].pid==pid){ name=nodes[i].cmd; break;}
    printf("[%d] %s\n",pid,name);
    for(int i=0;i<node_count;i++)
        if(nodes[i].ppid==pid)
            print_tree_recursive(nodes[i].pid,depth+1);
}

void print_process_tree(pid_t root_pid,int depth){
    build_process_list();
    print_tree_recursive(root_pid,depth);
}

