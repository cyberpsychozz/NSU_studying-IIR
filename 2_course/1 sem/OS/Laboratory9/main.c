#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "[LOG] Error: Usage %s <filename> (PID: %d)\n", argv[0], getpid());
        return 1;
    }

    char *filename = argv[1];
    printf("[LOG] Program started. Filename: %s (PID: %d)\n", filename, getpid());

    // Выбор части задания
    int choice;
    printf("Choose task part (1 or 2): ");
    scanf("%d", &choice);
    printf("[LOG] User chose part %d (PID: %d)\n", choice, getpid());

    if (choice == 1) {
        printf("[INFO] Part 1: Child runs cat after parent prints text. (PID: %d)\n", getpid());
    } else if (choice == 2) {
        printf("[INFO] Part 2: Parent waits for child to finish before printing last line. (PID: %d)\n", getpid());
    } else {
        printf("[LOG] Invalid choice. Exiting. (PID: %d)\n", getpid());
        return 1;
    }

    // Создание подпроцесса
    pid_t pid = fork();
    if (pid == -1) {
        perror("[LOG] Error forking process");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("[LOG] Child process started. (PID: %d)\n", getpid());
        printf("[LOG] Child executing cat on %s (PID: %d)\n", filename, getpid());
        execlp("cat", "cat", filename, NULL);
        perror("[LOG] Error executing cat");
        exit(1);
    } else {
        // Родительский процесс
        printf("[LOG] Parent process started. (PID: %d)\n", getpid());

        if (choice == 1) {
            // Часть 1: Родитель печатает текст сразу
            printf("[PARENT] Printing text while child runs... (PID: %d)\n", getpid());
            
            printf("[PARENT] Parent continues printing. (PID: %d)\n", getpid());
        } else if (choice == 2) {
            // Часть 2: Родитель допечатает только после завершения дочернего
            int status;
            waitpid(pid, &status, 0);
            printf("[LOG] Child process finished with status %d (PID: %d)\n", status, getpid());
            printf("[PARENT] Child finished, now printing last line. (PID: %d)\n", getpid());
        }

        if (choice == 2) {
            printf("[LOG] Program finished (PID: %d)\n", getpid());
        }
    }

    return 0;
}
