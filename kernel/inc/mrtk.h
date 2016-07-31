/*
 *      Copyright (c) 2015 timecriticalsystem.
 *       All rights reserved.
 */
#ifndef MRTK
#define MRTK

#include "mrtk_port.h"

/*  Size of the task name */
#define SIZETASKNAME    32

/* Size of the task type */
#define SIZETASKTYPE    32

/* Size of the event type */
#define SIZEEVENTTYPE   32

/* Maximum number of task waiting for an event */
#define MAXNBTASKFOREVENT   32


/* Maximum number of event */
#define NB_EVENT_MAX    4

/* Minimum priority of a task */
#define MIN_TASK_PRIORITY   1

/* Maximum number of processor */
#define MAX_NB_PROCESSOR    2

/*  Minimum number of processor */
#define MIN_NB_PROCESSOR   1

/* Maximum number of users tasks */
#define NB_USERS_TASK_MAX	56

/* Maximum number of tasks */
#define NB_TASK_MAX	(NB_USERS_TASK_MAX + MAX_NB_PROCESSOR)

/* Maximum task's id */
#define MAX_TASK_ID   NB_TASK_MAX

/* Minimum task's id */
#define MIN_TASK_ID   0

/* the kernel has been initialized */
#define MRTK_KERNEL_INITIALIZED 1

/* good paramater passed for kernel initialization */
#define MRTK_KERNEL_GOOD_PARAMETER  0

/* the number of processor is not in bounds */
#define MRTK_KERNEL_ERROR_PARAMETER -1

/* the maximum value of the Dkc, the user manual explains how this value
     is computed */
#define MRTK_DKC_MAX    40

#define MRTK_MAX_MSG_QUEUE	32

/* idle task context size */
#define TASK_CONTEXT_SIZE 64

#define START_CPU_ID	0

/****************************************************************************/
/*                      Different states of a task                          */
/*                                                                          */
/****************************************************************************/

typedef enum
{
    SLEEPING = 0,   /* the task is not yet created*/
    READY = 3,  /* the task is ready to run */
    EXECUTING = 6,  /* the task is running */
    SUSPENDED = 9,  /* the task is suspended, until a timeout occurs */
    WAITING = 12   /* the task is waiting for an event */
} TASKSTATE;


/****************************************************************************/
/*                      Event Control Block                                 */
/*                                                                          */
/****************************************************************************/

typedef struct event
{
    Uint8 EventType;    /* Event's type*/
    void *Data;     /* for event generate by the arrival of data */
    Uint16 EventCounter;    /* if the event is a semaphore */
    Uint16 TaskId;
    Boolean	IsTimeoutEnable;
} EVENT;

typedef struct
{
	void** QStart;
	void** QEnd;
	void** QIn;
	void** QOut;
	Uint16	QEntries;
	Uint16 QSize;
} MQueueControlBlock;

enum
{
	NO_EVENT = 1,
	MSG_QUEUE_EVENT,
	SEMAPHORE_EVENT
};

typedef enum
{
	NO_ERROR,
	ERROR_NULL_EVENT,
	ERROR_BAD_EVENT_TYPE,
	EVENT_TIMEOUT,
	ERROR_QUEUE_FULL,
	ERROR_PENDING_IN_INTERRUPT, /* cannot wait to an event, when we are in interrupt */
	ERROR_SEM_OVERFLOW
} EVENT_ERROR;

/****************************************************************************/
/*                      Task Control Block                                  */
/*                                                                          */
/****************************************************************************/

/* This structure keeps information allowing to manage and schedule the task */
typedef struct tcb
{
    Uint16 TaskId;          /* Task id (0..65535) */
    Uint32 ExecutionTime;	/* Task execution time */
    Uint32 Deadline;		/* Task Relative Deadline */
    Long64 AbsoluteDeadline;		/* Task Absolute Deadline */
    Uint32 Period;			/* Task Period, we  should have Deadline <= Period */
    Uint8 TaskName[SIZETASKNAME];   /* Task's name */
    TASKSTATE State;    /* The state of the task */
    EVENT *EventList;   /* pointer to the event list */
    STACK *StartStkptr; /* start stack pointer */
    STACK *EndStkptr;   /* end of the stack */
    Uint32 Delay;       /* the current delay of the task */
} TCB;


extern TCB TaskTcbTable[NB_TASK_MAX];

extern EVENT EventTable[NB_EVENT_MAX];

extern Uint16 NbProcessors;

extern Boolean MrtkTimelock;

extern Boolean TaskTcbTablelock;

extern Long64 MrtkTime_ms[MAX_NB_PROCESSOR];

extern Boolean MrtkRunning;

extern Uint16 RunningTasks[MAX_NB_PROCESSOR];

extern Uint32 EventTblFreeEntry;

extern Int16 mrtk_Init(Uint16 ip_nbprocessor, Float32 maxcpuusage);
extern void mrtk_start();

#endif
