#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];

    // Открываем файл для чтения-записи (не создаём, если не существует)
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    // Структура для блокировки
    struct flock lock;
    lock.l_type = F_WRLCK;    // Исключающая блокировка на запись
    lock.l_start = 0;         // Блокировка с начала файла
    lock.l_whence = SEEK_SET; // Относительно начала файла
    lock.l_len = 0;           // Блокировка всего файла (0 = до конца)

    printf("Attempting advisory lock on %s...\n", filename);

    // Установка допустимой блокировки (advisory)
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            fprintf(stderr, "File %s is already locked by another process.\n", filename);
        } else {
            perror("fcntl advisory lock failed");
        }
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Advisory lock acquired successfully. File is now locked.\n");
    printf("Launching editor (vi). Edit the file and save to test locking.\n");

    // Вызов редактора через system(3)
    char editor_cmd[256];
    snprintf(editor_cmd, sizeof(editor_cmd), "nano %s", filename);
    int editor_status = system(editor_cmd);
    if (editor_status != 0) {
        fprintf(stderr, "Editor exited with status %d\n", editor_status);
    }

    // Разблокировка файла
    lock.l_type = F_UNLCK;  // Снятие блокировки
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl unlock failed");
    } else {
        printf("Lock released.\n");
    }

    close(fd);
    return 0;
}