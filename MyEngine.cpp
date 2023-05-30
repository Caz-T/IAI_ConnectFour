//
// Created by Chen Qingzhi on 2023/5/30.
//

#include <random>
#include <algorithm>
#include "MyEngine.h"

using namespace std;

node::node(int x, int y, int w, int h, node* p): currx(x), curry(y), width(w), height(h), parent(p) {
    children = new node*[w];
    to_expand = new int[w];
    random_shuffle(to_expand, to_expand + w);
}

node* MyEngine::best_child(node* p) {
    double best_value = -1.0;
    node* best_node = nullptr;
    for (int i = 0; i < p->list_top; i++) {
        auto curr_child = p->children[i];
        double curr_value = curr_child->win_count * 1.0 / curr_child->visit_count + UCT_CONST * sqrt(2 * log(p->visit_count) / log(curr_child->visit_count));
        if (curr_value > best_value) {
            best_value = curr_value;
            best_node = curr_child;
        }
    }
    return best_node;
}

MyEngine::MyEngine(const int m, const int n, const int bx, const int by): width(n), height(m), ban_x(bx), ban_y(by) {
    memory = nullptr;
}

Point *MyEngine::search(const int last_x, const int last_y) {
    if (memory == nullptr) to_debug = new Point(0, 0);
    else to_debug->y++;
    return to_debug;
}


