#pragma once
#ifndef __IRDACLIENT_H__  
#define __IRDACLIENT_H__  
#include "TaskBase.h"


class IrdaClient : public TaskBase
	{
	public:
		static IrdaClient * GetInstance();
		static void RemoveInstance();
		~IrdaClient();
		int ClientInit();
		virtual void svc();
  virtual void stop();
		int SendData(char* buf, int size);
		int Connect();

	private:
		IrdaClient();
		int ClientStart();
		void LCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen);
		void NCPirda2com(byte *descbuf, byte *srcbuf, int& deslen, int srclen);

	private:
		SOCKET m_sock;
		static IrdaClient *m_IClient;
		WSADATA m_wsaData;    // 这结构是用于接收Wjndows Socket的结构信息的  
		bool m_IsConnect;
	};

#endif
