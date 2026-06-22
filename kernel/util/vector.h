#pragma once

#include "../int.h"
#include "../memory.h"



typedef struct{
	void* data;
	uint32 size;
	uint32 capacity;
	uint32 element_size;
} Vector;

void VECTOR_init(Vector* vec,uint32 element_size);
void VECTOR_push_back(Vector* vec,void* element);
void* VECTOR_pop_back(Vector* vec);
void* VECTOR_at(Vector* vec,uint32 index);
void VECTOR_reserve(Vector* vec,uint32 count);
void VECTOR_clear(Vector* vec);
void VECTOR_destroy(Vector* vec);



































