#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        fprintf(stderr, "N must be greater than 0\n");
        return 1;
    }

    int current = 1;

    while (1) {
        if (current == n) {
            printf("%d\n", current);
        } else {
            printf("%d ", current);
        }

        fflush(stdout);

        if (current == n) {
            return 0;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid > 0) {
            if (waitpid(pid, NULL, 0) == -1) {
                perror("waitpid");
                return 1;
            }

            return 0;
        }

        current++;
    }
}