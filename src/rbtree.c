#include "rbtree.h"

struct rb_node{
	struct rb_node *parent, *left, *right;
	uint16_t key;
	uint16_t color;
};

struct rb_tree {
	struct rb_node *root;
};

static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node);
static void rb_delete_fixup(struct rb_tree *tree, struct rb_node *child, struct rb_node *parent);
static int rb_verify_node(struct rb_node *node, int *black_height);

static void rb_left_rotate(struct rb_tree *tree, struct rb_node *node);
static void rb_right_rotate(struct rb_tree *tree, struct rb_node *node);
static void rb_transplant(struct rb_tree *tree, struct rb_node *old, struct rb_node *new);
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

void rb_delete(struct rb_tree *tree, struct rb_node *to_delete)
{
	struct rb_node *successor, *parent, *child;
	successor = parent = child = 0;

	int original_color = to_delete->color;

	if (!to_delete->left) {						/* when 1 or fewer children, replace by child */
		parent = to_delete->parent;
		child = to_delete->right;
		rb_transplant(tree, to_delete, child);
	}

	else if (!to_delete->right) {
		parent = to_delete->parent;
		child = to_delete->left;
		rb_transplant(tree, to_delete, child);
	}

	else {								/* when 2 children, replace by successor */
		successor = to_delete->right;

		while(successor->left)
			successor = successor->left;

		child = successor->right;
		original_color = successor->color;

		if (successor == to_delete->right)
			parent = successor;

		else {							/* when successor deeper in tree, */
			parent = successor->parent;			/* replace by its right child */
			rb_transplant(tree, successor, child);

			successor->right = to_delete->right;
			successor->right->parent = successor;
		}

		rb_transplant(tree, to_delete, successor);		/* replace to_delete by successor */

		successor->left = to_delete->left;
		successor->left->parent = successor;

		successor->color = to_delete->color;
	}

	to_delete->parent = to_delete->left = to_delete->right = 0;

	if (original_color == BLACK)
		rb_delete_fixup(tree, child, parent);
}

static void rb_delete_fixup(struct rb_tree *tree, struct rb_node *child, struct rb_node *parent)
{
	while (parent && !rb_red(child)) {
		if (child == parent->left) {						/* left child */
			struct rb_node *sibling = parent->right;

			if (rb_red(sibling)) {						/* case 1: red sibling */
				sibling->color = BLACK;
				parent->color = RED;
				rb_left_rotate(tree, parent);

				sibling = parent->right;
			}

			if (!rb_red(sibling->left) && !rb_red(sibling->right)) {	/* case 2: black sibling */
				sibling->color = RED;					/* with 2 black children */
				child = parent;
				parent = child->parent;
			}

			else {
				if (!rb_red(sibling->right)) {				/* case 3: black sibling */
					sibling->left->color = BLACK;			/* with red left child */
					sibling->color = RED;
					rb_right_rotate(tree, sibling);

					sibling = parent->right;
				}

				sibling->color = parent->color;				/* case 4: black sibling */
				parent->color = BLACK;					/* with red right child */
				sibling->right->color = BLACK;
				rb_left_rotate(tree, parent);

				child = tree->root;
				parent = child->parent;
			}
		}

		else {									/* symmetrical when right child */
			struct rb_node *sibling = parent->left;

			if (rb_red(sibling)) {
				sibling->color = BLACK;
				parent->color = RED;
				rb_right_rotate(tree, parent);

				sibling = parent->left;
			}

			if (!rb_red(sibling->left) && !rb_red(sibling->right)) {
				sibling->color = RED;
				child = parent;
				parent = child->parent;
			}

			else {
				if (!rb_red(sibling->left)) {
					sibling->right->color = BLACK;
					sibling->color = RED;
					rb_left_rotate(tree, sibling);

					sibling = parent->left;
				}

				sibling->color = parent->color;
				parent->color = BLACK;
				sibling->left->color = BLACK;
				rb_right_rotate(tree, parent);

				child = tree->root;
				parent = child->parent;
			}
		}
	}

	if (child)
		child->color = BLACK;
}

int rb_verify(struct rb_tree *tree)
{
	if (!tree)
		return -1;

	if (rb_red(tree->root))						/* tree root must be black */
		return -1;

	int black_height = 0;
	return rb_verify_node(tree->root, &black_height);
}

static int rb_verify_node(struct rb_node *node, int *black_height)
{
	if (!node) {							/* all leaves are black */
		*black_height = 1;
		return 0;
	}

	if (node->color != RED && node->color != BLACK)			/* every node is either red or black */
		return -1;

	if (node->color == RED) {					/* red nodes must not have red children */
		if (rb_red(node->left) || rb_red(node->right))
			return -1;
	}

	int left_black_height = 0, right_black_height = 0;

	if (rb_verify_node(node->left, &left_black_height) < 0)
		return -1;

	if (rb_verify_node(node->right, &right_black_height) < 0)
		return -1;

	if (left_black_height != right_black_height)			/* all simple paths from a node to descendant leaves */
		return -1;						/* contain the same number of black nodes */

	*black_height = left_black_height + (node->color == BLACK);
	return 0;
}

static void rb_left_rotate(struct rb_tree *tree, struct rb_node *node)
{
	if (!node)
		return;

	struct rb_node *child = node->right;

	if (!child)
		return;

	node->right = child->left;

	if (child->left)
		child->left->parent = node;

	child->parent = node->parent;

	if (!node->parent)
		tree->root = child;

	else if (node == node->parent->left)
		node->parent->left = child;

	else
		node->parent->right = child;

	child->left = node;
	node->parent = child;
}

static void rb_right_rotate(struct rb_tree *tree, struct rb_node *node)
{
	if (!node)
		return;

	struct rb_node *child = node->left;

	if (!child)
		return;

	node->left = child->right;

	if (child->right)
		child->right->parent = node;

	child->parent = node->parent;

	if (!node->parent)
		tree->root = child;

	else if (node == node->parent->left)
		node->parent->left = child;

	else
		node->parent->right = child;

	child->right = node;
	node->parent = child;
}

static void rb_transplant(struct rb_tree *tree, struct rb_node *old, struct rb_node *new)
{
	if (!old)
		return;

	if (!old->parent)
		tree->root = new;

	else if (old == old->parent->left)
		old->parent->left = new;

	else
		old->parent->right = new;

	if (new)
		new->parent = old->parent;
}

static int rb_red(struct rb_node *node)
{
	return node && node->color == RED;
}
