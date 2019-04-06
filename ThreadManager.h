/* 
 * File:   ThreanManager.h
 * Author: angelo
 *
 * Created on 28 marzo 2019, 16.28
 */

#ifndef THREANMANAGER_H
#define THREANMANAGER_H


#include <vector>
#include "Thread.h"
#include "ThreadClient.h"


class caThreadManager;

typedef std::vector<caThreadClient *> thArray;
typedef int (*functor_nexttask)(caThreadManager *instance);

class caThreadManager
: public caThreadClient {
private:
    int last_active;
    thArray clients;
    static pthread_mutex_t mMtxClients;
    static caThreadManager *instance;
    functor_nexttask getnetxtask;

    static int PriorityHigh(caThreadManager *mng);

    int getNextPriorityHigh(void);
public:
    caThreadManager();
    ~caThreadManager();
    bool InitThread(functor entry, void *param);
    bool Run(size_t index);
    bool AddClient(functor func, void *param, int priority, int index, const char *name);
    int GetClientsSize(void);
    void StartScheduler(void);
    void StopScheduler(void);
    void NextTask(void);
    void StopAllClients(void);
    void SetSchedulerMode(caSchedulerPriorityMode mode);

    inline void SetUserFunctor(functor_nexttask select) {
        getnetxtask = select;

    }
    inline static caThreadManager * getInstance(void){
        return instance;
    }
            
            
};



#endif /* THREANMANAGER_H */

