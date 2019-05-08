#include "stdafx.h"
#include "FastFileCopy.h"

GlobalData g_data;

struct ThreadData
{
	ULONGLONG threadID;
	ULONGLONG start;
	ULONGLONG size;
};


bool MergeTempFiles()
{
	CFile src;
	CFile final_dest;
	CFileException file_err;
	if(!final_dest.Open(g_data.GetDest(), CFile::modeCreate | CFile::modeWrite, &file_err))
	{
		g_data.AddLastError(_T("Error opening/creating destination file.\r\n"));
		return false;
	}

	UINT buf_size = 1024 * 1024; //1MB
	char *buf = new char[buf_size]; 

	bool error = false;
	ULONGLONG i = 1;
	while(i <= THREAD_COUNT)
	{
		CString src_filename;
		src_filename.Format(_T("\\fastfilecopy_temp_%d.tmp"), i);
		src_filename = g_data.m_tempPath + src_filename;

		if(!src.Open(src_filename, CFile::modeRead, &file_err))
		{
			g_data.AddLastError(_T("Error opening temporary downloaded file'") + src_filename + _T("'.\r\n"));
			error = true;
			goto Cleanup;
		}

		UINT bytes_read = 0;
		while(true)
		{
			try
			{
				bytes_read = src.Read(buf, buf_size);
				final_dest.Write(buf, bytes_read);
			}
			catch(CFileException *)
			{
				g_data.AddLastError(_T("Error in writing to the destination file.\r\n"));
				error = true;
				goto Cleanup;
			}

			if(bytes_read < buf_size)
				break;
		}

		src.Close();
		i++;
	}

Cleanup:
	if(src.m_hFile && (src.m_hFile != INVALID_HANDLE_VALUE))
		src.Close();

	if(final_dest.m_hFile && (final_dest.m_hFile != INVALID_HANDLE_VALUE))
		final_dest.Close();

	delete buf;

	i = 1;
	while(i <= THREAD_COUNT)
	{
		CString src_filename;
		src_filename.Format(_T("\\fastfilecopy_temp_%d.tmp"), i);
		src_filename = g_data.m_tempPath + src_filename;

		try
		{
			CFile::Remove(src_filename);
		}
		catch(CFileException*)
		{}

		i++;
	}

	return (!error);
}

