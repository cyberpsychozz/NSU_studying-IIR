#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>

// Печать типа файла и прав доступа
void print_mode(mode_t mode) {
    // Тип файла
    if (S_ISDIR(mode))      putchar('d');
    else if (S_ISREG(mode)) putchar('-');
    else if (S_ISLNK(mode)) putchar('l');
    else if (S_ISCHR(mode)) putchar('c');
    else if (S_ISBLK(mode)) putchar('b');
    else if (S_ISFIFO(mode))putchar('p');
    else if (S_ISSOCK(mode))putchar('s');
    else                    putchar('?');

    // Права: владелец
    putchar(mode & S_IRUSR ? 'r' : '-');
    putchar(mode & S_IWUSR ? 'w' : '-');
    putchar(mode & S_IXUSR ? 'x' : '-');

    // Группа
    putchar(mode & S_IRGRP ? 'r' : '-');
    putchar(mode & S_IWGRP ? 'w' : '-');
    putchar(mode & S_IXGRP ? 'x' : '-');

    // Остальные
    putchar(mode & S_IROTH ? 'r' : '-');
    putchar(mode & S_IWOTH ? 'w' : '-');
    putchar(mode & S_IXOTH ? 'x' : '-');
}

// Форматирование времени: "Oct 25 14:30" или "Oct 25  2023" (если старше 6 месяцев)
void print_time(time_t mtime) {
    struct tm *tm = localtime(&mtime);
    char buf[64];
    time_t now = time(NULL);
    
    if (now - mtime > 180 * 24 * 3600 || now < mtime) {
        
        strftime(buf, sizeof(buf), "%b %d  %Y", tm);
    } else {
        strftime(buf, sizeof(buf), "%b %d %H:%M", tm);
    }
    printf("%-12s", buf);
}

// Основная функция: печать строки как ls -ld
void print_ls_entry(const char *path) {
    struct stat st;
    
    // Используем lstat — чтобы символические ссылки не разыменовывались
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    // 1. Тип + права
    print_mode(st.st_mode);

    // 2. Количество жёстких ссылок (ширина 4)
    printf(" %4ld ", (long)st.st_nlink);

    // 3. Владелец
    struct passwd *pw = getpwuid(st.st_uid);
    printf("%-8s ", pw ? pw->pw_name : "unknown");

    // 4. Группа
    struct group *gr = getgrgid(st.st_gid);
    printf("%-8s ", gr ? gr->gr_name : "unknown");

    // 5. Размер — только для обычных файлов
    if (S_ISREG(st.st_mode)) {
        printf("%8ld ", (long)st.st_size);
    } else {
        printf("         ");  
    }

    // 6. Дата и время модификации
    print_time(st.st_mtime);


    char *name = basename((char*)path);
    printf(" %s\n", name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> [файл2] ...\n", argv[0]);
        return 1;
    }

    
    printf("%-10s %4s %-8s %-8s %8s %-12s %s\n",
           "Права", "Ссыл", "Владелец", "Группа", "Размер", "Дата", "Имя");
    printf("---------- ---- -------- -------- -------- ------------ ----------------------\n");

    for (int i = 1; i < argc; i++) {
        print_ls_entry(argv[i]);
    }

    return 0;
}