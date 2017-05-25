#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "QuarkTS.h"
#include <signal.h>

/*============================================================================*/
pthread_t TimerEmulation;
void* TimerInterruptEmulation(void* varargin){
    struct timespec tick={0, 0.01*1E9};
    for(;;){
        nanosleep(&tick, NULL);
        qISRHandler();
    }
}
/*============================================================================*/
qTask_t Task1, Task2, Task3, Task4, Task5, Task6, TaskTestST;

/*============================================================================*/
void Task1Callback(qEvent_t Data){
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);
   // qQueueEvent(Task1, "A");
    qQueueEvent(Task2, "A"); //20
    qQueueEvent(Task3, "B"); //50
    qQueueEvent(Task4, "C"); //10
    qQueueEvent(Task5, "D"); //80
    qQueueEvent(Task6, "E"); //10
    qQueueEvent(Task6, "F"); //10
    qQueueEvent(Task6, "G"); //10
    qQueueEvent(Task6, "H"); //10
    qQueueEvent(Task6, "I"); //10
    qQueueEvent(Task6, "J"); //10
    qQueueEvent(Task6, "K"); //10
}
/*============================================================================*/
void Task2Callback(qEvent_t Data){
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);
}
/*============================================================================*/
void Task3Callback(qEvent_t Data){
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);
}
/*============================================================================*/
void Task4Callback(qEvent_t Data){
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);
}
/*============================================================================*/
void Task5Callback(qEvent_t Data){
    qQueueEvent(Task6, "K"); //10
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);
}
/*============================================================================*/
void Task6Callback(qEvent_t Data){
    printf("Userdata : %s  Eventdata:%s\r\n", Data.UserData, Data.EventData);   
}
/*============================================================================*/
void IdleTaskCallback(qEvent_t Data){

}
/*============================================================================*/
void sigint_signal(int x){
    qQueueEvent(Task3, "X"); //10
    qQueueEvent(Task4, "Y"); //10
}
int main(int argc, char** argv) {
    //signal(SIGALRM, alarm_signal);
    signal(SIGINT, sigint_signal);
    pthread_create(&TimerEmulation, NULL, TimerInterruptEmulation, NULL );
    qSetup(0.01, IdleTaskCallback, 10);
    qCreateTask(Task1, Task1Callback, HIGH_Priority, 1, PERIODIC, ENABLE, "TASK1");
    qCreateEventTask(Task2, Task2Callback, 20, "TASK2");
    qCreateEventTask(Task3, Task3Callback, 50, "TASK3");
    qCreateEventTask(Task4, Task4Callback, 10, "TASK4");
    qCreateEventTask(Task5, Task5Callback, 80, "TASK5");
    qCreateEventTask(Task6, Task6Callback, 10, "TASK6");
    qSchedule();
    return (EXIT_SUCCESS);
}

