#pragma once
 
struct thread_queue{
    thread_queue* next;
    int *client_socket;
};

typedef struct thread_queue node_t;

node_t* head = NULL;
node_t* tail = NULL;


void enqueue(int *client_socket){
    node_t *newnode = new node_t();
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if(tail == NULL){
        head = newnode;
    }else{
        tail->next = newnode;
    }
    tail = newnode;
}

int* dequeue(){
    if(head == NULL) return NULL;
    else{
        int* result = head->client_socket;
        node_t *temp = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        free(temp);
        return result;
    }
}
