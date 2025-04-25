#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../piece_table.c"

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
    char *buf = malloc(size);
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
    
    piece_table ptbl = create_piece_table(buf, size);
    ptbl_insert_char(&ptbl, '&');
    ptbl_display(&ptbl);
    
    ptbl_update_global_cursor_pos(&ptbl, 37);
    char *last_line = "i'm the last line\n";
    for(int i = 0; i < strlen(last_line); i++) {
      ptbl_insert_char(&ptbl, last_line[i]);
    }

    printf("---------------------\n");
    ptbl_display(&ptbl);

    ptbl_update_global_cursor_pos(&ptbl, 7);
    char *middle_line = "cheeseburgers ";
    for(int i = 0; i < strlen(middle_line); i++) {
      ptbl_insert_char(&ptbl, middle_line[i]);
    }
    printf("---------------------\n");
    ptbl_display(&ptbl);

    ptbl_update_global_cursor_pos(&ptbl, 13);
    for (int i = 0; i < 6; i++) {
      ptbl_delete_char(&ptbl);
      printf("---------------------\n");
      ptbl_display(&ptbl);
    }

    ptbl_update_global_cursor_pos(&ptbl, 1);
    ptbl_delete_char(&ptbl);

    printf("---------------------\n");
    ptbl_display(&ptbl);

    free_piece_table(&ptbl);
    free(buf);                                  // free the buffer
    fclose(fp);                                 // close the file
    return EXIT_SUCCESS;
}

