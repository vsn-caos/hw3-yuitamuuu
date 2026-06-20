#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <CMD1> <CMD2>\n", argv[0]);
        return 1;
    }

    int pipe_fds[2];

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t first_pid = fork();

    if (first_pid == -1) {
        perror("fork");
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return 1;
    }

    if (first_pid == 0) {
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);

        execlp(argv[1], argv[1], (char *) NULL);

        perror("execlp");
        _exit(1);
    }

    pid_t second_pid = fork();

    if (second_pid == -1) {
        perror("fork");

        close(pipe_fds[0]);
        close(pipe_fds[1]);

        waitpid(first_pid, NULL, 0);
        return 1;
    }

    if (second_pid == 0) {
        if (dup2(pipe_fds[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(pipe_fds[0]);
        close(pipe_fds[1]);

        execlp(argv[2], argv[2], (char *) NULL);

        perror("execlp");
        _exit(1);
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);

    if (waitpid(first_pid, NULL, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (waitpid(second_pid, NULL, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    return 0;
}