/* date = August 19th 2021 1:17 pm */

#ifndef MOMO_LINKED_LIST_H
#define MOMO_LINKED_LIST_H

// NOTE(Momo): doubly linked list implementation
template<typename T>
struct LinkedList_Node {
    T item;
    LinkedList_Node* next;
    LinkedList_Node* prev;
};


template<typename T>
struct LinkedList {
    u32 count;
    LinkedList_Node<T>* head;
    LinkedList_Node<T>* tail;
};

template<typename T>
static inline void
LinkedList_Push(LinkedList<T>* l, LinkedList_Node<T>* node) {
    if (count == 0) {
        l->head = l->tail = node;
    }
    else {
        l->tail->next = node;
        l->tail = node;
    }
}


template<typename T>
static inline b8
LinkedList_Push(LinkedList<T>* l, T item, Arena* arena) {
    auto* node = Arena_Push<LinkedList_Node<T>>(arena);
    if (!node) {
        return false;
    }
    node->prev = node->next = nullptr;
    node->item = item;
    LinkedList_Push(l, node);
    return true;
}

#endif //MOMO_LINKED_LIST_H
