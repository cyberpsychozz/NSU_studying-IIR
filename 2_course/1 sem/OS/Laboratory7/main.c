#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

char *file_map = NULL; 
off_t file_size = 0; 
int first_input = 1;

void alarm_handler(int sig) {
    write(STDOUT_FILENO, file_map, file_size); 
    munmap(file_map, file_size);
    exit(0);
}

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

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    file_size = st.st_size;

    file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    long *offsets = NULL;
    int *lengths = NULL;
    int num_lines = 0;
    int capacity = 0;
    long current_offset = 0;
    int current_len = 0;

    for (off_t i = 0; i < file_size; i++) {
        current_len++;
        if (file_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity == 0 ? 10 : capacity * 2;
                offsets = realloc(offsets, capacity * sizeof(long));
                lengths = realloc(lengths, capacity * sizeof(int));
                if (!offsets || !lengths) {
                    perror("realloc failed");
                    munmap(file_map, file_size);
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
            offsets[num_lines] = i + 1;
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
                munmap(file_map, file_size);
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

    signal(SIGALRM, alarm_handler);

    int line;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (first_input) {
            alarm(5); 
        }

        if (scanf("%d", &line) != 1) {
            if (first_input) {
                alarm(0); 
            }
            printf("Invalid input. Try again.\n");
            while (getchar() != '\n'); 
            first_input = 0; 
            continue;
        }

        if (first_input) {
            alarm(0); 
            first_input = 0;
        }

        if (line == 0) {
            break;
        }

        if (line < 1 || line > num_lines) {
            printf("Invalid line number. Must be between 1 and %d.\n", num_lines);
            continue;
        }

        char *buf = malloc(lengths[line - 1] + 1);
        if (!buf) {
            perror("malloc failed");
            continue;
        }

        memcpy(buf, file_map + offsets[line - 1], lengths[line - 1]);
        buf[lengths[line - 1]] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    munmap(file_map, file_size);
    free(offsets);
    free(lengths);

    return 0;
}