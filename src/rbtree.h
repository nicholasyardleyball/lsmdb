#include <stdint.h>
#include <stdlib.h>

struct rb_node;
struct rb_tree;

int rb_insert(struct rb_tree *tree, uint16_t key, uint16_t value);
int rb_delete(struct rb_tree *tree, uint16_t key);
struct rb_node *rb_find(struct rb_tree *tree, uint16_t key);
int rb_verify(struct rb_tree *tree);
