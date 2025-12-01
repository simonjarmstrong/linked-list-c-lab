#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"

/* Allocate and initialize a new empty list. Returns NULL on allocation failure. */
list_t *list_alloc(void) {
  list_t *new_list = malloc(sizeof(list_t));
  if (new_list == NULL) {
    fprintf(stderr, "list_alloc: malloc failed\n");
    return NULL;
  }
  new_list->head = NULL;
  return new_list;
}

/* Free all nodes and then free the list structure itself.
   After calling this function the pointer passed becomes invalid. */
void list_free(list_t *list) {
  if (list == NULL) {
    return;
  }

  node_t *current = list->head;
  node_t *next_node = NULL;

  while (current != NULL) {
    next_node = current->next;
    free(current);
    current = next_node;
  }

  free(list);
}

/* Print the list elements in the form "v1 -> v2 -> ... -> NULL".
   If list is NULL, prints "LIST IS NULL". If empty prints "LIST IS EMPTY". */
void list_print(list_t *list) {
  if (list == NULL) {
    printf("LIST IS NULL\n");
    return;
  }

  node_t *cursor = list->head;
  if (cursor == NULL) {
    printf("LIST IS EMPTY\n");
    return;
  }

  while (cursor != NULL) {
    printf("%d ->", cursor->value);
    cursor = cursor->next;
  }
  printf("NULL\n");
}

/* Convert the list to a string. Caller is responsible for freeing returned buffer.
   The function dynamically grows the buffer as needed to avoid overflows.
   Returns NULL if allocation fails or if list is NULL. */
char *listToString(list_t *list) {
  if (list == NULL) {
    return NULL;
  }

  size_t capacity = 256;
  size_t used = 0;
  char *buf = malloc(capacity);
  if (buf == NULL) {
    return NULL;
  }
  buf[0] = '\0';

  node_t *cur = list->head;
  while (cur != NULL) {
    /* Estimate max characters this node might need (including arrow and sign). */
    char tmp[32];
    int n = snprintf(tmp, sizeof(tmp), "%d->", cur->value);
    if (n < 0) {
      free(buf);
      return NULL;
    }
    /* Grow buffer if needed */
    if (used + (size_t)n + 1 >= capacity) {
      size_t newcap = capacity * 2;
      while (used + (size_t)n + 1 >= newcap) newcap *= 2;
      char *newbuf = realloc(buf, newcap);
      if (newbuf == NULL) {
        free(buf);
        return NULL;
      }
      buf = newbuf;
      capacity = newcap;
    }
    memcpy(buf + used, tmp, (size_t)n);
    used += (size_t)n;
    buf[used] = '\0';
    cur = cur->next;
  }

  /* Append final "NULL" */
  const char *tail = "NULL";
  size_t tail_len = strlen(tail);
  if (used + tail_len + 1 >= capacity) {
    char *newbuf = realloc(buf, used + tail_len + 1);
    if (newbuf == NULL) {
      free(buf);
      return NULL;
    }
    buf = newbuf;
  }
  memcpy(buf + used, tail, tail_len);
  used += tail_len;
  buf[used] = '\0';

  return buf;
}

/* Return number of nodes in list. If list is NULL, returns 0. */
int list_length(list_t *list) {
  if (list == NULL) return 0;
  node_t *cursor = list->head;
  int count = 0;
  while (cursor != NULL) {
    count++;
    cursor = cursor->next;
  }
  return count;
}

/* Helper: allocate a new node and initialize it. Exits on allocation failure. */
static node_t *create_node(elem value) {
  node_t *n = malloc(sizeof(node_t));
  if (n == NULL) {
    fprintf(stderr, "create_node: malloc failed\n");
    exit(EXIT_FAILURE);
  }
  n->value = value;
  n->next = NULL;
  return n;
}

/* Add value to end of list. If list is NULL function does nothing. */
void list_add_to_back(list_t *list, elem value) {
  if (list == NULL) return;

  node_t *new_node = create_node(value);

  if (list->head == NULL) {
    list->head = new_node;
    return;
  }

  node_t *cursor = list->head;
  while (cursor->next != NULL) {
    cursor = cursor->next;
  }
  cursor->next = new_node;
}

