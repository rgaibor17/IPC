#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <fcntl.h> 
#include <sys/mman.h>
#include <sys/stat.h> 
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAX_ARGS 99

#define SIZE sizeof(struct timeval)

int main(int argc, char **argv)
{
    struct timeval *shared_time;
    const char* name = "time_shmm";
    int shm_fd;

    pid_t pid;

    char buf[MAXLINE] = {0};
    int n, status;
    
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    shared_time = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_time == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        struct timeval *start_time, time;
        const char* name = "time_shmm";
        int shm_fd;
        
        shm_fd = shm_open(name, O_RDONLY, 0666);
        start_time = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
        gettimeofday(&time, NULL);
        *start_time = time;
        shm_unlink(name);

        // Hijo ejecuta el comando
        if(execvp(argv[0], argv) < 0)
        {
            printf(" %s: Comando no encontrado.\n", argv[0]);
            exit(1);
        }
    }

    wait(&status);
    if (WIFEXITED(status)) {
        printf("Hijo %d termino con estado de salida: %d\n", pid, WEXITSTATUS(status));
    }

    struct timeval end_time;
    gettimeofday(&end_time, NULL);
                struct tm *tm_info;
            tm_info = localtime(&shared_time->tv_sec);
            char buffer[30];
            strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
            printf("Current time: %s.%06ld\n", buffer, shared_time->tv_usec);


    // Tiempo transcurrido
    long int seconds = end_time.tv_sec - shared_time->tv_sec;
    long int microseconds = end_time.tv_usec - shared_time->tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    
    printf("Tiempo total de ejecución: %.6f seconds\n", elapsed);

    // Cleanup
    munmap(shared_time, SIZE);
    shm_unlink(name);

    exit(0);
}
