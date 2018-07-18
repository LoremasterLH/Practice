#include "list.h"
#include <iostream>

#define debug

void List::add_node_start(void *data)
{
    Node *node = new Node();
    node->val = data;
    if(head == nullptr)  // When the list is empty.
    {
        head = node;
        tail = node;
    }
    else
    {
        node->prev = nullptr;
        node->next = head;
        head->prev = node;
        head = node;
    }
#ifdef debug
        std::cout << "Adding node with data " << *static_cast<int*>(data) << " at start";
        print_list();
#endif
}

void List::add_node_end(void *data)
{
    Node *node = new Node();
    node->val = data;
    if(head == nullptr)
    {
        head = node;
        tail = node;
    }
    else
    {
        node->prev = tail;
        node->next = nullptr;
        tail->next = node;
        tail = node;
    }
#ifdef debug
    std::cout << "Adding node with data " << *static_cast<int*>(data) << " at end";
    print_list();
#endif
}

void *List::extract_node_start()
{
    Node *node = head;
    int data = *static_cast<int*>(node->val);
    head = node->next;
    head->prev = nullptr;
#ifdef debug
    std::cout << "Extracting node with data " << data << " from start";
    print_list();
#endif
    return node;
}

void *List::extract_node_end()
{
    Node *node = tail;
    int data = *static_cast<int*>(node->val);
    tail = node->prev;
    tail->next = nullptr;
#ifdef debug
    std::cout << "Extracting node with data " << data << " from end";
    print_list();
#endif
    return node;
}

void *List::extract_node_at(const int &index)
{
    Node *node=head;
    int count=0;
    int data;
    while(node != nullptr)
    {
        if(count == index)
        {
            data = *static_cast<int*>(node->val);
            node->prev->next = node->next;
            node->next->prev = node->prev;
#ifdef debug
            std::cout << "Extracting node with data " << data << " from " << index;
            print_list();
#endif
            return node;
        }
        count++;
        node = node->next;
    }
    return nullptr;
}

void List::add_node_after(const int &index, void *data)
{
    Node *node=head;
    Node *new_node;
    int count=0;
    while(node != nullptr)
    {
        if(count == index)
        {
            new_node->val = data;
            new_node->prev = node;
            new_node->next = node->next;
            node->next = new_node;
            new_node->next->prev = new_node;
#ifdef debug
            std::cout << "Adding node with data " << *static_cast<int*>(data) << " after " << index;
            print_list();
#endif
            return;
        }
        count++;
        node = node->next;  // Iterates through the list.
    }
}

void List::print_list()
{
    Node *node = head;
    std::cout << std::endl;
    while(node != nullptr)  // Go through list until the end.
    {
        std::cout << *static_cast<int*>(node->val) << "\t";
        node = node->next;
    }
    std::cout << std::endl;
    delete node;
}
