//
// Created by Chen Qingzhi on 2023/5/30.
//

#include <random>
#include <algorithm>
#include <ctime>
#include "MyEngine.h"
#include "Judge.h"

using namespace std;

node::node(int x, int y, int w, int h, node* p): curr_x(x), curr_y(y), parent(p), width(w), height(h) {
    children = new node*[w];
    is_mach = p and not p->is_mach;
}

node::~node() {
    // Manage memory with clean()!
}

inline bool node::is_terminal() const { return is_term; }
inline bool node::is_fully_expanded() const { return curr_children_cnt >= width; }

void node::clean(node *to_save) {
    for (int i = 0; i < curr_children_cnt; i++) {
        if (children[i] != to_save) {
            clean(children[i]);
            delete children[i];
        }
    }
}

MyEngine::MyEngine(const int m, const int n, const int bx, const int by, double c): height(m), width(n), ban_x(bx), ban_y(by),
board(new int*[m]), top(new int[n]), buffer(new int*[m]), buffer_top(new int[n]), uct_const(c) {
    for (int i = 0; i < m; i++) {
        board[i] = new int[n];
        buffer[i] = new int[n];
    }
    for (int i = 0; i < n; i++) top[i] = m - 1;
    if (ban_x == m - 1) top[ban_y] = m - 2;
}

MyEngine::~MyEngine() {
    for (int i = 0; i < height; i++) delete[] board[i];
    delete[] board;
    delete[] top;
}

bool MyEngine::column_is_full(const int n) {
    return (ban_x == 0 and ban_y == n) ? board[1][n] != 0 : board[0][n] != 0;
}

void MyEngine::step_into(node* v) {
    board[v->curr_x][v->curr_y] = v.is_mach ? 2 : 1;
    top[v->curr_y] -= 1;
    if (v->curr_y == ban_y and top[v->curr_y] == ban_x) top[v->curr_y] -= 1;
}
void MyEngine::step_from(node* v) {
    board[v->curr_x][v->curr_y] = 0;
    top[v->curr_y] = v->curr_x;
}

node* MyEngine::expand(node* to_expand) {
    for (int i = to_expand->curr_children_cnt; i < width; i++) {
        if (not column_is_full(i)) {
            auto expanded = new node(this->top[i], i, this->width, this->height, to_expand);
            to_expand->children[i] = expanded;
            to_expand->curr_children_cnt = i + 1;
            step_into(expanded);
            expanded.is_term = expanded->is_mach and machineWin(expanded->curr_x, expanded->curr_y, height, width, board) or not expanded->is_mach and userWin(expanded->curr_x, expanded->curr_y, height, width, board);
            return expanded;
        }
    }
    assert(false);
}
node* MyEngine::best_child(node* to_check) {
    double best_val = -1;
    node* to_ret = nullptr;
    for (int i = 0; i < to_check->curr_children_cnt; i++) {
        auto to_ex = to_check->children[i];
        double val = to_check->is_mach ? -to_ex->q : to_ex->q;
        // If to_check (the current node in question) is made by machine, then we aim to pick the best move for human move. Vice versa.
        val = val / to_ex->visit_count + uct_const * sqrt(2 * log(to_check->visit_count) / to_ex->visit_count);
        if (val > best_val) {
            best_val = val;
            to_ret = to_ex;
        }
    }
    return to_ret;
}
node* MyEngine::tree_policy(node* curr_node) {
    while (not curr_node->is_terminal()) {
        if (curr_node->is_fully_expanded()) {
            curr_node = best_child(curr_node);
            step_into(curr_node);
        }
        else return expand(curr_node);
    }
    return curr_node;
}
double MyEngine::default_policy(node* to_roll) {
    for (int j = 0; j < width; j++) {
        buffer_top[j] = top[j];
        for (int i = top[j] + 1; i < height; i++) buffer[i][j] = board[i][j];
    }
    bool mach_turn = not to_roll->is_mach;
    while (true) {
        int choice = rand() % width;
        while (column_is_full(choice)) {
            choice += 1;
            choice %= width;
        }
        buffer[buffer_top[choice]][choice] = mach_turn ? 2 : 1;
        if (mach_turn) {
            if (machineWin(buffer_top[choice], choice, height, width, buffer)) return 1.0;
            if (isTie(width, buffer_top)) return 0.0;
        } else {
            if (userWin(buffer_top[choice], choice, height, width, buffer)) return -1.0;
            if (isTie(width, buffer_top)) return 0.0;
        }
        buffer_top[choice] -= 1;
        if (choice == ban_y and buffer_top[choice] == ban_x) buffer_top[choice] -= 1;
        mach_turn = not mach_turn;
    }
    assert(false);
}
void MyEngine::propagate_backwards(node* to_report, double delta) {
    while (to_report) {
        to_report->q += delta;
        to_report->visit_count += 1;
        step_from(to_report);
        to_report = to_report->parent;
    }
}

Point* MyEngine::search(const int last_x, const int last_y, time_t ponder_limit) {
    if (memory == nullptr) {
        memory = new node(last_x, last_y, width, height, nullptr);
        if (last_x != -1 and last_y != -1) step_into(memory);
    } else {
        for (int i = 0; i < memory->curr_children_cnt; i++) {
            if (last_y == memory->children[i]->curr_y) {
                auto new_root = memory->children[i];
                memory.clean(new_root);
                delete memory;
                memory = new_root;
                step_into(new_root);
                break;
            }
        }
    }

    while (clock() < ponder_limit) {
        auto vl = tree_policy(memory);
        auto delta = default_policy(vl);
        propagate_backwards(vl, delta);
    }
    auto to_ret = best_child(memory);
    memory->clean(to_ret);
    delete memory;
    memory = to_ret;
    step_into(to_ret);
    return new Point(to_ret->curr_x, to_ret->curr_y);
}
