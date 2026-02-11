#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h> 

#define SOCKET_PATH "/tmp/my_socket"

int main() {
    int client_fd;
    struct sockaddr_un addr;
    const char *text = "Hello, World! This Is A Test String.\n";

    // Создаём сокет
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Подключаемся к серверу
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("[Клиент] Подключён к серверу. Отправка текста...\n");

    // Отправляем текст
    write(client_fd, text, strlen(text));

    // Закрываем сокет — разрываем соединение
    close(client_fd);

    printf("[Клиент] Текст отправлен. Завершение работы.\n");

    return 0;
}