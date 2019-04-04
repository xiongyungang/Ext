#include "stdafx.h"
#include "LogTask.h"
#include <sys\stat.h>

#define LOGFILESIZE 50000000 //50M

LogTask *LogTask::m_Task = NULL;

LogTask *LogTask::GetInstance()
{
				if (m_Task == NULL)
				{
								m_Task = new LogTask();
				}
				return m_Task;
}

void LogTask::RemoveInstance()
{
				if (m_Task != NULL)
				{
								delete m_Task;
				}
}

LogTask::LogTask()
{
				InitializeCriticalSection(&m_section);
				m_event = CreateEvent(NULL,true,false,NULL);
				m_level = LEVEL_ERROR;
				m_fLog = NULL;
    m_endEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

LogTask::LogTask(const LogTask&)
{

}

LogTask& LogTask::operator=(const LogTask&)
{
				return *m_Task;
}

LogTask::~LogTask()
{
				DeleteCriticalSection(&m_section);
				join();
}

//日志等级大于1，生成debuglog.txt
void LogTask::SetLogLevel(int level)
{
				m_level = level;

				if (m_level > 1)
				{
								char filepath[MAX_PATH+1] = {0};
								if (GetModuleFileName(NULL,filepath,MAX_PATH) == 0)
								{
												printf("LogTask GetModuleFileName err %d.\n",GetLastError());
								}
								else
								{
												if (strcmp(filepath, "") != 0)
												{
																string strpath = filepath;
																int pos = strpath.rfind("\\");
																strpath = strpath.substr(0,pos+1);
																strpath += "debuglog.txt";
																char mode[10];
																strcpy(mode, "a+");
																struct _stat info;
																if (_stat(strpath.c_str(), &info) == 0)
																{
																				if (info.st_size > LOGFILESIZE)
																				{
																								strcpy(mode, "w+");
																				}
																}
																m_fLog = fopen(strpath.c_str(),mode);
												}

								}

				}
}

void LogTask::LOG_PRINT(int level, char* cFormat, ...)
{
				va_list args = NULL;
				va_start(args, cFormat);
				m_Task->PushLog(level,cFormat,args);
				va_end(args);
}

int LogTask::PushLog(int level, char* cFormat, va_list vlist)
{
				if (level <= m_level)
				{
								char levelstr[20];
								switch(level)
								{
								case LEVEL_DEBUG:
												strcpy(levelstr,"DEBUG");
												break;
								case LEVEL_ERROR:
												strcpy(levelstr,"ERROR");
												break;
								case LEVEL_INFOR:
								default:
												strcpy(levelstr,"INFOR");
												break;
								}
								char logbuf[2048];

								SYSTEMTIME sysTime;
								GetLocalTime(&sysTime);
								sprintf(logbuf,"%4d/%02d/%02d %02d:%02d:%02d.%03d [%s]",sysTime.wYear,sysTime.wMonth,
												sysTime.wDay,sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds,levelstr);
								int lens = strlen(logbuf);
								vsprintf_s(logbuf+lens,2047-lens,cFormat,vlist);
								string LogString(logbuf);

								EnterCriticalSection(&m_section);
								m_LogQue.push(LogString);
								SetEvent(m_event);
								LeaveCriticalSection(&m_section);
				}
				return 0;
}

void LogTask::stop()
{
    if (m_endEvent != NULL)
    {
        SetEvent(m_endEvent);
    }
}

void LogTask::svc()
{
				while(WAIT_OBJECT_0 != WaitForSingleObject(m_endEvent,0))
				{
								WaitForSingleObject(m_event,INFINITE);
								string LogString;
								EnterCriticalSection(&m_section);
								if (!m_LogQue.empty())
								{
												LogString = m_LogQue.front();
												m_LogQue.pop();
								}
								if (m_LogQue.empty())
								{
												//设置事件为无状态
												ResetEvent(m_event);
								}
								LeaveCriticalSection(&m_section);
								printf("%s\n",LogString.c_str());
								if (m_fLog)
								{
												fprintf(m_fLog,"%s\n",LogString.c_str());
												fflush(m_fLog);
								}
				}
}