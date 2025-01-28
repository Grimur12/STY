#include "scheduler.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>



/* You will need to define data structures for your queues
 * The queue from the Lab tutorial part can serve as a basis
 * But you will need multiple queues (one per priority)
 * Priority values are from 0 [highest] to MAX_PRIORITY [lowest]
 */

#define MAX_PRIORITY 10
typedef struct QueueNode {
  struct QueueItem *head;
  struct QueueItem *tail;
  struct QueueItem *next;
  struct QueueNode *task;
} QueueNode;

static Queue priorityQueue[MAX_PRIORITY];
/*
 * Part 1: Helper functions for queue manipulation
 *         (can be based on implemention in tutorial part)
 * (but: you are asked to do a performance optimization by adding a tail pointer)
 */

/*
 * Append to the tail of the queue. Does nothing if out of memory (malloc fails)
 */
void appendItem(Queue *queue, void *data)
{
	(void)queue;
	(void)data;
 struct QueueNode *newItem = (struct QueueNode *)malloc(sizeof(struct QueueNode));

 newItem->task = data;
 newItem->next = NULL;
 if (queue->head == NULL) {
   queue->head = newItem;
 }
 else {
   queue->tail->next = newItem;
   queue->tail = newItem;
 }
}

/*
 * Retrieve and remove the first element in the queue
 */
void *removeFirstItem(Queue *queue)
{
	(void)queue;
 if (queue->head == NULL) {
   return NULL;
 }
 struct QueueNode *head = queue->head;
 void *data = head->task;
 queue->head = head->next;
 free(head);
 return data;
}



/* 
 * Part 2: The actual multi level scheduler
 */

void initScheduler()
{
    /*
     * This function is called once initially and allows you to initialize data structures if needed
     * May in some implementations also just be empty....
     */

    for (int i = 0; i < MAX_PRIORITY; i++) {
      priorityQueue[i].head = NULL;
      priorityQueue[i].tail = NULL;
      }
}

/*
 * Called whenever a waiting task gets ready to run. 
 *
 */
void onTaskReady(Task *task)
{
    (void)task; /* The purpose of this line is just to prevent a compiler warning/error
                   if you try to compile this file before implementing this function
                   As soon as you have implemented this function, you can (and should)
                   remove this line. */

    /* to implement */
}

/*
 * Called whenever a running task is forced of the CPU
 * (e.g., through a timer interrupt).
 * If needed, sets task->state and/or adds task to a queue
 */
void onTaskPreempted(Task *task)
{
    (void)task;
    /* to implement */
}

/*
 * Called whenever a running task needs to wait.
 * If needed, sets task->state and/or adds task to a queue
 */
void onTaskWaiting(Task *task)
{
    (void)task;
    /* to implement */
}

/*
 * Decides which task is the next task to run and sets its state to running.
 */
Task *scheduleNextTask()
{
    // Try to schedule a task with the highest priority.
    // This will return a task with a lower priority if required to prevent
    // starvation.
    // returns NULL if no task is available
    //
    return NULL;
}
