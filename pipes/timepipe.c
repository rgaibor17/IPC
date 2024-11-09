#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAXLINE 1024

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <command> [args...]\n", argv[0]);
        exit(1);
    }

    int pipefd[2]; // pipefd[0] para leer, pipefd[1] para escribir
    pid_t pid;
    struct timeval start_time, end_time;

    // Syscall
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Proceso hijo
        close(pipefd[1]); // Se cierra el descriptor de escritura en el hijo

        // Se lee el tiempo de inicio
        if (read(pipefd[0], &start_time, sizeof(start_time)) <= 0) {
            perror("read");
            exit(1);
        }
        close(pipefd[0]); // Se cierra el descriptor de lectura del hijo

        // Hijo ejecuta el comando
        if (execvp(argv[1], &argv[1]) < 0)
        {
            printf(" %s: Comando no encontrado.\n", argv[0]);
            exit(1);
        }
    }

    // Proceso padre
    close(pipefd[0]); // Se cierra el descriptor de lectura del padre

    // Se registra el tiempo de inicio y escribe al descriptor de la pipe
    gettimeofday(&start_time, NULL);
    write(pipefd[1], &start_time, sizeof(start_time));
    close(pipefd[1]); // Se cierra el descriptor de escritura del padre

    // Se espera por el proceso hijo
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
        printf("\nHijo %d terminó con estado de salida: %d\n", pid, WEXITSTATUS(status));
    }

    // Se registra el tiempo de finalización
    gettimeofday(&end_time, NULL);

    // Se calcula el tiempo transcurrido
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long microseconds = end_time.tv_usec - start_time.tv_usec;
    if (microseconds < 0) {
        // Correct microseconds if there's a negative overflow
        microseconds += 1000000;
        seconds -= 1;
    }
    double elapsed = seconds + microseconds * 1e-6;

    printf("\nTiempo total de ejecución: %.6f seconds\n", elapsed);
    exit(0);
}
