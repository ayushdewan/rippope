#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define INITIAL_ADD_BUFFER_CAPACITY 10
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

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
typedef struct pl_node {
  struct pl_node *pre_node_p;
  struct pl_node *next_node_p;
  piece p;
} pl_node;

// Piece Table
typedef struct {
  char *orig_buf;                 // buffer containing original file contents (user owned)
  size_t orig_len;               // size of `orig` buffer
  append_only_buffer add_buffer; // buffer containing added text
  pl_node *piece_list_head_p;         // linked List of pieces
  size_t global_cursor_pos;     // position of cursor in table
  size_t local_cursor_pos;     // position of cursor in piece
  pl_node *cursor_hint;         // piece list node containing cursor
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
  pl_node *head = NULL;
  if (len > 0) {
    head = (pl_node *) malloc(sizeof(pl_node));
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
  }

  return (piece_table) {
    .orig_buf = buf,
    .orig_len = len,
    .add_buffer = (append_only_buffer) {
      .capacity = 10, // TODO: set a "better" default
      .len = 0,
      .buf = add_buf,
    },
    .piece_list_head_p = head,
    .global_cursor_pos = 0,
    .local_cursor_pos = 0,
    .cursor_hint = head,
  };
}

void free_piece_table(piece_table *ptbl_p) {
  free(ptbl_p->add_buffer.buf);

  pl_node *iter = ptbl_p->piece_list_head_p;
  while (iter != NULL) {
    pl_node *next = iter->next_node_p;
    free(iter);
    iter = next;
  }
}

piece_table_iterator create_ptbl_iterator(piece_table *ptbl_p) {
  assert(ptbl_p != NULL);
  assert(ptbl_p->piece_list_head_p != NULL);

  return (piece_table_iterator) {
    .ptbl_p = ptbl_p,
    .curr_piece_node = ptbl_p->piece_list_head_p,
    .global_index = 0,
    .piece_index = 0, 
  };
}

char query_ptbl_iterator(piece_table_iterator *pti_p) {
  assert(pti_p != NULL);
  assert(pti_p->curr_piece_node != NULL);
  assert(pti_p->ptbl_p != NULL);

  piece curr_piece = pti_p->curr_piece_node->p;
  size_t index = curr_piece.start + pti_p->piece_index;
  switch(curr_piece.buf_type) {
    case ORIGINAL:
      assert(index < pti_p->ptbl_p->orig_len);
      return pti_p->ptbl_p->orig_buf[index];
    case ADD:
      assert(index < pti_p->ptbl_p->add_buffer.len);
      return pti_p->ptbl_p->add_buffer.buf[index];
  }
}

void advance_ptbl_iterator(piece_table_iterator *pti_p) {
  assert(pti_p != NULL);
  assert(pti_p->curr_piece_node != NULL);
  assert(pti_p->ptbl_p != NULL);
  
  pti_p->piece_index++;
  if (pti_p->piece_index == pti_p->curr_piece_node->p.len) {
    pti_p->curr_piece_node = pti_p->curr_piece_node->next_node_p;
    pti_p->piece_index = 0;
  }
}

// Assumption that ... <-> x <-> z <-> ... (and at most one may be NULL)
// Turns into ... <-> x <-> y <-> z <-> ...
void pl_place_between(pl_node *x, pl_node *y, pl_node *z) {
  assert(y != NULL);
  y->next_node_p = z;
  y->pre_node_p = x;
  if (z != NULL) {
    z->pre_node_p = y;
  }
  if (x != NULL) {
    x->next_node_p = y;
  }
};

