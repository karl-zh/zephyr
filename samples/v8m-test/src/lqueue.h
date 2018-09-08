#ifndef __LQUEUE_H_  
#define __LQUEUE_H_  

typedef  char queue_element_type;
 
#ifndef __cplusplus
#define bool   _Bool
#define true   1
#define false  0
#endif

typedef struct queue   
{  
	queue_element_type *pBase;
	int front;
	int rear;
	int QueueMaxsize;
}QUEUE,*PQUEUE;  
 
void initial_queue(PQUEUE Q,int queue_maxsize);   
bool full_queue(PQUEUE Q);  
bool empty_queue(PQUEUE Q);  
bool en_queue(PQUEUE Q, queue_element_type val);  
bool de_queue(PQUEUE Q, queue_element_type *val);  
 
#endif
