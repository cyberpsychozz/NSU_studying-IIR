#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>  
#include <string.h> 

#define SOCKET_PATH "/tmp/my_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    ssize_t nread;

    // Удаляем старый сокет, если существует
    unlink(SOCKET_PATH);

    // Создаём сокет
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Привязываем сокет к пути
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Слушаем подключения
    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[Сервер] Ожидание подключения клиента...\n");

    // Принимаем подключение
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    printf("[Сервер] Клиент подключён. Чтение данных...\n");

    // Читаем данные от клиента
    while ((nread = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[nread] = '\0';  // Завершаем строку

        // Переводим в верхний регистр и выводим
        for (ssize_t i = 0; i < nread; i++) {
            putchar(toupper(buffer[i]));
        }
        fflush(stdout);
    }

    if (nread == -1) {
        perror("read failed");
    }

    // Закрываем сокеты и удаляем файл сокета
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("\n[Сервер] Соединение разорвано. Завершение работы.\n");

    return 0;
}