static UINT __cdecl DoCopy(LPVOID pParam)
{
	ThreadData *data = (ThreadData *)pParam;
	if(!data)
		return 1;

	g_data.IncThreadCount();

	bool error = false;

	CString dest_filename;
	dest_filename.Format(_T("\\fastfilecopy_temp_%d.tmp"), data->threadID);
	dest_filename = g_data.m_tempPath + dest_filename;

	CFile src_file;
	CFile dest_file;
	CFileException file_err;
	UINT buf_size = 10 * 1024; //10KB
	char *buf = new char[buf_size];
	if(!buf)
	{
		g_data.AddLastError(_T("Error - Out of memory.\r\n"));
		error = true;
		goto Cleanup;
	}

	if(!src_file.Open(g_data.GetSource(), CFile::modeRead | CFile::shareDenyNone, &file_err))
	{
		g_data.AddLastError(_T("Error opening source file.\r\n"));
		error = true;
		goto Cleanup;
	}

	try
	{
		src_file.Seek(data->start, CFile::begin);
	}
	catch(CFileException*)
	{
		g_data.AddLastError(_T("Error jumping to specific location in source file.\r\n"));
		error = true;
		goto Cleanup;
	}

	if(!dest_file.Open(dest_filename, CFile::modeCreate | CFile::modeWrite, &file_err))
	{
		g_data.AddLastError(_T("Error opening/creating temporary download file '") + dest_filename + _T("'.\r\n"));
		error = true;
		goto Cleanup;
	}

	ULONGLONG total_read = 0;
	ULONGLONG total_size = data->size;
	while(data->size > 0)
	{
		if(!g_data.GetShouldRun())
		{
			error = true;
			goto Cleanup;
		}

		UINT bytes_read = 0;
		try
		{
			bytes_read = src_file.Read(buf, (UINT)(min(buf_size, data->size)));
			dest_file.Write(buf, bytes_read);

			total_read += (ULONGLONG)bytes_read;
			g_data.m_downloadstatus[data->threadID - 1] = (UINT)((total_read * 100) / total_size);
		}
		catch(CFileException*)
		{
			g_data.AddLastError(_T("Error writing to temporary download file '") + dest_filename + _T("'.\r\n"));
			error = true;
			goto Cleanup;
		}

		if(bytes_read < min(buf_size, data->size))
		{
			//reached the end
			break;
		}
		else
		{
			data->size -= bytes_read;
		}
	}

Cleanup:
	if(dest_file.m_hFile && (dest_file.m_hFile != INVALID_HANDLE_VALUE))
		dest_file.Close();

	if(src_file.m_hFile && (src_file.m_hFile != INVALID_HANDLE_VALUE))
		src_file.Close();

	delete buf;

	if(error)
	{
		g_data.SetShouldRun(false);
		g_data.SetCopySuccess(false);
		try
		{
			CFile::Remove(dest_filename);
		}
		catch(CFileException*)
		{}
	}

	if(g_data.GetThreadCount() == 1) // i am the only one left alive
	{
		if(g_data.GetCopySuccess())
		{
			if(MergeTempFiles())
			{
				MessageBox(NULL, _T("Done Copy."), _T(""), MB_OK);
			}
			else
			{
				g_data.AddLastError(_T("Error in merging temporary files.\r\n"));
				error = true;
				MessageBox(NULL, g_data.GetLastError(), _T("Error"), MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			error = true;
			MessageBox(NULL, g_data.GetLastError(), _T("Error"), MB_OK | MB_ICONERROR);
		}

		int i = 0;
		while(i < THREAD_COUNT)
		{
			g_data.m_downloadstatus[i] = 0;
			i++;
		}

		g_data.SetRunning(false);
	}

	g_data.DecThreadCount();
	return error;
}

void FireCopy(CString source, CString dest)
{
	if(g_data.GetRunning() || source.IsEmpty() || dest.IsEmpty())
		return;

	g_data.SetRunning(true);
	g_data.SetShouldRun(true);

	g_data.SetSource(source);
	g_data.SetDest(dest);

	g_data.SetThreadCount(0);
	g_data.SetCopySuccess(true);
	g_data.SetLastError(_T(""));

	ULONGLONG filesize = 0;

	try
	{
		CFile src_file(source, CFile::modeRead);
		filesize = src_file.GetLength();
		src_file.Close();
	}
	catch(CFileException *removeerror)
	{
		g_data.SetShouldRun(false); //kill all started threads

		CString errstr;
		errstr.Format(_T("Error getting source file size : #%d."), removeerror->m_cause);
		MessageBox(NULL, errstr, _T("Error"), MB_OK | MB_ICONERROR);

		g_data.SetRunning(false);
		return;
	}

	try
	{
		CFile dest_file(dest, CFile::modeCreate | CFile::modeWrite);
		dest_file.Close();
	}
	catch(CFileException *removeerror)
	{
		g_data.SetShouldRun(false); //kill all started threads

		CString errstr;
		errstr.Format(_T("Error creating destination file : #%d."), removeerror->m_cause);
		MessageBox(NULL, errstr, _T("Error"), MB_OK | MB_ICONERROR);

		g_data.SetRunning(false);
		return;
	}

	ULONGLONG int_size = filesize / THREAD_COUNT;
	ULONGLONG last_size = filesize - (int_size * (THREAD_COUNT - 1));

	ULONGLONG i = 1;
	ULONGLONG start = 0;
	while(i <= THREAD_COUNT)
	{
		ThreadData *data = new ThreadData;
		if(!data)
		{
			g_data.AddLastError(_T("Error - Out of memory.\r\n"));
			goto Cleanup;
		}

		data->threadID = i;
		data->start = start;

		if(i == THREAD_COUNT) //last thread
			data->size = last_size;
		else
			data->size = int_size;

		i++;
		start = start + int_size;
		//HANDLE thd = CreateThread(NULL, 0, DoCopy, NULL, 0, NULL);
		CWinThread *thd = AfxBeginThread(DoCopy, (LPVOID)data);
		if(!thd)
		{
			g_data.AddLastError(_T("Error in starting thread.\r\n"));
			goto Cleanup;
		}
	}

Cleanup:
	if(!g_data.GetLastError().IsEmpty())
	{
		g_data.SetShouldRun(false); //kill all started threads
		MessageBox(NULL, g_data.GetLastError(), _T("Error"), MB_OK | MB_ICONERROR);
		g_data.SetRunning(false);
	}
}