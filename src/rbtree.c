#include "rbtree.h"

static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node);
static int rb_red(struct rb_node *node);

void rb_insert(struct rb_tree *tree, struct rb_node *to_insert)
{
	struct rb_node *node = tree->root;
	struct rb_node *parent = 0;

	while (node) {
		parent = node;
		node = (to_insert->key < node->key) ? node->left : node->right;
	}

	to_insert->parent = parent;

	if (!parent)
		tree->root = to_insert;

	else if (to_insert->key < parent->key)
		parent->left = to_insert;

	else
		parent->right = to_insert;

	to_insert->left = to_insert->right = 0;
	to_insert->color = RED;

	rb_insert_fixup(tree, to_insert);
}

static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node)
{
	struct rb_node *parent = node->parent;

	while (rb_red(parent)) {
		struct rb_node *gparent = parent->parent;

		if (parent == gparent->left) {					/* parent is a left child */
			struct rb_node *aunt = gparent->right;

			if (rb_red(aunt)) {					/* case 1: parent and aunt both red */
				parent->color = aunt->color = BLACK;
				gparent->color = RED;

				node = gparent;
				parent = node->parent;
			}

			else {
				if (node == parent->right) {			/* case 2: black aunt, right child */
					node = parent;
					rb_left_rotate(tree, node);

					parent = node->parent;
				}

				parent->color = BLACK;				/* case 3: black aunt, left child */
				gparent->color = RED;
				rb_right_rotate(tree, gparent);
			}
		}

		else {								/* symmetrical when parent is a right child */
			struct rb_node *aunt = gparent->left;

			if (rb_red(aunt)) {
				parent->color = aunt->color = BLACK;
				gparent->color = RED;

				node = gparent;
				parent = node->parent;
			}

			else {
				if (node == parent->left) {
					node = parent;
					rb_right_rotate(tree, node);

					parent = node->parent;
				}

				parent->color = BLACK;
				gparent->color = RED;
				rb_left_rotate(tree, gparent);
			}
		}
	}

	tree->root->color = BLACK;
}

static int rb_red(struct rb_node *node)
{
	return node && node->color == RED;
}
