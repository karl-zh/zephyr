#include "lqueue.h"
#include <stdlib.h>
#include <time.h>

#include <wifi_esp8266.h>
#include <cmsis_gcc.h>

void initial_queue(PQUEUE Q,int queue_maxsize)
{
	Q->front = Q->rear = 0;
	Q->pBase = (queue_element_type *)rx_buf;
    Q->QueueMaxsize = queue_maxsize;
}

bool en_queue(PQUEUE Q, queue_element_type val)
{
	if(full_queue(Q))
		return false;
	else
	{
		Q->pBase[Q->rear]=val;
		Q->rear=(Q->rear+1)%Q->QueueMaxsize;
 
		return true;
	}
}

bool de_queue(PQUEUE Q, queue_element_type *val)
{
    int key;
	if(empty_queue(Q))
	{
		return false;
	}
	else
	{
		key = irq_lock();
//        __disable_irq();
        *val=Q->pBase[Q->front];
        Q->front=(Q->front+1)%Q->QueueMaxsize;
//        __enable_irq();
		irq_unlock(key);

		return true;  
	}
}

bool empty_queue(PQUEUE Q)
{
	if(Q->front==Q->rear)
		return true;
	else
		return false;
}

bool full_queue(PQUEUE Q)
{
	if(Q->front==(Q->rear+1)%Q->QueueMaxsize)
		return true;
	else
		return false;
}
