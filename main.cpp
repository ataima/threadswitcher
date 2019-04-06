/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: angelo
 *
 * Created on 28 marzo 2019, 14.11
 */
#include <stdio.h>
#include <pthread.h>
#include "Thread.h"
#include "ThreadClient.h"
#include "ThreadSimple.h"
#include "ThreadManager.h"
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <time.h> 


static struct timespec start, finish;
#define MAX_CLIENT 100
static unsigned int counters[MAX_CLIENT];

unsigned long int getelapsedtime(void) {
    long seconds = finish.tv_sec - start.tv_sec;
    long ns = finish.tv_nsec - start.tv_nsec;

    if (start.tv_nsec > finish.tv_nsec) { // clock underflow 
        --seconds;
        ns += 1000000000;
    }
    unsigned long int result = (seconds * 1000000000) + ns;
    return result;
}

void * func_client(void *param) {
    int index = (int) (unsigned long int) (param);
    clock_gettime(CLOCK_REALTIME, &finish);
    unsigned long int total = getelapsedtime();
    if (index < MAX_CLIENT) {
        std::cout << total << " ) >>exec func client id=" << index << " (" << counters[index] <<
                ")" << std::endl;
        counters[index]++;
    } else
        std::cout << total << " ) >>exec func client id=" << index << " HUH" << std::endl;
    return nullptr;
}

static int curr_task = 0;

void * func_scheduler(void *param) {
    void * res = nullptr;
    caThreadManager *manager = (caThreadManager *) param;
    if (manager != nullptr) {
        // std::cout<<"request next task"<<std::endl;
        manager->NextTask();
    }
    return res;
}

int main(void) {
    int i, c, average, mintick, maxtick;
    char names[64];
    caThreadManager manager;
    for (i = 0; i < MAX_CLIENT; i++) {
        sprintf(names, "exec client %d", i);
        counters[i] = 0;
        manager.AddClient(func_client, (void *) (unsigned long int)i,i,i, names);
    }
    manager.InitThread(func_scheduler, &manager);
    clock_gettime(CLOCK_REALTIME, &start);
    manager.SetSchedulerMode(caSchedulerPriorityMode::MORE_INCR);
    sleep(1);
    manager.StartScheduler();
    sleep(5);
    manager.StopScheduler();
    sleep(1);
    manager.StartScheduler();
    sleep(5);
    manager.StopScheduler();
    manager.StopAllClients();
    manager.ReqExit();
    mintick = 10000000;
    maxtick = average = 0;
    for (i = 0; i < MAX_CLIENT; i++) {
        std::cerr << "COUNTER[" << i << "]=" << counters[i] << std::endl;
        average += counters[i];
        if (counters[i] < mintick) {
            mintick = counters[i];
        }
        if (counters[i] > maxtick) {
            maxtick = counters[i];
        }
    }
    std::cerr << "TOTAL TICK CLIENTS: " << average << std::endl;
    average /= MAX_CLIENT;
    std::cerr << "AVERAGE TICK X CLIENTS: " << average << " ( " << 10000.0 / average << " ms )" << std::endl;
    std::cerr << "MIN TICK  CLIENT: " << mintick << std::endl;
    std::cerr << "MAX TICK  CLIENT: " << maxtick << std::endl;

    return 0;
}

