#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "procman.h"

#define MAX_CMD_LEN 256

// ---- Manejador de señal SIGINT ----
void handle_sigint(int sig) {
    (void)sig;
    printf("\nShutting down\n");
    wait_all_processes();
    exit(0);
}

// ---- Programa principal ----
int main(void) {
    char line[MAX_CMD_LEN];

    // Registrar el manejador de SIGINT
    signal(SIGINT, handle_sigint);

    printf("=== Bienvenido a ProcMan ===\n");
    printf("Comandos disponibles: help, create <cmd> [args], list, kill <pid>, wait, tree, quit\n");

    while (1) {
        printf("ProcMan> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        // Quitar salto de línea
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "quit") == 0) {
            printf("Saliendo de ProcMan...\n");
            break;
        } else if (strcmp(line, "help") == 0) {
            printf("Comandos:\n");
            printf("  help                   - Muestra esta ayuda\n");
            printf("  create <cmd> [args...] - Crea un nuevo proceso\n");
            printf("  list                   - Lista todos los procesos\n");
            printf("  kill <pid> [force]     - Termina un proceso\n");
            printf("  wait                   - Espera a que terminen todos los procesos\n");
            printf("  tree                   - Muestra árbol de procesos\n");
            printf("  quit                   - Salir del programa\n");
        } else if (strncmp(line, "create ", 7) == 0) {
            char *cmdline = line + 7;
            char *args[3];
            args[0] = "bash";
            args[1] = "-c";
            args[2] = cmdline;
            args[3] = NULL;

            pid_t pid = create_process(args[0], args);
            if (pid > 0)
                printf("Created process %d\n", pid);
            else
                printf("Error creating process\n");

        } else if (strcmp(line, "list") == 0) {
            list_processes();
        } else if (strncmp(line, "kill ", 5) == 0) {
            char *rest = line + 5;
            char *pid_str = strtok(rest, " ");
            char *force_str = strtok(NULL, " ");
            int force = (force_str != NULL && strcmp(force_str, "1") == 0) ? 1 : 0;
            pid_t pid = atoi(pid_str);
            if (terminate_process(pid, force) == 0)
                printf("Terminated process %d\n", pid);
            else
                printf("Error terminating process %d\n", pid);
        } else if (strcmp(line, "wait") == 0) {
            printf("Waiting for all child processes...\n");
            wait_all_processes();
        } else if (strcmp(line, "tree") == 0) {
            printf("Árbol de procesos:\n");
            print_process_tree(getpid(), 0);
        } else if (strlen(line) > 0) {
            printf("Comando desconocido: %s\n", line);
        }
    }

    // Al salir, cerrar procesos restantes solo si hay alguno
    if (process_count > 0) {
        wait_all_processes();
    }

    return 0;
}

