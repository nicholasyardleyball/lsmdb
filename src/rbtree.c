#include "rbtree.h"

#define RED 0
#define BLACK 1

struct rb_node{
	struct rb_node *parent, *left, *right;
	uint16_t key, value;
	uint16_t color;
};

struct rb_tree {
	struct rb_node *root;
};

/* tree lifecycle */
static void rb_free_node(struct rb_node *node);

/* modification and lookup */
static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node);
static void rb_delete_fixup(struct rb_tree *tree, struct rb_node *child, struct rb_node *parent);

/* debugging */
static int rb_verify_node(struct rb_node *node, int *black_height);
static void rb_print(struct rb_node *node, unsigned depth);

/* utilities */
static void rb_left_rotate(struct rb_tree *tree, struct rb_node *node);
static void rb_right_rotate(struct rb_tree *tree, struct rb_node *node);
static void rb_transplant(struct rb_tree *tree, struct rb_node *old, struct rb_node *new);
static int rb_red(struct rb_node *node);

struct rb_tree *rb_create(void)
{
	struct rb_tree *tree = malloc(sizeof(*tree));

	if (!tree)
		return 0;

	tree->root = 0;
	return tree;
}

void rb_clear(struct rb_tree *tree)
{
	if (!tree)
		return;

	rb_free_node(tree->root);
	tree->root = 0;
}

void rb_destroy(struct rb_tree *tree)
{
	if (!tree)
		return;

	rb_free_node(tree->root);
	free(tree);
}

static void rb_free_node(struct rb_node *node)
{
	if (!node)
		return;

	rb_free_node(node->left);
	rb_free_node(node->right);
	free(node);
}

int rb_insert(struct rb_tree *tree, uint16_t key, uint16_t value)
{
	struct rb_node *node = tree->root;
	struct rb_node *parent = 0;

	while (node) {
		parent = node;

		if (node->key == key) {
			node->value = value;
			return 0;
		}

		node = (key < node->key) ? node->left : node->right;
	}

	struct rb_node *to_insert = malloc(sizeof(*to_insert));

	if (!to_insert)
		return -1;

	to_insert->key = key;
	to_insert->value = value;
	to_insert->parent = parent;
	to_insert->left = to_insert->right = 0;
	to_insert->color = RED;

	if (!parent)
		tree->root = to_insert;

	else if (key < parent->key)
		parent->left = to_insert;

	else
		parent->right = to_insert;

	rb_insert_fixup(tree, to_insert);
	return 0;
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

int rb_delete(struct rb_tree *tree, uint16_t key)
{
	struct rb_node *to_delete = 0, *successor = 0, *parent = 0, *child = 0;

	to_delete = rb_find(tree, key);

	if (!to_delete)
		return -1;

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
	free(to_delete);

	if (original_color == BLACK)
		rb_delete_fixup(tree, child, parent);

	return 0;
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

struct rb_node *rb_find(struct rb_tree *tree, uint16_t key)
{
	struct rb_node *node = tree->root;

	while (node) {
		if (node->key == key)
			return node;

		node = (key < node->key) ? node->left : node->right;
	}

	return 0;
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

void rb_dump(struct rb_tree *tree)
{
	rb_print(tree->root, 0);
}

static void rb_print(struct rb_node *node, unsigned depth)
{
	if (!node)
		return;

	rb_print(node->right, depth + 1);

	for (size_t i = 0; i < depth; i++)
		printf("        ");

	printf("[%c]%05u:%05u\n", (node->color == RED) ? 'R' : 'B', node->key, node->value);

	rb_print(node->left, depth + 1);
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
