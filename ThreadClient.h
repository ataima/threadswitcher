/* 
 * File:   ThreadClient.h
 * Author: angelo
 *
 * Created on 28 marzo 2019, 14.11
 */

#ifndef THREADCLIENT_H
#define THREADCLIENT_H


#include "Thread.h"

class caThreadClient {
protected:
    caThreadStatus mStatus;
    caThreadMode mMode;
    pthread_t *mThid;
    static pthread_mutex_t mMtx;
    pthread_cond_t mCond;
    functor reqFunc;
    int mPriority;
    int mCurPriority;
    int mIndex;
    unsigned long int mTickCount;
    void *reqParam;
    char mName[32];


    bool CreateThread();
    int WaitForSignal(void);
    void * ExecuteClient(void);
    int Lock(void);
    int Unlock(void);
    int CondWait(void);
    int CondSignal(void);

    void DestroyThread(void);
    void JoinThread(void);



public:
    caThreadClient(int priority = 0, int index = 0);
    ~caThreadClient();
    bool InitThread(functor entry, void *param, const char *name);
    void SleepThread(unsigned int delay);
    void Resume(void);
    void ReqExit(void);

    inline caThreadStatus getStatus(void) {
        return mStatus;
    }

    inline void setStatus(caThreadStatus m) {
        mStatus = m;
    }

    inline caThreadMode getMode(void) {
        return mMode;
    }

    inline pthread_t * getThreadId(void) {
        return mThid;
    }

    inline const char *getName(void) {
        return mName;
    }

    inline int getPriority(void) {
        return mPriority;
    }

    inline int getCurPriority(void) {
        return mCurPriority;
    }

    inline int getTickCount(void) {
        return mTickCount;
    }

    inline int getIndex(void) {
        return mIndex;
    }

    inline void updateCurPriority(void) {
        mCurPriority--;
        if (mCurPriority < 0)
            mCurPriority = mPriority;
    }


public:
    static void * entry_point(void *param);
    static void cleanup_point(void *param);

};

#endif /* THREADCLIENT_H */

