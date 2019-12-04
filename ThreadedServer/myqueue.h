// MY QUEUE
#ifndef MYQUEUE_H_
#define MYQUEUE_H_

typedef struct node{
    struct node* next;
    int *client_socket;
}node_t;

void enqueue(int *client_socket);
int* dequeue();

#endif
