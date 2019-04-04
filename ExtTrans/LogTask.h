#pragma once
#ifndef __LOGTASK_H__  
#define __LOGTASK_H__ 
#include "TaskBase.h"
#include <stdarg.h>
#include <queue>
#include <string>
#include <PROCESS.H>  

using namespace std;
#define LEVEL_ERROR 1
#define LEVEL_INFOR 2
#define LEVEL_DEBUG 3

class LogTask : public TaskBase
{
public:
				static LogTask *GetInstance();
				static void RemoveInstance();
				void SetLogLevel(int level);
				virtual void svc();
    virtual void stop();
				static void LOG_PRINT(int level, char* cFormat, ...);

				~LogTask();

private:
				LogTask();
				LogTask(const LogTask&);
				LogTask& operator=(const LogTask&);

				int PushLog(int level, char* cFormat, va_list vlist);

private:
				static LogTask *m_Task;
				int m_level;
				queue<string> m_LogQue;
				CRITICAL_SECTION m_section;
				HANDLE m_event;
				FILE* m_fLog;
};

#endif