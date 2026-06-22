#include "vector.h"





void VECTOR_init(Vector* vec,uint32 element_size){
	if(!vec) return;
	
	vec->data = kalloc(1*element_size);
	vec->size = 0;
	vec->capacity = 1;
	vec->element_size = element_size;
}
void VECTOR_push_back(Vector* vec,void* element){
	if(!vec) return;
	if(vec->size == vec->capacity){
		vec->capacity = vec->capacity*2;
		vec->data = krealloc(vec->data,vec->capacity*vec->element_size);
	}
	memcpy(((char*)vec->data + vec->element_size*vec->size),element,vec->element_size);
	vec->size++;
}
void* VECTOR_pop_back(Vector* vec){
	if(!vec) return 0;
	if(!vec->size) return 0;
	vec->size--;
	return ((char*)vec->data + vec->element_size*vec->size);
}
void* VECTOR_at(Vector* vec,uint32 index){
	if(!vec) return 0;
	if(!vec->size) return 0;
	if(vec->size <= index) return 0;
	return ((char*)vec->data + vec->element_size*index);
}
void VECTOR_reserve(Vector* vec,uint32 count){
	if(!vec) return;
	if(count <= vec->capacity) return;
	
	vec->capacity = count;
	vec->data = krealloc(vec->data,vec->capacity*vec->element_size);
}
void VECTOR_clear(Vector* vec){
	if(!vec) return;
	vec->size = 0;
}
void VECTOR_destroy(Vector* vec){
	if(!vec) return;
	
	if(vec->data) kfree(vec->data);
	vec->data = 0;
	vec->capacity = 0;
	vec->size = 0;
	vec->element_size = 0;
}












































































