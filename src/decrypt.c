#include "decrypt.h"

int decrypt(char *encrypted_string) {
    printf("%s\n", encrypted_string);
    return 0;
}

int main() {
    char str[5] = "test";
    decrypt(str);
    return 0;
}
