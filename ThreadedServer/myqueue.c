#include "myqueue.h"
#include <stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;

void enqueue(int *client_socket){
    node_t *newNode = malloc(sizeof(node_t));

    newNode->client_socket = client_socket;
    newNode->next = NULL;
    if(tail == NULL){
        head = newNode;
    } else {
        tail->next = newNode;
    }

    tail = newNode;
}

// Return null if queue is empty
int* dequeue(){
   if(head == NULL){
        return NULL;
    } else {
        int *result = head->client_socket;
        node_t *temp= head;
        head = head->next;
        if(head == NULL){ tail = NULL;}
        free(temp);
        return result;
    }
}
