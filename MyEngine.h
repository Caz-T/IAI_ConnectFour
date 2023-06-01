//
// Created by Chen Qingzhi on 2023/5/30.
//

#ifndef STRATEGY_MYENGINE_H
#define STRATEGY_MYENGINE_H

#include <ctime>
#include "Point.h"
#include "Judge.h"

class node {
public:
    node(int x, int y, int w, int h, node* p);
    ~node();

    inline bool is_terminal() const;
    inline bool is_fully_expanded() const;

    // Step info
    int curr_x, curr_y;

    // Core properties (Computed at initialisation)
    bool is_mach;
    bool is_term;

    // Board info
    int width, height;

    // Structure info
    node* parent;
    node** children;
    int curr_children_cnt = 0; // for programming convenience

    // Statistics
    int visit_count = 0;
    double q = 0.0;

    void clean(node* to_save = nullptr);
};

class MyEngine {
public:
    MyEngine(const int m, const int n, const int bx, const int by, double c);
    ~MyEngine();
    Point* search(const int last_x, const int last_y, time_t ponder_limit);
    inline bool column_is_full(const int n);

private:
    node* expand(node*);
    node* best_child(node*);
    node* tree_policy(node*);
    double default_policy(node*);
    void propagate_backwards(node*, double delta);

    void step_into(node*);
    void step_from(node*);

    int width, height;
    int ban_x, ban_y;
    node* memory;  // Root of the current decision tree
    int** board;
    int* top;
    // Current board status.
    // Whenever the engine visits a new node, board is changed.
    // Remember to reset after each operation!
    int** buffer;
    int* buffer_top;

    double uct_const;
};

#endif //STRATEGY_MYENGINE_H
