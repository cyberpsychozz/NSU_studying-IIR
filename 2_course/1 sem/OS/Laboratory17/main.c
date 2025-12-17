#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_LINE 40
#define BELL '\a'
#define CTRL(c) ((c) & 0x1F)  // Преобразование Ctrl+X → код

// Специальные символы 
#define ERASE   0x7F    // Backspace 
#define KILL    CTRL('U')
#define WORD_ERASE CTRL('W')
#define EOF_KEY CTRL('D')

// Глобальные переменные
struct termios orig_termios;
char line[MAX_LINE + 1] = {0};  
int pos = 0;                    

// Восстановление терминала при выходе
void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Настройка raw-режима
void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);  // Откл. эхо, канонич. режим, Ctrl-C/V, сигналы
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // Откл. Ctrl-S/Q
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);  // Откл. постобработку вывода
    raw.c_cc[VMIN] = 1;       // Читать по 1 байту
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Звуковой сигнал
void beep(void) {
    write(STDOUT_FILENO, "\a", 1);
}

// Удаление последнего слова (до пробела)
void erase_word(void) {
    // Удаляем пробелы с конца
    while (pos > 0 && line[pos-1] == ' ') {
        pos--;
        write(STDOUT_FILENO, "\b \b", 3);  
    }
    // Удаляем слово
    while (pos > 0 && line[pos-1] != ' ') {
        pos--;
        write(STDOUT_FILENO, "\b \b", 3);
    }
}

// Перенос слова на новую строку, если > 40 символов
void wrap_line(void) {
    if (pos <= MAX_LINE) return;

    
    int word_start = pos;
    while (word_start > 0 && line[word_start-1] != ' ') word_start--;

    if (word_start == 0) {
        
        write(STDOUT_FILENO, "\r\n", 2);
        // Копируем остаток строки
        memmove(line, line + MAX_LINE, pos - MAX_LINE);
        pos -= MAX_LINE;
        line[pos] = '\0';
        write(STDOUT_FILENO, line, pos);
    } else {
        // Переносим по пробелу
        write(STDOUT_FILENO, "\r\n", 2);
        memmove(line, line + word_start, pos - word_start);
        pos -= word_start;
        line[pos] = '\0';
        write(STDOUT_FILENO, line, pos);
    }

    // Курсор теперь в начале новой строки
}

int main(void) {
    enable_raw_mode();
    printf("Управление: CTRL + D в начале строки - выход\r\n CTRL+U - стирается последняя строка\r\n CTRL + W - стирает последнее слово\r\n");
    printf("Введите текст:\r\n> ");

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == EOF_KEY && pos == 0) {
            // Ctrl+D в начале строки — выход
            printf("\r\n[LOG] Выход по Ctrl+D\r\n");
            break;
        }

        if (c == ERASE && pos > 0) {
            // Backspace — стираем один символ
            pos--;
            line[pos] = '\0';
            write(STDOUT_FILENO, "\b \b", 3);
            continue;
        }

        if (c == KILL) {
            // Ctrl+U — стираем всю строку
            while (pos > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                pos--;
            }
            line[0] = '\0';
            continue;
        }

        if (c == WORD_ERASE) {
            // Ctrl+W — стираем последнее слово
            erase_word();
            line[pos] = '\0';
            continue;
        }

        if (c == '\r' || c == '\n') {
            // Enter — завершаем строку
            printf("\r\nВы ввели: %s\r\n", line);
            // Сброс строки
            pos = 0;
            line[0] = '\0';
            printf("> ");
            continue;
        }

        // Обычный печатаемый символ
        if (isprint(c) && pos < MAX_LINE) {
            line[pos++] = c;
            line[pos] = '\0';
            write(STDOUT_FILENO, &c, 1);
        } else if (isprint(c)) {
            // Попытка превысить 40 символов — переносим слово
            line[pos++] = c;
            line[pos] = '\0';
            wrap_line();
        } else {
            // Непечатаемый символ (кроме спец. клавиш) → звонок
            beep();
        }
    }

    restore_terminal();
    return 0;
}