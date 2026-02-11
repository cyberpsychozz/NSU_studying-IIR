#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 256

int main() {
    int sock_fd;
    struct sockaddr_un addr;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // printf("Подключено к серверу. Отправка сообщений...\n");

    const char *messages[] = {
        "Hello World!\n",
        "Test Message!!!",
        NULL
    };

    for (int i = 0; messages[i] != NULL; i++) {
        if (write(sock_fd, messages[i], strlen(messages[i])) == -1) {
            perror("write");
            break;
        }
        usleep(3000);
    }

    // printf("Сообщения отправлены.\n");

    close(sock_fd);

    return 0;
}