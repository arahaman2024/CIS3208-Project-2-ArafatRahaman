#include <stdio.h>

#define MAX_LINE_LENGTH 256

int main() {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        printf("%s", line);
    }
    return 0;
}