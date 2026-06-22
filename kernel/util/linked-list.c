#include "linked-list.h"



void LIST_init(LinkedList* list){
	list->count = 0;
	list->head = 0;
	list->tail = 0;
}

void LIST_push_front(LinkedList* list,LinkedList_Node* node){
	node->next = list->head;
	node->prev = 0;
	if(list->head){ list->head->prev = node;
	}else{ list->tail = node; }
	list->head = node;
	list->count++;
}
void LIST_push_back(LinkedList* list,LinkedList_Node* node){
	node->prev = list->tail;
	node->next = 0;
	if(list->tail){ list->tail->next = node;
	}else{ list->head = node; }
	list->tail = node;
	list->count++;
}

LinkedList_Node* LIST_pop_front(LinkedList* list){
	if(!list->head) return 0;
	
	LinkedList_Node* node = list->head;
	list->head = node->next;
	if(list->head){ list->head->prev = 0;
	}else{ list->tail = 0; }
	list->count--;
	
	node->prev = 0;
	node->next = 0;
	return node;
}
LinkedList_Node* LIST_pop_back(LinkedList* list){
	if(!list->tail) return 0;
	
	LinkedList_Node* node = list->tail;
	list->tail = node->prev;
	if(list->tail){ list->tail->next = 0;
	}else{ list->head = 0; }
	list->count--;
	
	node->prev = 0;
	node->next = 0;
	return node;
}

void LIST_remove(LinkedList* list,LinkedList_Node* node){
	
	if(node->prev) node->prev->next = node->next;
	if(node->next) node->next->prev = node->prev;
	
	if(list->head == node) list->head = node->next;
	if(list->tail == node) list->tail = node->prev;
	
	node->prev = 0;
	node->next = 0;
	list->count--;
}

void LIST_insert_before(LinkedList* list,LinkedList_Node* before_this,LinkedList_Node* node){
	node->next = before_this;
	node->prev = before_this->prev;
	if(before_this->prev){ before_this->prev->next = node;
	}else{ list->head = node; }
	before_this->prev = node;
	list->count++;
}
void LIST_insert_after(LinkedList* list,LinkedList_Node* after_this,LinkedList_Node* node){
	node->next = after_this->next;
	node->prev = after_this;
	if(after_this->next){ after_this->next->prev = node;
	}else{ list->tail = node; }
	after_this->next = node;
	list->count++;
}





























