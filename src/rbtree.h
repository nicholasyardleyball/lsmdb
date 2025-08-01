#include <stdint.h>
#include <stdlib.h>

struct rb_node;
struct rb_tree;

/* tree lifecycle */
struct rb_tree *rb_create(void);
void rb_clear(struct rb_tree *tree);
void rb_destroy(struct rb_tree *tree);

/* modification and lookup */
int rb_insert(struct rb_tree *tree, uint16_t key, uint16_t value);
int rb_delete(struct rb_tree *tree, uint16_t key);
struct rb_node *rb_find(struct rb_tree *tree, uint16_t key);

/* debugging */
int rb_verify(struct rb_tree *tree);
