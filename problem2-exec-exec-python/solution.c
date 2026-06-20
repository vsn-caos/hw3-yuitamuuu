#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char *expression = NULL;
    size_t capacity = 0;

    ssize_t length = getline(&expression, &capacity, stdin);

    if (length == -1) {
        free(expression);
        return 1;
    }

    while (length > 0 &&
           (expression[length - 1] == '\n' ||
            expression[length - 1] == '\r')) {
        expression[length - 1] = '\0';
        length--;
    }

    execlp(
        "python3",
        "python3",
        "-c",
        "import sys; print(eval(sys.argv[1]))",
        expression,
        (char *) NULL
    );

    perror("execlp");
    free(expression);

    return 1;
}