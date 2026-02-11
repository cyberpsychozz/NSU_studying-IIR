#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t count = 0;

void i() {
    signal(SIGINT, i);
    write(1, "\a", 1);
    count++;
}

void q() {
    printf("\nСигнал прозвучал %d раз\n", count);
    exit(0);
}

int main() {
    printf("Программа запущена.\n");
    printf("Нажмите Ctrl+C — будет звуковой сигнал.\n");
    printf("Нажмите Ctrl+\\ — программа завершится и покажет счётчик.\n\n");
    signal(SIGINT, i);
    signal(SIGQUIT, q);
    for (;;) {
        pause();
    }
}
