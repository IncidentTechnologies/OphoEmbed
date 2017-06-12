#include "taskqueue.h"

TASK_QUEUE g_taskQueue;

RESULT CreateTaskQueue(TASK_QUEUE *taskQueue) {
	RESULT r = R_OK;

	memset(taskQueue, 0, sizeof(TASK_QUEUE));
	taskQueue->m_pTaskQueue = CreateList();
	taskQueue->m_countID = 1;
	taskQueue->m_intervalMS = DEFAULT_TASK_QUEUE_INTERVAL_MS;

Error:
	return r;
}

// Initializes the global task queue
RESULT InitializeTaskQueue() {
	RESULT r = R_OK;

	CRM(CreateTaskQueue(&g_taskQueue), "InitializeTaskQueue: Failed to initialize task queue");

Error:
	return r;
}

QUEUE_TASK *CreateTask(fnQueueTask fnTask, int argc, char *argv[], unsigned char fLoop) {
	QUEUE_TASK *newTask = (QUEUE_TASK*)calloc(1, sizeof(QUEUE_TASK));

	newTask->m_fnTask = fnTask;
	newTask->m_argc = argc;
	newTask->m_argv = argv;
	newTask->m_fLoop = fLoop;
	newTask->m_ID = g_taskQueue.m_countID++;

	return newTask;
}

RESULT AddTaskLoop(fnQueueTask fnTask, int argc, char *argv[]) {
	RESULT r = R_OK;

	QUEUE_TASK *newTask = CreateTask(fnTask, argc, argv, TRUE);
	CNRM(newTask, "AddTaskLoop: Failed to allocate new task");

	CRM(PushItem(g_taskQueue.m_pTaskQueue, newTask), "AddTaskLoop: Failed to push new task to queue");

Error:
	return r;
}

RESULT AddTask(fnQueueTask fnTask, int argc, char *argv[]) {
	RESULT r = R_OK;

	QUEUE_TASK *newTask = CreateTask(fnTask, argc, argv, FALSE);
	CNRM(newTask, "AddTask: Failed to allocate new task");

	CRM(PushItem(g_taskQueue.m_pTaskQueue, newTask), "AddTask: Failed to push new task to queue");

Error:
	return r;
}

QUEUE_TASK *GetCurrentTask() {
	return g_taskQueue.m_currentTask;
}

unsigned char TaskPending() {
	return (g_taskQueue.m_pTaskQueue->m_count != 0);
}

int GetTaskQueueIntervalMs() {
	return g_taskQueue.m_intervalMS;
}

RESULT UpdateTaskQueue() {
	RESULT r = R_OK;

	if(TaskPending())
		CRM(ExecuteTaskQueue(), "UpdateTaskQueue: Failed to execute task queue");

Error:
	return r;
}

// This will execute the task and then deallocate it's memory/context
RESULT ExecuteTaskQueue() {
	RESULT r = R_OK;
	unsigned char fDeallocate = TRUE;

	// First pop the task
	QUEUE_TASK *task = (QUEUE_TASK*)(PopFrontItem(g_taskQueue.m_pTaskQueue));
	g_taskQueue.m_currentTask = task;

	CRM(task->m_fnTask(task->m_argc, task->m_argv), "ExecuteTaskQueue: Failed to execute task TID: %d", task->m_ID);

	// If the loop flag is set, don't deallocate and run the task again
	if(task->m_fLoop == TRUE) {
		fDeallocate = FALSE;
		CRM(PushItem(g_taskQueue.m_pTaskQueue, task), "ExecuteTaskQueue: Failed to push task back into queue");
	}

Error:
	if(task != NULL && fDeallocate == TRUE) {
		DeallocateTask(task);
		task = NULL;
	}
	g_taskQueue.m_currentTask = NULL;
	return r;
}

RESULT DeallocateTask(QUEUE_TASK *task) {
	RESULT r = R_OK;

	// Not much to do except release the argv
	free(task->m_argv);

Error:
	return r;
}
