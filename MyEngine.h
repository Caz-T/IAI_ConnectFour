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
    ~node() = default;
    // Manage memory with clean()!

    inline bool is_terminal() const;
    inline bool is_fully_expanded() const;

    // Step info
    int curr_x, curr_y;

    // Core properties (Computed at initialisation)
    bool is_mach;
    bool is_term = false;

    // Board info
    int width, height;

    // Structure info
    node* parent;
    node** children;
    int curr_children_cnt = 0; // for programming convenience

    // Statistics
    int visit_count = 0;
    double q = 0.0;

    void clean(node* to_save = nullptr) const;
};

class MyEngine {
public:
    MyEngine(int m, int n, int bx, int by, double c);
    ~MyEngine();

    Point* search(int last_x, int last_y, time_t ponder_limit);
    inline bool column_is_full(int n, bool is_dry_run = false);

private:
    void step_into(node*);
    void step_into_faked(int, int, bool);
    void step_from(node*);
    void step_from_faked(int, int);

    node* expand(node*);
    node* best_child(node*) const;
    node* tree_policy(node*);
    double default_policy(node*);
    void propagate_backwards(node*, double delta);

    int ikanakerebanaranai(int layer_cnt);
    bool leads_to_victory(int x, int y, int allowed_recursions);
    // A move "leads to victory", i.f.f.
    // i)   the move immediately wins the game; or
    // ii)  recursion is allowed, and for each move the opponent could make, there is a move that leads to victory.

    int width, height;
    int ban_x, ban_y;
    node* memory = nullptr;  // Root of the current decision tree
    int** board;
    int* top;
    // Current board status.
    // Whenever the engine visits a new node, board is changed.
    // Remember to reset after each operation!
    int** buffer;
    int* buffer_top;

    int expansion_cnt = 1;  // records expansions
    int expansion_limit = 0x7fffffff;

    double uct_const;

    // for debugging only
    void print_board() const;
};

#endif //STRATEGY_MYENGINE_H
