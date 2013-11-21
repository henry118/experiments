#ifndef __RBTREE_H__
#define __RBTREE_H__

#include <iostream>
#include <string.h>
#include <cassert>

template<class T>
class rbtree {
    struct node {
        enum color_t { red, black };
        color_t color;
        T key;
        node *left, *right, *parent;
    } *root, *null;
    size_t nodenum;
public:
    rbtree() : nodenum(0) {
        null = new node;
        bzero(null, sizeof(node));
        null->color = node::black;
        root = null;
    }
    ~rbtree() {
        clear();
        delete null;
    }
    void insert(T key) {
        node *x = root, *y = null, *z = newnode(key);
        while (x != null) {
            y = x;
            if (z->key < x->key)
                x = x->left;
            else
                x = x->right;
        }
        z->parent = y;
        if (y == null)
            root = z;
        else if (z->key < y->key)
            y->left = z;
        else
            y->right = z;

        insert_fixup(z);
        ++nodenum;
    }
    void remove(T key) {
        node * z = find(key);
        if (!z || z == null)
            return;
        remove(z);
    }
    bool contains(T key) const {
        return find(key) != null;
    }
    bool empty() const {
        return root == null;
    }
    size_t size() const {
        return nodenum;
    }
    void clear() {
        while (root != null)
            remove(root);
    }
    void pretty_print() const {
        pretty_print(root);
    }
private:
    node * newnode(T newval) {
        node * n = new node;
        n->color = node::red;
        n->key = newval;
        n->left = null;
        n->right = null;
        n->parent = null;
        return n;
    }
    int depth(node * x) const {
        int v = 0;
        while (x != root) {
            x = x->parent;
            ++v;
        }
        return v;
    }
    node * find(T key) const {
        node * x = root;
        while (x != null) {
            if (x->key == key) {
                return x;
            } else if (x->key > key)
                x = x->left;
            else
                x = x->right;
        }
        return null;
    }
    void pretty_print(node * x) const {
        const static char * RED = "\33[22;31m";
        const static char * NC = "\33[0m";

        if (!x || x == null)
            return;

        int dps = depth(x);
        if (dps > 0) {
            std::cout << '|';
            for (int i = 0; i < 4 * dps; i++)
                std::cout << '-';
        }
        if (x->color == node::red) std::cout << RED;
        std::cout << x->key;
        if (x->color == node::red) std::cout << NC;
        std::cout << std::endl;
        pretty_print(x->left);
        pretty_print(x->right);
    }
    node * minimum(node * x) const {
        assert(x != null);
        node * y = x;
        while (y->left != null)
            y = y->left;
        return y;
    }
    node * maximum(node * x) const {
        assert(x != null);
        node * y = x;
        while (y->right = null)
            y = y->right;
        return y;
    }
    void left_rotate(node * x) {
        node * y = x->right;
        x->right = y->left;
        if (y->left != null)
            y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == null)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;
    }
    void right_rotate(node * x) {
        node * y = x->left;
        x->left = y->right;
        if (y->right != null)
            y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == null)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->right = x;
        x->parent = y;
    }
    void transplant(node * u, node * v) {
        if (u->parent == null)
            root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;
        v->parent = u->parent;
    }
    void remove(node * z) {
        node *y = z, *x = null;
        typename node::color_t y_orig_color = y->color;
        if (z->left == null) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == null) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_orig_color = y->color;
            x = y->right;
            if (y->parent == z)
                x->parent = y;
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        if (y_orig_color == node::black)
            remove_fixup(x);
        delete z;
        --nodenum;
    }
    void insert_fixup(node * z) {
        node *y = 0;
        while (z->parent->color == node::red) {
            if (z->parent == z->parent->parent->left) {
                y = z->parent->parent->right;
                if (y->color == node::red) {
                    z->parent->color = node::black;
                    y->color = node::black;
                    z->parent->parent->color = node::red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = node::black;
                    z->parent->parent->color = node::red;
                    right_rotate(z->parent->parent);
                }
            } else {
                y = z->parent->parent->left;
                if (y->color == node::red) {
                    z->parent->color = node::black;
                    y->color = node::black;
                    z->parent->parent->color = node::red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = node::black;
                    z->parent->parent->color = node::red;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->color = node::black;
    }
    void remove_fixup(node * x) {
        node * w = null;
        while (x != root && x->color == node::black) {
            if (x == x->parent->left) {
                w = x->parent->right;
                if (w->color == node::red) {
                    w->color = node::black;
                    x->parent->color = node::red;
                    left_rotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == node::black && w->right->color == node::black) {
                    w->color = node::red;
                    x = x->parent;
                } else {
                    if (w->right->color == node::black) {
                        w->left->color = node::black;
                        w->color = node::red;
                        right_rotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = node::black;
                    w->right->color = node::black;
                    left_rotate(x->parent);
                    x = root;
                }
            } else {
                w = x->parent->left;
                if (w->color == node::red) {
                    w->color = node::black;
                    x->parent->color = node::red;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }
                if (w->left->color == node::black && w->right->color == node::black) {
                    w->color = node::red;
                    x = x->parent;
                } else {
                    if (w->left->color == node::black) {
                        w->right->color = node::black;
                        w->color = node::red;
                        left_rotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = node::black;
                    w->left->color = node::black;
                    right_rotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = node::black;
    }
};

#endif //__RBTREE_H__
