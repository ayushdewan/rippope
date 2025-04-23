#include <stdlib.h>
#include <stdio.h>

#define INITIAL_ADD_BUFFER_CAPACITY 10
#define BUF_SELECT(piece_table, piece) ({piece_table.orig_buf, piece_table.add_buf.buf}[piece.buf_type])

// Append Only Buffer
typedef struct {
  size_t capacity;      // current capacity of internal buffer
  size_t len;           // current length of internal buffer
  char *buf;            // internal buffer (not null terminated)
} append_only_buffer;

// TODO: consider replacing type with just the pointer to buffer
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
typedef struct {
  pl_node *pre_node_p;
  pl_node *next_node_p;
  piece p;
} pl_node;

// Piece Table
typedef struct {
  char *orig_buf;             // buffer containing original file contents
  size_t orig_len;            // size of `orig` buffer
  append_only_buffer add_buffer; // buffer containing added text
  pl_node *piece_list_p;        // Linked List of pieces
} piece_table; 

// TODO: consider replacing/augmenting with logarithmic random access methods with bbst
// Comparison: Faster full table iteration at the cost of no random access
// Piece Table Iterator
typedef struct {
  piece_table *ptbl_p;        // pointer to piece table
  pl_node *curr_piece_node; // pointer to current node in piece list
  size_t piece_index;       // index of character in reference to current piece
  size_t global_index;      // index of character in reference to entire piece table
} piece_table_iterator;

piece_table create_piece_table(char *buf, size_t len) {
  // initialize append only internal buffer
  char *add_buf = malloc(INITIAL_ADD_BUFFER_CAPACITY);
  if (add_buf == NULL) {
    fprintf(stderr, "Error: piece table allocation failed");
    exit(1);
  }

  // initialize piece list with original buffer
  pl_node *head = malloc(sizeof(pl_node));
  if (head == NULL) {
    fprintf(stderr, "Error: piece table allocation failed");
    exit(1);
  }
  *head = (pl_node) {
    .pre_node_p = NULL,
    .next_node_p = NULL,
    .p = (piece) {
      .buf_type = ORIGINAL,
      .start = 0,
      .len = len,
    }
  };

  return (piece_table) {
    .orig_buf = buf,
    .orig_len = len,
    .add_buffer = (append_only_buffer) {
      .capacity = 10, // TODO: set a "better" default
      .len = 0,
      .buf = add_buf,
    },
  };
}

piece_table_iterator create_piece_table_iterator(piece_table *ptbl_p) {
  assert(ptbl_p != NULL);
  assert(ptbl_p->piece_list_p != NULL);

  return (piece_table_iterator) {
    .ptbl_p = ptbl_p,
    .curr_piece_node = ptbl_p->piece_list_p,
    .global_index = 0,
    .piece_index = 0,  
  };
}

// TODO: create similar function for adding multiple chars at once (for better copy paste support)
void append_char(append_only_buffer *add_buffer_p, char c) {
  assert(add_buffer_p != NULL);
  assert(add_buffer_p->buf != NULL);

  // resize buffer if capacity limit reached
  if (add_buffer_p->len == add_buffer_p->capacity) {
    add_buffer_p->capacity *= 2;
    add_buffer_p->buf = realloc(add_buffer_p->buf, add_buffer_p->capacity);
  }

  add_buffer_p->buf[add_buffer_p->len] = c;
  add_buffer_p->len++;
}

void insert_char(piece_table *ptbl_p, size_t cursor_pos, pl_node* cursor_hint) {
  assert(cursor_pos <= ptbl_p->orig_len + ptbl_p->add_buffer.len);
  assert(ptbl_p->piece_list_p != NULL);

  if (cursor_hint == NULL) {
    cursor_hint = ptbl_p->piece_list_p;
    size_t curr_len = ptbl_p->piece_list_p->p.len;
    while (curr_len < cursor_pos) {
      // assuming DS invariants hold this access should be safe
      assert(cursor_hint->next_node_p != NULL);
      cursor_hint = cursor_hint->next_node_p;
      curr_len += cursor_hint->p.len;
    }
  }
}