#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    
    long *offsets = NULL;
    int *lengths = NULL;
    int num_lines = 0;
    int capacity = 0;

    char ch;
    long current_offset = 0;
    int current_len = 0;

    while (read(fd, &ch, 1) == 1) {
        current_len++;
        current_offset++;

        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity == 0 ? 10 : capacity * 2;
                offsets = realloc(offsets, capacity * sizeof(long));
                lengths = realloc(lengths, capacity * sizeof(int));
                if (!offsets || !lengths) {
                    perror("realloc failed");
                    close(fd);
                    free(offsets);
                    free(lengths);
                    exit(EXIT_FAILURE);
                }
            }

            if (num_lines == 0) {
                offsets[0] = 0;
            }

            lengths[num_lines] = current_len - 1; 
            num_lines++;
            
            offsets[num_lines] = current_offset;

            current_len = 0;
        }
    }
    
    if (current_len > 0) {
        if (num_lines >= capacity) {
            capacity++;
            offsets = realloc(offsets, capacity * sizeof(long));
            lengths = realloc(lengths, capacity * sizeof(int));
            if (!offsets || !lengths) {
                perror("realloc failed");
                close(fd);
                free(offsets);
                free(lengths);
                exit(EXIT_FAILURE);
            }
        }

        lengths[num_lines] = current_len;
        num_lines++;
    }

    offsets = realloc(offsets, num_lines * sizeof(long));
    lengths = realloc(lengths, num_lines * sizeof(int));

    
    printf("Debug: Line table (line, offset, length)\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: offset = %ld, length = %d\n", i + 1, offsets[i], lengths[i]);
    }

    int line;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line) != 1) {
            printf("Invalid input. Try again.\n");
            continue;
        }

        if (line == 0) {
            break;
        }

        if (line < 1 || line > num_lines) {
            printf("Invalid line number. Must be between 1 and %d.\n", num_lines);
            continue;
        }

        if (lseek(fd, offsets[line - 1], SEEK_SET) == -1) {
            perror("lseek failed");
            continue;
        }

        char *buf = malloc(lengths[line - 1] + 1);
        if (!buf) {
            perror("malloc failed");
            continue;
        }

        ssize_t bytes_read = read(fd, buf, lengths[line - 1]);
        if (bytes_read != lengths[line - 1]) {
            perror("read failed");
            free(buf);
            continue;
        }

        buf[lengths[line - 1]] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(offsets);
    free(lengths);

    return 0;
}