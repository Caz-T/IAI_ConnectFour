//
// Created by Chen Qingzhi on 2023/5/30.
//

#ifndef STRATEGY_MYENGINE_H
#define STRATEGY_MYENGINE_H

#include "Point.h"
#include "Judge.h"

const double UCT_CONST = 0.7;

class node {
public:
    node(int x, int y, int w, int h, node* p);

    int currx, curry; // current step
    int width, height;
    int visit_count = 0;
    int win_count = 0;
    double q = 0.0;
    node* parent;
    node** children;
    int* to_expand;
    int list_top = 0;

};

class MyEngine {
public:
    MyEngine(const int m, const int n, const int bx, const int by);
    Point* search(const int last_x, const int last_y);

private:
    node* expand(node*);
    node* best_child(node*);
    node* tree_policy(node*);
    double default_policy(node*);
    void propagate_backwards(node*, double delta);

    int width, height;
    int ban_x, ban_y;
    node* memory;  // saves the computations
    Point* to_debug;
};

#endif //STRATEGY_MYENGINE_H
