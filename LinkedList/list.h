#ifndef LIST_H
#define LIST_H
#include "node.h"

class List {
private:
    Node *head=nullptr;
    Node *tail=nullptr;
    void print_list();

public:
    void add_node_start(void *data);
    void add_node_end(void *data);
    void *extract_node_start();
    void *extract_node_end();
    void *extract_node_at(const int &index);
    void add_node_after(const int &index, void *data);
};

#endif // LIST_H
