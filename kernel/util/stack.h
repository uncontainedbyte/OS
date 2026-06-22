#pragma once

#include "vector.h"



typedef Vector Stack;

#define STACK_push(stack,val) (VECTOR_push_back(stack,val))
#define STACK_pop(stack)      (VECTOR_pop_back(stack))
#define STACK_top(stack)      (VECTOR_at(stack,stack->size-1))
#define STACK_empty(stack)    ((stack)->size == 0)
