#include "jtree.h"

void rbtree_del(rbtree* tree, u32 node) {
	tnode* p = rbtree_node(tree, node);
	tnode* r = rbtree_node(tree, p->r);
	tnode* l = rbtree_node(tree, p->l);
	tnode* g = rbtree_node(tree, p->p);
	u32 color;

	if (p->r == U32_MAX) {
		l->data = (l->data & ~RBTREE_COLOR_MASK) | RBTREE_BLACK;
		l->p = p->p;
		node = p->l;
		color = RBTREE_RED;
	} elif (r->l == U32_MAX) {
		color = rbtree_color(tree, p->r);
		r->data = (r->data & ~RBTREE_COLOR_MASK) | rbtree_color(tree, node);
		r->l = p->l;
		l->p = p->r;
		r->p = p->p;
		node = p->r;
	} else {
		u32 sh = rbtree_next(tree, node);
		color = rbtree_color(tree, node);
		node = sh;
		tnode* s = rbtree_node(tree, sh);
		s->data = (s->data & ~RBTREE_COLOR_MASK) | color;
		color = s->r == U32_MAX ? rbtree_color(tree, sh) : rbtree_color(tree, s->r);

		if (s->r != U32_MAX) {
			tnode* r = rbtree_node(tree, s->r);
			r->data = (r->data & ~RBTREE_COLOR_MASK) | RBTREE_BLACK;
			r->p = s->p;
		}

		tnode* g = rbtree_node(tree, s->p);
		g->l = s->r;
		s->l = p->l;
		s->r = p->r;
	}

	u32 isleft = node == g->l;
	u32* change = isleft ? &g->l : &g->r;
	*change = node;
	list_hfree(&tree->data, p->data);
	list_hfree(&tree->nodes, node);

	if (color == RBTREE_RED) return;

	u32 ph = p->p;
	u32 xh = isleft ? p->l : p->r;
	u32 wh = isleft ? p->r : p->l;
	p = rbtree_node(tree, ph);
	tnode* x = rbtree_node(tree, xh);
	tnode* w = rbtree_node(tree, wh);

	while (true) {
		if (rbtree_color(tree, wh) == RBTREE_RED) {
			u32 dir = isleft ? RBTREE_LEFT : RBTREE_RIGHT;
			w->data = (data & ~RBTREE_COLOR_MASK) | rbtree_color(tree, ph);
			p->data = (data & ~RBTREE_COLOR_MASK) | RBTREE_RED;
			rbtree_rotate(tree, w->p, dir);
			wh = isleft ? p->r : p->l;
			w = rbtree_node(tree, wh);
		}

		if (rbtree_color(tree, w->l) | rbtree_color(tree, w->r) == RBTREE_RED) {
			w->data = (data & ~RBTREE_COLOR_MASK) | RBTREE_RED;
		} else {
			u32 nh = isleft ? w->r : w->l;
			if (nh == U32_MAX || rbtree_color(tree, nh) == RBTREE_BLACK) {
				u32 dir = isleft ? RBTREE_RIGHT : RBTREE_LEFT;
				rbtree_rotate(tree, wh, dir);
				nh = isleft ? w->r : w->l;
				wh = isleft ? p->r : p->l;
				w = rbtree_node(tree, wh);
			}

			tnode* n = rbtree_node(tree, nh);
			n->data = (n->data & ~RBTREE_COLOR_MASK) | RBTREE_BLACK;
			p->data = (p->data & ~RBTREE_COLOR_MASK) | RBTREE_BLACK;
			w->data = (w->data & ~RBTREE_COLOR_MASK) | rbtree_color(tree, ph);
			u32 dir = isleft ? RBTREE_LEFT : RBTREE_RIGHT;
			rbtree_rotate(tree, p, dir);

			ph = wh;
			p = w;
		}

		if (rbtree_color(tree, ph) == RBTREE_BLACK) return;
		p->data = (p->data & ~RBTREE_COLOR_MASK) | RBTREE_BLACK;

		// a +1 black imbalance occurs on the p subtree, so we rebalance the other subtree as if a black node was deleted
		tnode* g = rbtree_node(tree, p->p);
		isleft = ph == g->r;
		ph = p->p;
		p = g;

		xh = isleft ? p->l : p->r;
		x = rbtree_node(tree, xh);
		wh = isleft ? p->r : p->l;
		w = rbtree_node(tree, wh);
	}
}

u32 rbtree_next(rbtree* tree, u32 node) {
	tnode* n = rbtree_node(tree, node);
	node = n->r;
	u32 tmp = n->p;
	while (node != U32_MAX) {
		n = rbtree_node(tree, node);
		tmp = node;
		node = n->l;
	}

	return tmp;
}

u32 rbtree_prev(rbtree* tree, u32 node) {
	tnode* n = rbtree_node(tree, node);
	node = n->l;
	u32 tmp = n->p;
	while (node != U32_MAX) {
		n = rbtree_node(tree, node);
		tmp = node;
		node = n->r;
	}

	return tmp;
}

tnode* rbtree_node(rbtree* tree, u32 node) {
	return (tnode*)list_at(&tree->nodes, node);
}

u32 rbtree_color(rbtree* tree, u32 node) {
	tnode* n = rbtree_node(tree, node);
	return n->data & RBTREE_COLOR_MASK;
}

void rbtree_rotate(rbtree* tree, u32 node, u32 direction) {
	tnode* p = rbtree_node(tree, node);
	tnode* g = rbtree_node(tree, p->p);
	tnode* l = rbtree_node(tree, p->l);
	tnode* r = rbtree_node(tree, p->r);

	u32 lh = p->l;
	u32 rh = p->r;

	u32 bh = direction ? l->r : r->l;
	u32* change = direction ? &p->l : &p->r;
	*change = bh;

	tnode* x = direction ? l : r;
	tnode* b = bh != U32_MAX ? rbtree_node(tree, bh) : x;
	b->p = node;
	x->p = p->p;

	change = direction ? &x->r : &x->l;
	*change = node;

	p->p = direction ? lh : rh;

	u32 isleft = node == g->l;
	change = isleft ? &g->l : &g->r;
	*change = direction ? lh : rh;
}

void rbtree_destroy(rbtree* tree) {
	list_destroy(&tree->nodes);
	list_destroy(&tree->data);
	*tree = {};
}
