/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS), Shanghai Jiao Tong University (SJTU)
 * OS-Lab-2020 (i.e., ChCore) is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 *   PURPOSE.
 *   See the Mulan PSL v1 for more details.
 */

#ifdef CHCORE
#include <common/kmalloc.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <common/radix.h>
#endif
#include <common/errno.h>

/* ceil(a/b) */
#define DIV_UP(a, b) (((a)+(b)-1)/(b))

#define RADIX_LEVELS (DIV_UP(RADIX_MAX_BITS, RADIX_NODE_BITS))

struct radix *new_radix(void)
{
	struct radix *radix;

	radix = kzalloc(sizeof(*radix));
	BUG_ON(!radix);

	return radix;
}

void init_radix(struct radix *radix)
{
	radix->root = kzalloc(sizeof(*radix->root));
	BUG_ON(!radix->root);
	radix->value_deleter = NULL;
}

void init_radix_w_deleter(struct radix *radix, void (*value_deleter) (void *))
{
	init_radix(radix);
	radix->value_deleter = value_deleter;
}

static struct radix_node *new_radix_node(void)
{
	struct radix_node *n = kzalloc(sizeof(struct radix_node));

	// kdebug("radix_new_node: %p\n", n);
	if (!n) {
		return ERR_PTR(-ENOMEM);
	}

	return n;
}

int radix_add(struct radix *radix, u64 key, void *value)
{
	struct radix_node *node;
	struct radix_node *new;
	u16 index[RADIX_LEVELS];
	int i;
	int k;

	// kdebug("==============> radix= %p\n", radix);
	if (!radix->root) {
		new = new_radix_node();
		if (IS_ERR(new))
			return -ENOMEM;
		radix->root = new;
	}
	node = radix->root;
	// kdebug("radix->root  = %p\n", node);

	/* calculate index for each level */
	for (i = 0; i < RADIX_LEVELS; ++i) {
		index[i] = key & RADIX_NODE_MASK;
		key >>= RADIX_NODE_BITS;
	}

	/* the intermediate levels */
	for (i = RADIX_LEVELS - 1; i > 0; --i) {
		k = index[i];
		if (!node->children[k]) {
			new = new_radix_node();
			if (IS_ERR(new))
				return -ENOMEM;
			node->children[k] = new;
		}
		node = node->children[k];
	}

	/* the leaf level */
	k = index[0];
	node->values[k] = value;

	return 0;
}

void *radix_get(struct radix *radix, u64 key)
{
	struct radix_node *node;
	u16 index[RADIX_LEVELS];
	int i;
	int k;

	if (!radix->root)
		return NULL;
	node = radix->root;

	/* calculate index for each level */
	for (i = 0; i < RADIX_LEVELS; ++i) {
		index[i] = key & RADIX_NODE_MASK;
		key >>= RADIX_NODE_BITS;
	}

	/* the intermediate levels */
	for (i = RADIX_LEVELS - 1; i > 0; --i) {
		k = index[i];
		if (!node->children[k])
			return NULL;
		node = node->children[k];
	}

	/* the leaf level */
	k = index[0];
	return node->values[k];
}

int radix_del(struct radix *radix, u64 key)
{
	return radix_add(radix, key, NULL);
}

static void radix_free_node(struct radix_node *node, int node_level,
			    void (*value_deleter) (void *))
{
	int i;

	WARN_ON(!node, "should not try to free a node pointed by NULL");
	if (node_level == RADIX_LEVELS - 1) {
		if (value_deleter) {
			for (i = 0; i < RADIX_NODE_SIZE; i++) {
				if (node->values[i])
					value_deleter(node->values[i]);
			}
		}
	} else {
		for (i = 0; i < RADIX_NODE_SIZE; i++) {
			if (node->children[i])
				radix_free_node(node->children[i],
						node_level + 1, value_deleter);
		}
	}
}

int radix_free(struct radix *radix)
{
	if (!radix || !radix->root) {
		WARN("trying to free an empty radix tree");
		return -EINVAL;
	}
	// recurssively free nodes and values (if value_deleter is not NULL)
	radix_free_node(radix->root, 0, radix->value_deleter);

	return 0;
}
