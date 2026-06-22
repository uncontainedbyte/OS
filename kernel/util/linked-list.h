#pragma once

#include "../int.h"





typedef struct _LinkedList_Node{
	struct _LinkedList_Node* prev;
	struct _LinkedList_Node* next;
} LinkedList_Node;

typedef struct{
	LinkedList_Node* head;
	LinkedList_Node* tail;
	uint32 count;
} LinkedList;

void LIST_init(LinkedList* list);
void LIST_push_front(LinkedList* list,LinkedList_Node* node);
void LIST_push_back(LinkedList* list,LinkedList_Node* node);
LinkedList_Node* LIST_pop_front(LinkedList* list);
LinkedList_Node* LIST_pop_back(LinkedList* list);
void LIST_remove(LinkedList* list,LinkedList_Node* node);
void LIST_insert_before(LinkedList* list,LinkedList_Node* before_this,LinkedList_Node* node);
void LIST_insert_after(LinkedList* list,LinkedList_Node* after_this,LinkedList_Node* node);





































