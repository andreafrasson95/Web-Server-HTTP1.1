#include <stdlib.h>
#include "linked_list.h"

struct node * push_data(struct node *head){
   if(head == NULL){
     head=(struct node *)malloc(sizeof(struct node));
     head->next=NULL;
   }   
   else{
     struct node *current=head;
     while(current->next!=NULL) current=current->next;
     current->next=(struct node *)malloc(sizeof(struct node));
     //current->next->data=data;
     current->next->next=NULL;
   }  
}


