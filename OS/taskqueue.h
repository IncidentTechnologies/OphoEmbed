#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

// The task queue is a simple task queue that will run a task to completion in the queue
// and then remove it from the queue

#include "../Common/EHM.h"
#include "../DS/list.h"

#define DEFAULT_TASK_QUEUE_INTERVAL_MS 500

// Tasks of the signature RESULT fn(int argc, const char *argv[]);
typedef RESULT (*fnQueueTask)(int, const char *[]);

typedef struct {
	fnQueueTask m_fnTask;
	int m_argc;
	char **m_argv;
	unsigned m_fLoop: 1;
	int m_ID;
} QUEUE_TASK;

typedef struct {
	list *m_pTaskQueue;
	int m_countID;
	QUEUE_TASK *m_currentTask;
	int m_intervalMS;
} TASK_QUEUE;

extern TASK_QUEUE g_taskQueue;

RESULT CreateTaskQueue(TASK_QUEUE *taskQueue);
RESULT InitializeTaskQueue();
QUEUE_TASK *GetCurrentTask();
unsigned char TaskPending();

QUEUE_TASK *CreateTask(fnQueueTask fnTask, int argc, char *argv[], unsigned char fLoop);
RESULT AddTask(fnQueueTask fnTask, int argc, char *argv[]);
RESULT AddTaskLoop(fnQueueTask fnTask, int argc, char *argv[]);
RESULT ExecuteTaskQueue();

RESULT UpdateTaskQueue();
int GetTaskQueueIntervalMs();

RESULT DeallocateTask(QUEUE_TASK *task);

#endif // ! TASK_QUEUE_H_
