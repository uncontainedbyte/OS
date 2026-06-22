#pragma once

#include "linked-list.h"



typedef LinkedList Queue;

#define QUEUE_push(queue,node) (LIST_push_back(queue,node))
#define QUEUE_pop(queue)       (LIST_pop_front(queue))
#define QUEUE_empty(queue)     ((queue)->count == 0)
