#include <cstdio>

char str[] = "hello";

int main(void) {
    char c;
    while (1) {
        c = getchar();
        if (c == '\n') printf("%s", str);
    }
    return 0;
}
