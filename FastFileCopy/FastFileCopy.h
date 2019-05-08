// FastFileCopy.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFastFileCopyApp:
// See FastFileCopy.cpp for the implementation of this class
//

class CFastFileCopyApp : public CWinApp
{
public:
	CFastFileCopyApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CFastFileCopyApp theApp;

#include "afxmt.h"
#define THREAD_COUNT 5

class LockClass
{
public:
	LockClass(CSemaphore *sem)
	{
		m_sem = sem;
		if(m_sem)
			m_sem->Lock();
	}

	~LockClass()
	{
		if(m_sem)
			m_sem->Unlock();
	}

	CSemaphore *m_sem;
};

class GlobalData
{
public:
	GlobalData()
		: m_lock(1)
	{
		m_shouldrun = true;
		m_running = false;
		m_threadcount = 0;
		m_copysuccess = true;
		m_lasterror = _T("");
		m_source = _T("");
		m_dest = _T("");

		int i = 0;
		while(i < THREAD_COUNT)
		{
			m_downloadstatus[i] = 0;
			i++;
		}

		DWORD len = GetTempPath(MAX_PATH, m_tempPath);
		if((len == 0) || (len > MAX_PATH))
		{
			StringCchCopy(m_tempPath, MAX_PATH, _T("c:\\"));
		}
	}

	void SetShouldRun(bool val) { LockClass lk(&m_lock); m_shouldrun = val; }
	void SetRunning(bool val) { LockClass lk(&m_lock); m_running = val; }
	void SetCopySuccess(bool val) { LockClass lk(&m_lock); m_copysuccess = val; }
	void SetLastError(CString val) { LockClass lk(&m_lock); m_lasterror = val; }
	void AddLastError(CString val) { LockClass lk(&m_lock); m_lasterror += val; }
	void SetSource(CString val) { LockClass lk(&m_lock); m_source = val; }
	void SetDest(CString val) { LockClass lk(&m_lock); m_dest = val; }
	void SetThreadCount(UINT val) { LockClass lk(&m_lock); m_threadcount = val; }
	void IncThreadCount() { LockClass lk(&m_lock); m_threadcount++; }
	void DecThreadCount() { LockClass lk(&m_lock); m_threadcount--; }

	bool GetShouldRun() { LockClass lk(&m_lock); return m_shouldrun; }
	bool GetRunning() { LockClass lk(&m_lock); return m_running; }
	bool GetCopySuccess() { LockClass lk(&m_lock); return m_copysuccess; }
	CString GetLastError() { LockClass lk(&m_lock); return m_lasterror; }
	CString GetSource() { LockClass lk(&m_lock); return m_source; }
	CString GetDest() { LockClass lk(&m_lock); return m_dest; }
	UINT GetThreadCount() { LockClass lk(&m_lock); return m_threadcount; }

	TCHAR m_tempPath[MAX_PATH];
	UINT m_downloadstatus[THREAD_COUNT];

private:
	CSemaphore m_lock;

	//for FireCopy
	bool m_shouldrun;
	bool m_running;

	//for DoCopy
	UINT m_threadcount;
	bool m_copysuccess;
	CString m_lasterror;

	//data
	CString m_source;
	CString m_dest;
};

extern GlobalData g_data;