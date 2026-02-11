#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>  
#include <string.h> 

int main() {
    int pipefd[2];  // Массив для дескрипторов канала: [0] - чтение, [1] - запись
    pid_t pid;

    // Создаём канал
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // Создаём подпроцесс
    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {  // Родительский процесс
        // Закрываем конец чтения в родителе
        close(pipefd[0]);

        // Текст для отправки (смешанный регистр)
        const char *text = "Hello, World! NSu.\n";

        // Записываем текст в канал
        write(pipefd[1], text, strlen(text));

        // Закрываем конец записи
        close(pipefd[1]);

        // Ждём завершения ребёнка
        wait(NULL);
    } else {  // Дочерний процесс
        // Закрываем конец записи в ребёнке
        close(pipefd[1]);

        char buf[1024];
        ssize_t nread;

        // Читаем из канала
        while ((nread = read(pipefd[0], buf, sizeof(buf))) > 0) {
            // Переводим в верхний регистр и выводим
            for (ssize_t i = 0; i < nread; i++) {
                putchar(toupper(buf[i]));
            }
        }

        // Закрываем конец чтения
        close(pipefd[0]);
    }

    return 0;
}