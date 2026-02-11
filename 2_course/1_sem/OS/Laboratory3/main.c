#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main() {
    
    printf("Step 1: Initial UIDs\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());


    printf("\nStep 2: Attempt to open file 'data.txt'\n");
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("fopen failed");
    } else {
        printf("File 'data.txt' opened successfully\n");
        fclose(file);
    }

    
    printf("\nStep 3: Setting real and effective UID\n");
    if (setuid(geteuid()) == -1) {
        perror("setuid failed");
        exit(EXIT_FAILURE);
    }
    printf("setuid(geteuid()) called successfully\n");

    
    printf("\nStep 4: UIDs after setuid\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    printf("\nAttempt to open file 'data.txt' again\n");
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("fopen failed");
    } else {
        printf("File 'data.txt' opened successfully\n");
        fclose(file);
    }

    return 0;
}