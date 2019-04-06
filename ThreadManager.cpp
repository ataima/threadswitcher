/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <pthread.h>
#include "ThreadClient.h"
#include "ThreadManager.h"
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <set>

pthread_mutex_t caThreadManager::mMtxClients = PTHREAD_MUTEX_INITIALIZER;
caThreadManager *caThreadManager::instance = nullptr;

caThreadManager::caThreadManager() {
    HERE1();
    last_active = -1;
    clients.clear();
    getnetxtask = nullptr;
    if (instance != nullptr) {
        std::cerr << "caThreadManager::instance already set !!" << std::endl;
    }
    instance = this;
}

caThreadManager::~caThreadManager() {
    HERE1();
    StopAllClients();
    pthread_mutex_destroy(&mMtxClients);
}

void caThreadManager::StopAllClients(void) {
    pthread_mutex_lock(&mMtxClients);
    thArray::iterator it = clients.begin();
    thArray::iterator _end = clients.end();
    while (it != _end) {
        if (*it != nullptr) {
            (*it)->ReqExit();
            delete *it;
            *it = nullptr;
        }
        it++;
    }
    clients.clear();
    pthread_mutex_unlock(&mMtxClients);
}

bool caThreadManager::Run(size_t index) {
    HERE1();
    bool result = true;
    pthread_mutex_lock(&mMtxClients);
    size_t size = clients.size();
    if (index < size) {
        caThreadClient * clientThread = clients.at(index);
        if (clientThread != nullptr) {
            if (clientThread->getStatus() == caThreadStatus::WAIT_SIGNAL) {
                clientThread->Resume();
                result = !(clientThread->getStatus() == caThreadStatus::RESUME ||
                        clientThread->getStatus() == caThreadStatus::RUNNING);
                if (result == false)
                    last_active = index;
            }
        }
    }
    pthread_mutex_unlock(&mMtxClients);
    return result;
}

bool caThreadManager::InitThread(functor entry, void *param) {
    HERE1();
    bool result = false;
    mMode = caThreadMode::WAIT_ONLY_START;
    result = caThreadClient::InitThread(entry, param, "Manager");
    while (getStatus() != caThreadStatus::WAIT_SIGNAL) {
        usleep(1000);
    }
    return result;
}

int caThreadManager::GetClientsSize(void) {
    HERE1();
    int size = 0;
    pthread_mutex_lock(&mMtxClients);
    size = (int) clients.size();
    pthread_mutex_unlock(&mMtxClients);
    return size;
}

bool caThreadManager::AddClient(functor func, void *param, int priority, int index, const char *name) {
    HERE1();
    bool result = true;
    caThreadClient *client = new caThreadClient(priority, index);
    if (client != nullptr) {
        pthread_mutex_lock(&mMtxClients);
        clients.push_back(client);
        pthread_mutex_unlock(&mMtxClients);
        client->InitThread(func, param, name);
        while (client->getStatus() != caThreadStatus::WAIT_SIGNAL) {
            usleep(1000);
        }
        result = false;
    }
    return result;
}

void caThreadManager::StartScheduler(void) {
    HERE1();
    do {
        mMode = WAIT_ONLY_START;
    } while (getMode() != caThreadMode::WAIT_ONLY_START);
    Resume();
}

void caThreadManager::StopScheduler(void) {
    HERE1();
    do {
        mMode = WAIT_ALWAYS;
        usleep(1000);
    } while (getStatus() != caThreadStatus::WAIT_SIGNAL);
}

void caThreadManager::NextTask(void) {
    HERE1();
    int temp = -1;
    if (getnetxtask != nullptr) {
        temp = getnetxtask(this);
        if (temp != -1)
            last_active = temp;
    } else {
        last_active++;
    }
    if ((last_active > clients.size()) ||
            (last_active < 0)) {
        last_active = 0;
    }
    Run(last_active);

}

void caThreadManager::SetSchedulerMode(caSchedulerPriorityMode mode) {
    HERE1();
    if (mode == caSchedulerPriorityMode::ROUND_ROBIN) {
        getnetxtask = nullptr;
    } else
        if (mode == caSchedulerPriorityMode::MORE_INCR) {
        getnetxtask = caThreadManager::PriorityHigh;
    } else
        if (mode == caSchedulerPriorityMode::MODE_USER) {
    }
}

int caThreadManager::PriorityHigh(caThreadManager *mng) {
    HERE();
    if (mng == nullptr) {
        return -1;
    } else {
        return mng->getNextPriorityHigh();
    }
}

int caThreadManager::getNextPriorityHigh(void) {
    HERE1();

    struct thread_pri_index {
        int th_index;
        int th_priority;
        unsigned long int th_ticks;
    };

    struct thread_comp_incr {

        bool operator()(const thread_pri_index& lhs, const thread_pri_index& rhs) const {
            if (lhs.th_priority < rhs.th_priority) {
                return true;
            } else
                if (lhs.th_priority == rhs.th_priority) {
                return lhs.th_ticks > rhs.th_ticks;
            } else {
                return false;
            }
        }
    };
    pthread_mutex_lock(&mMtxClients);
    std::set<thread_pri_index, thread_comp_incr> sorted_thread;
    thArray::iterator it = clients.begin();
    thArray::iterator _end = clients.end();
    while (it != _end) {
        caThreadClient *tmp = *it;
        thread_pri_index id;
        id.th_index = tmp->getIndex();
        id.th_priority = tmp->getCurPriority();
        id.th_ticks = tmp->getTickCount();
        sorted_thread.insert(id);
        it++;
    }
    std::set<thread_pri_index, thread_comp_incr>::reverse_iterator rit = sorted_thread.rbegin();
    int target = rit->th_index;
    clients.at(target)->updateCurPriority();
    pthread_mutex_unlock(&mMtxClients);
    return target;
}

