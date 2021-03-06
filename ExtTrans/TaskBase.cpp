#include "stdafx.h"
#include <stdio.h>
#include "TaskBase.h"
#include <baseLib/logLib.h>

TaskBase::TaskBase()
{

}

TaskBase::~TaskBase()
{
    
}

void TaskBase::start()
{
				//printf("TaskBase::start\n");
				m_handle = CreateThread(NULL, 0, run, this, 0, NULL);
				if ( m_handle == NULL)
				{
        logMessage1(LOGLEVEL_DEBUG,_T("TaskBase::start CreateThread error:%d\n"), GetLastError());
				}
}

DWORD TaskBase::run(LPVOID pVoid)
{
				TaskBase *task = (TaskBase*)pVoid;
				task->svc();
				return 0;
}

void TaskBase::join()
{
				WaitForSingleObject(m_handle, INFINITE);
}
