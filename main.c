#define _POSIX_C_SOURCE	199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

#include "QuarkTS.h"
/*============================================================================*/
pthread_t TimerEmulation;
void* TimerInterruptEmulation(void* arg){
    struct timespec tick={0, 0.01*1E9};
    for(;;){
        nanosleep(&tick, NULL);
        qSchedulerSysTick();
    }
}
/*============================================================================*/
qTask_t Task1, Task2, Task3, Task4, Task5, Task6, TaskTestST, blinktask, SMTask, SMTask2;

qSM_Status_t firststate(qSM_t *fsm);
qSM_Status_t secondstate(qSM_t *fsm);

void datacapture(qSM_t *fsm){
    
}

/*============================================================================*/
qSM_Status_t firststate(qSM_t *fsm){
    qEvent_t e = fsm->Data;
    if(e->FirstCall){
        puts("state machine init");
    }
    static qSTimer_t tmr;

    if(fsm->StateFirstEntry){
        qSTimerSet(&tmr, 2.5);
        printf("[%s] first\r\n", (char*)e->TaskData);
    }
    if (qSTimerExpired(&tmr)){
        fsm->NextState = secondstate;
    }
    return qSM_EXIT_SUCCESS;
}
/*============================================================================*/
qSM_Status_t secondstate(qSM_t *fsm){
    qEvent_t e = fsm->Data;
    static qSTimer_t tmr;
    if(fsm->StateFirstEntry){
        qSTimerSet(&tmr, 2.5);
        printf("[%s] second\r\n", (char*)e->TaskData);
    }
    
    if (qSTimerExpired(&tmr)){
        fsm->NextState = firststate;
    }
    return qSM_EXIT_SUCCESS;
}

/*============================================================================*/
void Task1Callback(qEvent_t e){
    static qSTimer_t tmr = QSTIMER_INITIALIZER;
    printf("Userdata : %s  Eventdata:%s   %d\r\n", (char*)e->TaskData, (char*)e->EventData, qTaskGetCycles(&Task1));
    if(e->FirstCall){
        puts("FirstCall");
    }
    if(e->FirstIteration){
        puts("FirtsIteration");
    }
    if(e->LastIteration){
        puts("LastIteration");
    }
    if(qSTimerFreeRun(&tmr, 0.5)){
        puts("Timer expired");
    }     
}
/*============================================================================*/
void Task2Callback(qEvent_t e){
    printf("Userdata : %s  Eventdata:%s\r\n", (char*)e->TaskData, (char*)e->EventData);
}
/*============================================================================*/
void Task3Callback(qEvent_t e){
    printf("Userdata : %s  Eventdata:%s\r\n", (char*)e->TaskData, (char*)e->EventData);
    if(e->Trigger == byRBufferPop){
        int *ptr = (int*)e->EventData;
        printf("ring extracted data %d\r\n",*ptr);
    }
}
/*============================================================================*/
void Task4Callback(qEvent_t e){
    printf("Userdata : %s  Eventdata:%s\r\n", (char*)e->TaskData, (char*)e->EventData);
}
/*============================================================================*/
void Task5Callback(qEvent_t e){
    printf("Userdata : %s  Eventdata:%s\r\n", (char*)e->TaskData, (char*)e->EventData);
}
/*============================================================================*/
void Task6Callback(qEvent_t e){
    printf("Userdata : %s  Eventdata:%s\r\n", (char*)e->TaskData, (char*)e->EventData);   
}
/*============================================================================*/
void IdleTaskCallback(qEvent_t e){
    static qSTimer_t t = QSTIMER_INITIALIZER;
    if(qSTimerFreeRun(&t, 5.0)){
        puts("hi");
        qTaskSetIterations(&Task1, 6);
        qTaskResume(&Task1);      
    }
}
/*============================================================================*/
void blinktaskCallback(qEvent_t e){
    static qSTimer_t tmr;
    qCoroutineSemaphore_t mutex;
    qCoroutineSemaphoreInit(&mutex, 1);
    qCoroutineBegin{
        puts("led on");
        qSTimerSet(&tmr, 1);
        qCoroutineWaitUntil(qSTimerExpired(&tmr));
        qCoroutineSemaphoreWait(&mutex);
        qSTimerSet(&tmr, 1);
        qCoroutineSemaphoreSignal(&mutex);
        puts("led off");
        qCoroutineWaitUntil(qSTimerExpired(&tmr));
    }qCoroutineEnd;
}
/*============================================================================*/
int main(int argc, char** argv) {
    qRBuffer_t ringBuffer;
    pthread_create(&TimerEmulation, NULL, TimerInterruptEmulation, NULL );
    qMemoryHeapCreate(mtxheap, 10, qMB_4B);
    qSM_t statemachine;
    void *memtest;
    int x=5 , y=6;
    
    memtest = qMemoryAlloc(&mtxheap, 10*sizeof(int));
    qRBufferInit(&ringBuffer, memtest, sizeof(int), 10);
    qRBufferPush(&ringBuffer, &x);
    qRBufferPush(&ringBuffer, &y);
    qSchedulerSetup(0.01, IdleTaskCallback, 10);  
    
    qSchedulerAddxTask(&blinktask, blinktaskCallback, qLowest_Priority, 0.05, qPeriodic, qEnabled, "blink");
    qSchedulerAddxTask(&Task1, Task1Callback, qHigh_Priority, 0.5, 5, qEnabled, "TASK1");
    qSchedulerAddeTask(&Task3, Task3Callback, qMedium_Priority, "TASK3");
    qTaskLinkRBuffer(&Task3, &ringBuffer, qRB_AUTOPOP, qLink);
    qSchedulerAddeTask(&Task4, Task4Callback, 10, "TASK4");
    qSchedulerAddeTask(&Task5, Task5Callback, 80, "TASK5");
    qSchedulerAddeTask(&Task6, Task6Callback, 10, "TASK6");
    qSchedulerAddSMTask(&SMTask, qHigh_Priority, 0.1, &statemachine, firststate, NULL, NULL, NULL, NULL, qEnabled, "smtask");
    /*qISR_Byte_t DataAlloc[100] = {0};
    qISR_ByteBuffer_t Buffer;
    qISR_ByteBufferInit(&Buffer, DataAlloc, sizeof(DataAlloc), '\r', isalpha, tolower);*/
    
    #ifdef Q_TASK_DEV_TEST
        qSchedulePrintChain();
    #endif
        printf("%p %p\r\n\r\n",firststate, secondstate);
    qSchedulerRun();
    
    return (EXIT_SUCCESS);
}

