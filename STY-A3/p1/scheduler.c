#include "scheduler.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>



/* You will need to define data structures for your queues
 * The queue from the Lab tutorial part can serve as a basis
 * But you will need multiple queues (one per priority)
 * Priority values are from 0 [highest] to MAX_PRIORITY [lowest]
 */


/* We will use Queue and QueueItem structs from .h */
static Queue priorityQueues[MAX_PRIORITY + 1]; /* 0 highest, and Max + 1 for lowest*/

static int countPriorityScheduledNum[MAX_PRIORITY + 1]; // We have 11 total priorities, all starting at 0, so 0 times scheduled, when we schedule a task with priority x we add 1 to that x so countPriorityScheduledNum[x] += 1

/*
 * Append to the tail of the queue. Does nothing if out of memory (malloc fails)
 */
void appendItem(Queue *queue, void *data)
{
  struct QueueItem *newItem = (QueueItem *)malloc(sizeof(QueueItem));
  if (newItem == NULL) 
  {
    return; // malloc fail, silent ignore, i.e does nothing
  }
  // Adding an item to the queue, since its a queue it's the last item so next is NULL
  newItem->data = data;
  newItem->next = NULL;
  // If the head is empty, the first item we are adding will be both head and tail
  if (queue->head == NULL) 
  {
    queue->head = newItem;
  } // If its not empty, we make the item currently in the tail point to the new item, to make it the last item.
  else 
  {
    queue->tail->next = newItem;
  }
  // In both cases tail points to the new item
  queue->tail = newItem;
}

/*
 * Retrieve and remove the first element in the queue
 */

void *removeFirstItem(Queue *queue)
{
  if (queue->head == NULL) 
  {
    return NULL; // Queue is empty
  }
  // Get the current head
  struct QueueItem *headItem = queue->head;
  // Get the data of the current head
  void *data = headItem->data;
  // now we make the next item the new head
  queue->head = headItem->next;
  // Free malloc
  free(headItem);
  // return the data that was in the head we just removed
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
    // <= MAX because we want 0 - 10 inclusive
    for (int i = 0; i <= MAX_PRIORITY; i++) 
    {
      priorityQueues[i].head = NULL;
      priorityQueues[i].tail = NULL;
      countPriorityScheduledNum[i] = 0; // All counts for schedules to 0
    }
}

/*
 * Called whenever a waiting task gets ready to run. 
 */
void onTaskReady(Task *task)
{
  int priority = task->priority;
  if (priority < 0 || priority > MAX_PRIORITY) 
  {
    return; // Priorty check, kanski del síðar, þarf kanski ekki
  }
  task->state = STATE_READY; // state ready to run
  appendItem(&priorityQueues[priority], task); // append the task to the priorityqueue when ready to run
}

/*
 * Called whenever a running task is forced of the CPU
 * (e.g., through a timer interrupt).
 * If needed, sets task->state and/or adds task to a queue
 */
void onTaskPreempted(Task *task)
{
  int priority = task->priority;
  if (priority < 0 || priority > MAX_PRIORITY) 
  {
    return;
  }
  // Its interrupted so assuming it hasnt finished, we need to add it back to the queue and set the state as ready to run again so it can finish
  task->state = STATE_READY; // state ready to run
  appendItem(&priorityQueues[priority], task); // append the task to the priorityqueue when ready to run
}

/*
 * Called whenever a running task needs to wait.
 * If needed, sets task->state and/or adds task to a queue
 */
void onTaskWaiting(Task *task)
{
  task->state = STATE_WAITING; // Just waiting state not added to the queue since its not ready to run
}

/*
 * Decides which task is the next task to run and sets its state to running.
 */
Task *scheduleNextTask() 
{
  for (int priority = 0; priority <= MAX_PRIORITY; priority++) 
  { // 0 is highest priority so we look for a DLL of the tasks in that priority index
    // If it is not empty, and this priority has not been scheduled 4 times before we can continue
    if (priorityQueues[priority].head != NULL && countPriorityScheduledNum[priority] < MAX_SAMEPRIO_LENGTH) 
    {
      Task *task = removeFirstItem(&priorityQueues[priority]);
      if (task != NULL) 
      {
        task->state = STATE_RUNNING;
        countPriorityScheduledNum[priority]++;
        return task;
      }
    }
    else
    {
      // This priority has been scheduled more than 4 times so we try to schedule the next priority after it in the list.
      for (int oneLessPriority = priority + 1; oneLessPriority <= MAX_PRIORITY; oneLessPriority++) 
      {
        // for all the priorites after this one we need to check if they are below the schedule limit aswell and are not empty, so there are tasks there
        if (priorityQueues[oneLessPriority].head != NULL && countPriorityScheduledNum[oneLessPriority] < MAX_SAMEPRIO_LENGTH) 
        {
          // If we find a priority that we can schedule and is not empty
          Task *task = removeFirstItem(&priorityQueues[oneLessPriority]);
          if (task != NULL) 
          {
            task->state = STATE_RUNNING;
            countPriorityScheduledNum[oneLessPriority]++;
            countPriorityScheduledNum[priority] = 0; // reset count
            return task;
          }
        }
      }
      countPriorityScheduledNum[priority] = 0; // We need to reset the schedule count for the current priority even if no lower priority tasks exist
    }
  }
  return NULL;
}

