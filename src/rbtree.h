#include <stdint.h>

struct rb_node;
struct rb_tree;

void rb_insert(struct rb_tree *tree, struct rb_node *to_insert);
void rb_delete(struct rb_tree *tree, struct rb_node *to_delete);
int rb_verify(struct rb_tree *tree);