// TODO: create similar function for adding multiple chars at once (for better copy paste support)
void aob_append_char(append_only_buffer *add_buffer_p, char c) {
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

// TODO: create similar function for adding multiple chars at once (for better copy paste support)
void ptbl_insert_char(piece_table *ptbl_p, char c) {
  assert(ptbl_p != NULL);
  assert(ptbl_p->global_cursor_pos <= ptbl_p->orig_len + ptbl_p->add_buffer.len); // TODO: revise this check with a better notion of total piece table length

  // populate add buffer
  aob_append_char(&ptbl_p->add_buffer, c);

  // empty piece table case
  if (ptbl_p->piece_list_head_p == NULL) {
    // allocate a new add piece
    pl_node *new_node = (pl_node *)malloc(sizeof(pl_node));
    if (new_node == NULL) {
      // TODO: handle error
    }
    *new_node = (pl_node) {
      .pre_node_p = NULL,
      .next_node_p = NULL,
      .p = (piece) {
        .buf_type = ADD,
        .start = ptbl_p->add_buffer.len - 1,
        .len = 1,
      },
    };

    ptbl_p->piece_list_head_p = new_node;
    ptbl_p->cursor_hint = new_node;
    ptbl_p->local_cursor_pos = 1;
    ptbl_p->global_cursor_pos = 1;
    return;
  }

  pl_node *cursor_hint = ptbl_p->cursor_hint;
  assert(cursor_hint != NULL);
  assert(cursor_hint->p.len >= ptbl_p->local_cursor_pos);
  
  // current piece contains end of add buffer
  int is_end = cursor_hint->p.len == ptbl_p->local_cursor_pos; 
  if (cursor_hint->p.buf_type == ADD && is_end && cursor_hint->p.start + cursor_hint->p.len == ptbl_p->add_buffer.len - 1) {
    cursor_hint->p.len++;
    ptbl_p->local_cursor_pos++;
    ptbl_p->global_cursor_pos++;
    return;
  }

  // allocate a new add piece
  pl_node *new_node = (pl_node *)malloc(sizeof(pl_node));
  if (new_node == NULL) {
    // TODO: handle error
  }
  *new_node = (pl_node) {
    .pre_node_p = NULL,
    .next_node_p = NULL,
    .p = (piece) {
      .buf_type = ADD,
      .start = ptbl_p->add_buffer.len - 1,
      .len = 1,
    },
  };

  if (is_end) {
    pl_place_between(cursor_hint, new_node, cursor_hint->next_node_p);
  } else if (ptbl_p->local_cursor_pos == 0) {
    if (cursor_hint->pre_node_p == NULL) {
      ptbl_p->piece_list_head_p = new_node;
    }
    pl_place_between(cursor_hint->pre_node_p, new_node, cursor_hint);
  } else {
    // split current piece by shortening it and allocating new piece
    pl_node *split_node = (pl_node*)malloc(sizeof(pl_node));
    if (split_node == NULL) {
      // TODO: handle error
    }
    *split_node = (pl_node) {
      .pre_node_p = NULL,
      .next_node_p = NULL,
      .p = (piece) {
        .buf_type = cursor_hint->p.buf_type,
        .start = ptbl_p->local_cursor_pos,
        .len = cursor_hint->p.len - ptbl_p->local_cursor_pos,
      },
    };

    cursor_hint->p.len = ptbl_p->local_cursor_pos;
    pl_place_between(cursor_hint, split_node, cursor_hint->next_node_p);
    pl_place_between(cursor_hint, new_node, split_node);
  }

  // update cursor state
  ptbl_p->local_cursor_pos = 1;
  ptbl_p->global_cursor_pos++;
  ptbl_p->cursor_hint = new_node;
}

void ptbl_delete_char(piece_table *ptbl_p) {
  assert(ptbl_p != NULL);
  assert(ptbl_p->global_cursor_pos <= ptbl_p->orig_len + ptbl_p->add_buffer.len); // TODO: revise this check with a better notion of total piece table length
  assert(ptbl_p->piece_list_head_p != NULL);

  // do nothing if cursor is at beginning of file
  if (ptbl_p->global_cursor_pos == 0) {
    return;
  }

  pl_node *cursor_hint = ptbl_p->cursor_hint;
  assert(cursor_hint != NULL);
  assert(cursor_hint->p.len >= ptbl_p->local_cursor_pos);

  ptbl_p->global_cursor_pos--;

  // move cursor hint to previous block if on first index
  if (ptbl_p->local_cursor_pos == 0) {
    assert(cursor_hint->pre_node_p != NULL);
    cursor_hint = cursor_hint->pre_node_p;
    ptbl_p->cursor_hint = cursor_hint;
    ptbl_p->local_cursor_pos = cursor_hint->p.len;
  }

  // 3 cases where no split needed:
  // Case 1: deleting 1-length piece (list removal & free needed)
  // Case 2: deleting from end of piece
  // Case 3: deleting from beginning of piece
  if (cursor_hint->p.len == 1) {
    if (cursor_hint->next_node_p != NULL) {
      cursor_hint->next_node_p->pre_node_p = cursor_hint->pre_node_p;
    }
    if (cursor_hint->pre_node_p == NULL) {
      ptbl_p->piece_list_head_p = cursor_hint->next_node_p;
      ptbl_p->cursor_hint = cursor_hint->next_node_p;
      ptbl_p->local_cursor_pos = 0;
    } else {
      cursor_hint->pre_node_p->next_node_p = cursor_hint->next_node_p;
      ptbl_p->cursor_hint = cursor_hint->pre_node_p;
      ptbl_p->local_cursor_pos = ptbl_p->cursor_hint->p.len;
    }
    free(cursor_hint);
    return;
  } else if (ptbl_p->local_cursor_pos == cursor_hint->p.len) {
    cursor_hint->p.len--;
    ptbl_p->local_cursor_pos--;
    return;
  } else if (ptbl_p->local_cursor_pos == 1) {
    cursor_hint->p.len--;
    cursor_hint->p.start++;
    ptbl_p->local_cursor_pos--;
    return;
  }

  // split block into two
  pl_node *split_node = (pl_node*)malloc(sizeof(pl_node));
  if (split_node == NULL) {
    // TODO: handle error
  }
  *split_node = (pl_node) {
    .pre_node_p = NULL,
    .next_node_p = NULL,
    .p = (piece) {
      .buf_type = cursor_hint->p.buf_type,
      .start = cursor_hint->p.start + ptbl_p->local_cursor_pos,
      .len = cursor_hint->p.len - ptbl_p->local_cursor_pos,
    },
  };

  cursor_hint->p.len -= split_node->p.len + 1;
  ptbl_p->local_cursor_pos = cursor_hint->p.len;
  pl_place_between(cursor_hint, split_node, cursor_hint->next_node_p);
}

void ptbl_update_global_cursor_pos(piece_table *ptbl_p, size_t new_global_cursor_pos) {
  // TODO: notion of piece table length
  // assert(new_global_cursor_pos < TOTAL_PTBL_LEN);
  ptbl_p->global_cursor_pos = new_global_cursor_pos;
  size_t running_len = 0;
  pl_node *head = ptbl_p->piece_list_head_p;
  while (head != NULL && running_len + head->p.len < new_global_cursor_pos) {
    running_len += head->p.len;
    head = head->next_node_p;
  }
  
  if (head == NULL) {
    // TODO: panic
    return;
  }

  ptbl_p->local_cursor_pos = new_global_cursor_pos - running_len;
  ptbl_p->cursor_hint = head;
}

void ptbl_display(piece_table *ptbl_p) {
  pl_node *head = ptbl_p->piece_list_head_p;
  while (head != NULL) {
    piece p = head->p;
    switch (p.buf_type) {
      case ORIGINAL:
        printf(BLUE "%.*s" RESET, (int) p.len, ptbl_p->orig_buf + p.start);
        break;
      case ADD:
        printf(RED "%.*s" RESET, (int) p.len, ptbl_p->add_buffer.buf + p.start);
        break;
    }
    printf("|");
    head = head->next_node_p;
  }
}

