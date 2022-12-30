#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

struct node{
 void *data;
 struct node * next;   
};

struct node * push_data(struct node *head);

#endif
