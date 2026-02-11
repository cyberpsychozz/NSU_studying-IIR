#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10240
#define TARGET_CLIENTS 10000

int main() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0};
    fd_set readfds, allfds;
    int clients[MAX_CLIENTS] = {0};
    int max_fd = server_fd;
    char buf[BUFFER_SIZE];
    int total = 0, finished = 0;

    unlink(SOCKET_PATH);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 128);

    FD_ZERO(&allfds);
    FD_SET(server_fd, &allfds);

    printf("select-сервер запущен\n");

    while (1) {
        readfds = allfds;
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) <= 0) continue;

        if (FD_ISSET(server_fd, &readfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd != -1) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] == 0) {
                        clients[i] = fd;
                        FD_SET(fd, &allfds);
                        if (fd > max_fd) max_fd = fd;
                        total++;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i];
            if (fd == 0 || !FD_ISSET(fd, &readfds)) continue;

            int n = read(fd, buf, BUFFER_SIZE);
            if (n <= 0) {
                close(fd);
                FD_CLR(fd, &allfds);
                clients[i] = 0;
                finished++;
            } else {
                for (int j = 0; j < n; j++) buf[j] = toupper(buf[j]);
                write(1, buf, n);
            }
        }

        if (total >= TARGET_CLIENTS && finished >= TARGET_CLIENTS) {
            printf("select-сервер завершается\n");
            break;
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}