#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Проверка наличия аргументов
    if (argc < 2) {
        printf("[LOG] Error: Usage %s <command> [args...]\n", argv[0]);
        return 1;
    }

    char *command = argv[1]; // Первая команда
    printf("[LOG] Program started. Command: %s (PID: %d)\n", command, getpid());

    // Создание порождённого процесса
    pid_t pid = fork();
    if (pid == -1) {
        perror("[LOG] Error forking process");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("[LOG] Child process started. Executing %s (PID: %d)\n", command, getpid());
        // Передача всех аргументов команде
        execvp(command, argv + 1); // argv + 1 пропускает имя программы
        perror("[LOG] Error executing command");
        exit(1);
    } else {
        // Родительский процесс
        printf("[LOG] Parent process started. Waiting for child (PID: %d)\n", getpid());

        // Ожидание завершения порождённого процесса
        int status;
        waitpid(pid, &status, 0);
        printf("[LOG] Child process finished (PID: %d)\n", getpid());

        // Вывод кода завершения
        if (WIFEXITED(status)) {
            printf("[LOG] Exit code of %s: %d (PID: %d)\n", command, WEXITSTATUS(status), getpid());
        } else {
            printf("[LOG] Process %s did not exit normally (PID: %d)\n", command, getpid());
        }

        printf("[LOG] Program finished (PID: %d)\n", getpid());
    }

    return 0;
}
