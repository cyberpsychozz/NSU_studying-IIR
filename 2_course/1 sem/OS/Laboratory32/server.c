#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <aio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"
#define MAX_CLIENTS 20000
#define BUFFER_SIZE 4096
#define TARGET_CLIENTS 10000

struct client {
    int fd;
    struct aiocb cb;
    char buffer[BUFFER_SIZE];
} clients[MAX_CLIENTS];

int server_fd;
volatile int total_connected = 1;
volatile int total_finished = 0;

_Atomic int active_clients = 0;

void aio_completion_handler(union sigval sv) {
    struct aiocb *cb = sv.sival_ptr;
    struct client *c = NULL;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (&clients[i].cb == cb) {
            c = &clients[i];
            break;
        }
    }
    if (!c || c->fd == -1) return;

    ssize_t n = aio_return(cb);
    if (n > 0) {
        for (ssize_t j = 0; j < n; j++)
            c->buffer[j] = toupper((unsigned char)c->buffer[j]);
        write(STDOUT_FILENO, c->buffer, n);
        aio_read(cb);
    } else {
        close(c->fd);
        c->fd = -1;
        total_finished++;
        active_clients--;
    }
}

int main() {
    unlink(SOCKET_PATH);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 512);

    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].fd = -1;

    printf("AIO-сервер запущен\n");

    while (total_finished < TARGET_CLIENTS) {
        int fd;
        while ((fd = accept(server_fd, NULL, NULL)) != -1) {
            if (total_connected >= MAX_CLIENTS) { close(fd); break; }

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = fd;
                    total_connected++;
                    active_clients++;

                    memset(&clients[i].cb, 0, sizeof(struct aiocb));
                    clients[i].cb.aio_fildes = fd;
                    clients[i].cb.aio_buf = clients[i].buffer;
                    clients[i].cb.aio_nbytes = BUFFER_SIZE;
                    clients[i].cb.aio_sigevent.sigev_notify = SIGEV_THREAD;
                    clients[i].cb.aio_sigevent.sigev_notify_function = aio_completion_handler;
                    clients[i].cb.aio_sigevent.sigev_value.sival_ptr = &clients[i].cb;

                    aio_read(&clients[i].cb);
                    break;
                }
            }
        }

        if (total_connected >= TARGET_CLIENTS && total_connected % 1000 == 999) {
            printf("[AIO] Подключено: %d | Активно: %d | Завершено: %d\n",
                   total_connected, (int)active_clients, total_finished);
        }

        usleep(10000);
    }

    printf("AIO-сервер завершается\n");
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}