/* Add value to front of list. If list is NULL function does nothing. */
void list_add_to_front(list_t *list, elem value) {
  if (list == NULL) return;

  node_t *new_node = create_node(value);
  new_node->next = list->head;
  list->head = new_node;
}

/* Insert value at 1-based index in the list.
   Valid indices: 1 .. length+1
   - index == 1 inserts at front
   - index == length+1 appends to back
   If index is out of range or list is NULL, function does nothing. */
void list_add_at_index(list_t *list, elem value, int index) {
  if (list == NULL) return;
  if (index < 1) return;

  /* Insert at front */
  if (index == 1) {
    list_add_to_front(list, value);
    return;
  }

  int pos = 1;
  node_t *cursor = list->head;
  /* Move to node at position index-1 (1-based). Stop if cursor becomes NULL. */
  while (cursor != NULL && pos < index - 1) {
    cursor = cursor->next;
    pos++;
  }

  /* If cursor is NULL, index was too large ( > length ). In order to allow insertion
     at length+1 (append), check if pos == length and cursor != NULL; otherwise do nothing. */
  if (cursor == NULL) {
    return;
  }

  /* If inserting at end (index == length+1) this will set new_node->next = NULL */
  node_t *new_node = create_node(value);
  new_node->next = cursor->next;
  cursor->next = new_node;
}

/* Remove last element and return its value. Returns -1 on error (NULL or empty list). */
elem list_remove_from_back(list_t *list) {
  if (list == NULL || list->head == NULL) return -1;

  /* Single element */
  if (list->head->next == NULL) {
    elem v = list->head->value;
    free(list->head);
    list->head = NULL;
    return v;
  }

  node_t *cursor = list->head;
  while (cursor->next->next != NULL) {
    cursor = cursor->next;
  }
  node_t *last = cursor->next;
  elem v = last->value;
  cursor->next = NULL;
  free(last);
  return v;
}

/* Remove first element and return its value. Returns -1 on error. */
elem list_remove_from_front(list_t *list) {
  if (list == NULL || list->head == NULL) return -1;

  node_t *to_remove = list->head;
  elem v = to_remove->value;
  list->head = to_remove->next;
  free(to_remove);
  return v;
}

/* Remove element at 1-based index and return its value.
   Valid indices: 1 .. length
   Returns -1 if index invalid or list empty/NULL. */
elem list_remove_at_index(list_t *list, int index) {
  if (list == NULL || list->head == NULL) return -1;
  if (index < 1) return -1;

  if (index == 1) {
    return list_remove_from_front(list);
  }

  int pos = 1;
  node_t *cursor = list->head;
  while (cursor != NULL && pos < index - 1) {
    cursor = cursor->next;
    pos++;
  }

  if (cursor == NULL || cursor->next == NULL) return -1;

  node_t *to_remove = cursor->next;
  elem v = to_remove->value;
  cursor->next = to_remove->next;
  free(to_remove);
  return v;
}

/* Check existence of value in list. Returns true/false only (no printing). */
bool list_is_in(list_t *list, elem value) {
  if (list == NULL) return false;

  node_t *cursor = list->head;
  while (cursor != NULL) {
    if (cursor->value == value) return true;
    cursor = cursor->next;
  }
  return false;
}

/* Return value at 1-based index. Returns -1 if index invalid or list is NULL. */
elem list_get_elem_at(list_t *list, int index) {
  if (list == NULL || index < 1) return -1;

  node_t *cursor = list->head;
  int pos = 1;
  while (cursor != NULL) {
    if (pos == index) return cursor->value;
    cursor = cursor->next;
    pos++;
  }
  return -1;
}

/* Return 1-based index of value in the list (first occurrence).
   Returns -1 if not found or if list is NULL. */
int list_get_index_of(list_t *list, elem value) {
  if (list == NULL) return -1;

  node_t *cursor = list->head;
  int pos = 1;
  while (cursor != NULL) {
    if (cursor->value == value) return pos;
    cursor = cursor->next;
    pos++;
  }
  return -1;
}
