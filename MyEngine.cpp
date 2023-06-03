//
// Created by Chen Qingzhi on 2023/5/30.
//

#include <random>
#include <cassert>
#include <iostream>
#include <cstring>
#include "MyEngine.h"
#include "Judge.h"

using namespace std;

node::node(int x, int y, int w, int h, node* p): curr_x(x), curr_y(y), parent(p), width(w), height(h) {
    children = new node*[w];
    for (int i = 0; i < w; i++) children[i] = nullptr;
    is_mach = p and not p->is_mach;
}

inline bool node::is_terminal() const { return is_term; }
inline bool node::is_fully_expanded() const { return curr_children_cnt >= width; }

void node::clean(node *to_save) const {
    for (int i = 0; i < curr_children_cnt; i++) {
        if (children[i] == nullptr) continue;
        if (children[i] != to_save) {
            children[i]->clean();
            delete children[i];
        }
    }
    delete[] children;
}

MyEngine::MyEngine(const int m, const int n, const int bx, const int by, double c): height(m), width(n), ban_x(bx), ban_y(by),
board(new int*[m]), top(new int[n]), buffer(new int*[m]), buffer_top(new int[n]), uct_const(c) {
    for (int i = 0; i < m; i++) {
        board[i] = new int[n];
        memset(board[i], 0, sizeof(int) * n);
        buffer[i] = new int[n];
        memset(buffer[i], 0, sizeof(int) * n);

    }
    for (int i = 0; i < n; i++) top[i] = m - 1;
    if (ban_x == m - 1) top[ban_y] = m - 2;
}

MyEngine::~MyEngine() {
    for (int i = 0; i < height; i++) delete[] board[i];
    delete[] board;
    delete[] top;
}

bool MyEngine::column_is_full(const int n, bool is_dry_run) {
    auto board_to_check = is_dry_run ? buffer : board;
    return (ban_x == 0 and ban_y == n) ? board_to_check[1][n] != 0 : board_to_check[0][n] != 0;
}

void MyEngine::step_into(node* v) {
    // cout << "Stepping into (" << v->curr_x << ", " << v->curr_y << ")" << endl;
    // print_board();
    board[v->curr_x][v->curr_y] = v->is_mach ? 2 : 1;
    top[v->curr_y] -= 1;
    if (v->curr_y == ban_y and top[v->curr_y] == ban_x) top[v->curr_y] -= 1;
}
void MyEngine::step_from(node* v) {
    // cout << "Stepping out from (" << v->curr_x << ", " << v->curr_y << ")" << endl;
    // print_board();
    board[v->curr_x][v->curr_y] = 0;
    top[v->curr_y] = v->curr_x;
}
void MyEngine::step_into_faked(int x, int y, bool is_mach) {
    buffer[x][y] = is_mach ? 2 : 1;
    buffer_top[y] -= 1;
    if (y == ban_y and buffer_top[y] == ban_x) buffer_top[y] -= 1;
}
void MyEngine::step_from_faked(int x, int y) {
    buffer[x][y] = 0;
    buffer_top[y] = x;
}

