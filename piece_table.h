#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include <assert.h>
#include <stdio.h>

#define INITIAL_ADD_BUFFER_CAPACITY 10
#define RED_C "\x1b[31m"
#define BLUE_C "\x1b[34m"
#define RESET "\x1b[0m"

#define EDIT_TEXT_BUFFER_MAX_SIZE 256
#define MAX_LINE_BREAKS 256

// Append Only Buffer
typedef struct {
  size_t capacity; // current capacity of internal buffer
  size_t len;      // current length of internal buffer
  char *buf;       // internal buffer (not null terminated)
} append_only_buffer;

// Type that marks which buffer a piece references
typedef enum {
  ORIGINAL,
  ADD,
} buffer_type;

// Piece
typedef struct {
  buffer_type buf_type; // denotes buffer to look into
  size_t start;         // starting character of piece
  size_t len;           // length of piece
} piece;

// Linked list of pieces, pl = "piece list"
typedef struct pl_node {
  struct pl_node *pre_node_p;
  struct pl_node *next_node_p;
  piece p;
} pl_node;

// Piece Table
typedef struct {
  char *orig_buf;  // buffer containing original file contents (user owned)
  size_t orig_len; // size of `orig` buffer
  append_only_buffer add_buffer; // buffer containing added text
  pl_node *piece_list_head_p;    // linked List of pieces
  size_t global_cursor_pos;      // position of cursor in table
  size_t local_cursor_pos;       // position of cursor in piece
  pl_node *cursor_hint;          // piece list node containing cursor
} piece_table;

// Piece Table Iterator
typedef struct {
  piece_table *ptbl_p;      // pointer to piece table
  pl_node *curr_piece_node; // pointer to current node in piece list
  size_t piece_index;       // index of character in reference to current piece
} piece_table_iterator;

// export data to be used in a rendering engine
typedef struct {
  char edit_text_buf[EDIT_TEXT_BUFFER_MAX_SIZE];
  size_t edit_text_len;

  int line_break_pos[MAX_LINE_BREAKS];
  size_t num_line_breaks;
  char line_numbers[MAX_LINE_BREAKS][8];

  size_t cursor_line;
  size_t cursor_offset;
} render_buffers;

// Function prototypes
piece_table create_piece_table(char *buf, size_t len);
void free_piece_table(piece_table *ptbl_p);
piece_table_iterator create_ptbl_iterator(piece_table *ptbl_p);
char query_ptbl_iterator(piece_table_iterator *pti_p);
void advance_ptbl_iterator(piece_table_iterator *pti_p);
int ptbl_iterator_end(piece_table_iterator *pti_p);
void create_line_number(render_buffers *render_bufs_p, size_t line);
void load_ptbl_data(piece_table *ptbl_p, render_buffers *render_bufs_p);
void pl_place_between(pl_node *x, pl_node *y, pl_node *z);
void aob_append_char(append_only_buffer *add_buffer_p, char c);
void ptbl_insert_char(piece_table *ptbl_p, char c);
void ptbl_delete_char(piece_table *ptbl_p);
void ptbl_update_global_cursor_pos(piece_table *ptbl_p, size_t new_global_cursor_pos);
void ptbl_display(piece_table *ptbl_p);

#endif // PIECE_TABLE_H
