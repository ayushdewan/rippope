#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *path = argv[1];
    FILE *fp = fopen(path, "rb");               // open for reading, binary-safe
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    // find file size
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        return EXIT_FAILURE;
    }
    long size = ftell(fp);
    rewind(fp);                                 // same as fseek(fp, 0, SEEK_SET)

    // allocate buffer (+1 for NUL if you want to treat it as a C-string)
    char *buf = malloc(size + 1);
    if (!buf) {
        perror("malloc");
        fclose(fp);
        return EXIT_FAILURE;
    }

    // read the whole file
    size_t nread = fread(buf, 1, size, fp);
    if (nread != (size_t)size) {
        perror("fread");
        free(buf);
        fclose(fp);
        return EXIT_FAILURE;
    }
    buf[size] = '\0';                           // optional NUL terminator

    // do something with buf here (e.g., printf("%s\n", buf);)
    printf("%s\n", buf);

    free(buf);                                  // free the buffer
    fclose(fp);                                 // close the file
    return EXIT_SUCCESS;
}

