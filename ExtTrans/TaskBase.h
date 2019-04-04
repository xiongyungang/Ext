#pragma once
#ifndef __TASKBASE_H__  
#define __TASKBASE_H__ 
#include <windows.h>

class TaskBase
{
public:
    TaskBase();
    ~TaskBase();
    void start();
    virtual void svc() = 0;
    virtual void stop() = 0;
protected:
    HANDLE m_handle;
    HANDLE m_endEvent;
    static DWORD WINAPI run(LPVOID  pVoid);
    void join();
};

#endif
