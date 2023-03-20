#pragma once

#include "jmacro.h"
#include "jmem.h"
#include "jtype.h"

#define rbtree(TYPE) rbtree
#define rbtree_init(TYPE) rbtree_init_##TYPE
#define rbtree_add(TYPE) rbtree_add_##TYPE
void rbtree_del(rbtree* tree, u32 node);
u32 rbtree_next(rbtree* tree, u32 node);
u32 rbtree_prev(rbtree* tree, u32 node);
#define rbtree_find(TYPE) rbtree_find_##TYPE
#define rbtree_best(TYPE) rbtree_best_##TYPE
#define rbtree_at(TYPE) rbtree_at_##TYPE

tnode* rbtree_node(rbtree* tree, u32 node);
u32 rbtree_color(rbtree* tree, u32 node); // assume null/leaf node is red
void rbtree_rotate(rbtree* tree, u32 node, u32 direction);
void rbtree_destroy(rbtree* tree);

typedef struct tnode tnode;
typedef struct rbtree rbtree;

enum {
	RBTREE_RED = 0 << 31,
	RBTREE_BLACK = 1 << 31,

	RBTREE_LEFT = 0,
	RBTREE_RIGHT = 1,

	RBTREE_ROOT = 0,
	RBTREE_COLOR_MASK = 1 << 31
};

struct tnode {
	u32 l;
	u32 r;
	u32 p;
	u32 data; // this stores the color bit
};

struct rbtree {
	mem_list nodes;
	mem_list data;
};

#define RBTREE_DECLARE_INIT(TYPE) \
void rbtree_init(TYPE)(rbtree* tree, TYPE* val)

#define RBTREE_DECLARE_ADD(TYPE) \
void rbtree_add(TYPE)(rbtree* tree, TYPE* val)

#define RBTREE_DECLARE_FIND(TYPE) \
u32 rbtree_find(TYPE)(rbtree* tree, TYPE* VAL)

#define RBTREE_DECLARE_BEST(TYPE) \
u32 rbtree_best(TYPE)(rbtree* tree, TYPE* VAL)

#define RBTREE_DECLARE_AT(TYPE) \
TYPE* rbtree_at(TYPE)(rbtree* tree, u32 node)

#define RBTREE_DECLARE_FN(TYPE, ...) \
DECLARE_FN(RBTREE_DECLARE, TYPE, __VA_ARGS__)

#define RBTREE_DEFINE_INIT(TYPE) \
void rbtree_init(TYPE)(rbtree* tree, TYPE* val) { \
	list_init(&tree->nodes, sizeof(tnode), 0); \
	list_init(&tree->data, sizeof(TYPE), 0); \
	u32 handle = list_halloc(&tree->nodes); \
	assert(handle == RBTREE_ROOT); \
	tnode* root = rbtree_node(tree, handle); \
	handle = list_halloc(&tree->data); \
	TYPE* rval = rbtree_at(TYPE)(tree, handle); \
	copy(TYPE)(rval, val); \
	root->l = U32_MAX; \
	root->r = U32_MAX; \
	root->p = U32_MAX; \
	root->data = handle | RBTREE_BLACK; \
}

#define RBTREE_DEFINE_ADD(TYPE) \
void rbtree_add(TYPE)(rbtree* tree, TYPE* val) { \
	tnode* p = rbtree_node(tree, RBTREE_ROOT); \
	TYPE* v = rbtree_at(tree, RBTREE_ROOT); \
	u32 c = lt(val, v) ? p->l : p->r; \
	u32 ph; \
	while (c != U32_MAX) { \
		p = rbtree_node(tree, c); \
		v = rbtree_at(tree, c); \
		ph = c; \
	    c = lt(val, v) ? p->l : p->r; \
	} \
	u32 handle = list_halloc(&tree->nodes); \
	tnode* node = rbtree_node(tree, handle); \
	u32* child = lt(val, v) ? &p->l : &p->r; \
	*child = handle; \
	u32 tmp = list_halloc(&tree->data); \
	v = rbtree_at(tree, tmp); \
	copy(TYPE)(v, val); \
	node->p = ph; \
	node->r = U32_MAX; \
	node->l = U32_MAX; \
	node->data = tmp; \
	while (true) { \
		if (rbtree_color(tree, ph) == RBTREE_BLACK) return; \
		tnode* g = rbtree_node(tree, p->p); \
		u32 uh = ph == g->l ? g->r : g->l; \
		tnode* u = rbtree_node(tree, uh); \
		if (rbtree_color(tree, uh) == RBTREE_RED) { \
			p->data = p->data | RBTREE_BLACK; \
			u->data = p->data | RBTREE_BLACK; \
			g->data = g->data ^ RBTREE_BLACK; \
			handle = p->p; \
			ph = g->p; \
			p = rbtree_node(tree, ph); \
		} else { \
			u32 lrotate = ph == g->l && handle == p->r; \
			u32 rrotate = ph == g->r && handle == p->l; \
			if (lrotate || rrotate) { \
				u32 tmp = ph; \
				u32 dir = lrotate ? RBTREE_LEFT : RBTREE_RIGHT; \
				ph = lrotate ? p->r : p->l; \
				rbtree_rotate(tree, ph, dir); \
			} \
			u32 dir = ph == g->l ? RBTREE_RIGHT : RBTREE_LEFT; \
			rbtree_rotate(tree, ph, dir); \
			break; \
		} \
	} \
}

#define RBTREE_DEFINE_FIND(TYPE) \
u32 rbtree_find(TYPE)(rbtree* tree, TYPE* val) { \
	tnode* node = rbtree_node(tree, RBTREE_ROOT); \
	TYPE* data = rbtree_at(TYPE)(tree, node->data); \
	u32 next = lt(data, val) ? node->l : node->r; \
	u32 tmp = U32_MAX; \
	while (next != U32_MAX && !eq(data, val)) { \
		node = rbtree_node(tree, next); \
		data = rbtree_at(TYPE)(tree, node->data); \
		tmp = next; \
		next = lt(data, val) ? node->l : node->r; \
	} \
	return next == U32_MAX ? tmp : U32_MAX; \
} \

#define RBTREE_DEFINE_BEST(TYPE) \
u32 rbtree_best(TYPE)(rbtree* tree, TYPE* val) { \
	tnode* node = rbtree_node(tree, RBTREE_ROOT); \
	TYPE* data = rbtree_at(TYPE)(tree, node->data); \
	u32 next = lt(data, val) ? node->l : node->r; \
	u32 tmp = U32_MAX; \
	while (next != U32_MAX || !eq(data, val)) { \
		node = rbtree_node(tree, next); \
		data = rbtree_at(TYPE)(tree, node->data); \
		tmp = next; \
		next = lt(data, val) ? node->l : node->r; \
	} \
	return tmp; \
} \

#define RBTREE_DEFINE_AT(TYPE) \
TYPE* rbtree_at(TYPE)(rbtree* tree, u32 node) { \
	tnode* n = rbtree_node(tree, node); \
	return (TYPE*)list_at(&tree->data, node->data); \
}

#define RBTREE_DEFINE_FN(TYPE, ...) \
DECLARE_FN(RBTREE_DEFINE, TYPE, __VA_ARGS__)