node* MyEngine::expand(node* to_expand) {
    for (int i = to_expand->curr_children_cnt; i < width; i++) {
        if (not column_is_full(i)) {
            auto expanded = new node(this->top[i], i, this->width, this->height, to_expand);
            to_expand->children[i] = expanded;
            i += 1;
            while (i < width and column_is_full(i) ) i += 1;
            to_expand->curr_children_cnt = i;
            step_into(expanded);
            expanded->is_term = (expanded->is_mach and machineWin(expanded->curr_x, expanded->curr_y, height, width, board))
                    or (not expanded->is_mach and userWin(expanded->curr_x, expanded->curr_y, height, width, board))
                    or isTie(width, top);
            // expansion_cnt += 1;
            return expanded;
        }
    }
    assert(false);
}
node* MyEngine::best_child(node* to_check) const {
    double best_val = -3.0;
    node* to_ret = nullptr;
    for (int i = 0; i < to_check->curr_children_cnt; i++) {
        auto to_ex = to_check->children[i];
        if (to_ex == nullptr) continue;
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
    // if (expansion_cnt > expansion_limit * 1.2) return curr_node;  // prevent overflow beforehand
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
    // cerr << "Starting default policy" << endl;
    bool mach_turn = to_roll->is_mach;
    if (mach_turn) {
        if (machineWin(to_roll->curr_x, to_roll->curr_y, height, width, board)) return 1.0;
        if (isTie(width, top)) return 0.0;
    } else {
        if (userWin(to_roll->curr_x, to_roll->curr_y, height, width, board)) return -1.0;
        if (isTie(width, top)) return 0.0;
    }
    mach_turn = not mach_turn;
    // cerr << "Game not ended, starting rollout" << endl;
    for (int j = 0; j < width; j++) {
        buffer_top[j] = top[j];
        for (int i = 0; i < height; i++) buffer[i][j] = board[i][j];
    }
    // cerr << "Constructed rollout board" << endl;
    while (true) {
        int choice_y = rand() % width;
        while (column_is_full(choice_y, true)) {
            choice_y += 1;
            choice_y %= width;
        }
        auto choice_x = buffer_top[choice_y];
        if (choice_y == ban_y and choice_x == ban_x) choice_x -= 1;
        buffer[choice_x][choice_y] = mach_turn ? 2 : 1;
        buffer_top[choice_y] = choice_x - 1;
        if (mach_turn) {
            if (machineWin(choice_x, choice_y, height, width, buffer)) return 1.0;
            if (isTie(width, buffer_top)) return 0.0;
        } else {
            if (userWin(choice_x, choice_y, height, width, buffer)) return -1.0;
            if (isTie(width, buffer_top)) return 0.0;
        }
        mach_turn = not mach_turn;

        // cerr << "Rolling out..." << endl;
    }
    assert(false);
}
void MyEngine::propagate_backwards(node* to_report, double delta) {
    while (true) {
        to_report->q += delta;
        to_report->visit_count += 1;
        if (to_report->parent == nullptr) break;
        step_from(to_report);
        to_report = to_report->parent;
    }
}

Point* MyEngine::search(const int last_x, const int last_y, time_t ponder_limit) {
    /*
    if (expansion_cnt > expansion_limit) {
        memory->clean();
        delete memory;
        memory = nullptr;
        expansion_cnt = 1;
    }
     */
    if (memory == nullptr) {
        memory = new node(last_x, last_y, width, height, nullptr);
        if (last_x != -1 and last_y != -1) step_into(memory);
    } else {
        auto new_root = memory->children[last_y];
        memory->clean(new_root);
        delete memory;
        memory = new_root;
        if (memory == nullptr) memory = new node(last_x, last_y, width, height, nullptr);
        memory->parent = nullptr;
        step_into(memory);
    }

    node* to_ret;
    int kanarazu = ikanakerebanaranai(1);
    if (kanarazu != -1) {
        if (memory->children[kanarazu] == nullptr) {
            memory->children[kanarazu] = new node(top[kanarazu], kanarazu, width, height, memory);
        }
        to_ret = memory->children[kanarazu];
        cerr << "kanarazu triggered! " << kanarazu << endl;
    } else {
        while (clock() < ponder_limit) {
            auto vl = tree_policy(memory);
            auto delta = default_policy(vl);
            propagate_backwards(vl, delta);
        }
        to_ret = best_child(memory);
    }

    memory->clean(to_ret);
    delete memory;
    memory = to_ret;
    memory->parent = nullptr;
    step_into(to_ret);
    return new Point(to_ret->curr_x, to_ret->curr_y);
}

void MyEngine::print_board() const {
    cout << "Board:" << endl;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
    cout << "Top: ";
    for (int j = 0; j < width; j++) cout << top[j] << " ";
    cout << endl;
}

int MyEngine::ikanakerebanaranai(int layer_cnt) {
    for (int layer = 0; layer < layer_cnt; layer++) {
        // probe for definite wins
        for (int j = 0; j < width; j++) {
            for (int i = 0; i < height; i++) {
                buffer[i][j] = board[i][j];
            }
            buffer_top[j] = top[j];
        }
        for (int i = 0; i < width; i++) {
            if (column_is_full(i, true)) continue;
            if (leads_to_victory(buffer_top[i], i, layer)) return i;
        }

        // probe for definite losses
        for (int j = 0; j < width; j++) {
            for (int i = 0; i < height; i++) {
                switch(board[i][j]) {
                    case 2: buffer[i][j] = 1; break;
                    case 1: buffer[i][j] = 2; break;
                    default: buffer[i][j] = 0; break;
                }
            }
            buffer_top[j] = top[j];
        }
        for (int i = 0; i < width; i++) {
            if (column_is_full(i, true)) continue;
            if (leads_to_victory(buffer_top[i], i, layer)) return i;
        }
    }

    return -1;
}

bool MyEngine::leads_to_victory(int x, int y, int allowed_recursions) {
    step_into_faked(x, y, true);
    if (machineWin(x, y, height, width, buffer)) {
        step_from_faked(x, y);
        return true;
    }
    if (isTie(width, buffer_top) or allowed_recursions == 0) {
        step_from_faked(x, y);
        return false;
    }
    for (int opy = 0; opy < width; opy++) { // opponent's y
        if (column_is_full(opy, true)) continue; // this won't cause infinite loop since there's no draws hitherto
        int opx = buffer_top[opy];
        step_into_faked(opx, opy, false);
        if (userWin(opx, opy, height, width, buffer) or isTie(width, buffer_top)) {
            step_from_faked(opx, opy);
            step_from_faked(x, y);
            return false;
        }
        bool flag = false;
        for (int response = 0; response < width; response++) {
            if (column_is_full(response, true)) continue;
            if (leads_to_victory(buffer_top[response], response, allowed_recursions - 1)) {
                // successfully responded to this opposing move
                flag = true;
                break;
            }
        }
        if (not flag) { // if this opposing move cannot be properly responded
            step_from_faked(opx, opy);
            step_from_faked(x, y);
            return false;  // the move in question is not considered a kanarazu
        }
        step_from_faked(opx, opy);
    }
    step_from_faked(x, y);
    return true;
}
