#include"stdafx.h"
#include <OYM_log.h>
/*
* OYM_Log is used to print log to a file, which use timestamp as file name.
* other modules create a instant of OYM_Log by new OYM_Log(modual_name, length_of_modual_name);
* then the log will print with modual_name as a tag.
*/

#ifdef WIN32

#else
#include<sys/time.h>
#endif

OYM_VOID OYM_Log::GetFileName(OYM_PCHAR filename)
{
#ifdef WIN32
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf_s((char*)filename, LOG_LOG_FILE_NAME_LENGTH, "%02d_%02d_%02d_%03d.txt", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
#else
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	time_t t;
	
	gettimeofday(&tv, &tz);
	time(&t);
	tm = localtime(&t);
	
	offset = sprintf(mFileName, "%02d:%02d:%02d.%03d.txt", tm->tm_hour, tm->tm_min, tm->tm_sec, int(tv.tv_usec / 1000));
	total += offset;
#endif
}

OYM_Log::OYM_Log(OYM_CPCHAR modual_name, OYM_UINT8 length)
{
	//mTag = (OYM_PCHAR)new char(length);
	//memcpy(mTag, modual_name, length);

	mLogLevel = LOG_LEVEL_OFF;
	mReference++;
	if (mLogLevel != LOG_LEVEL_OFF)
	{
		if (mReference == 1)
		{
			GetFileName(mFileName);
#ifdef WIN32
			fopen_s(&mLogFile, (const char*)mFileName, "wt+");
			if (mLogFile == NULL)
			{
				printf("open file failed!!!");
			}

			//fopen_s(&mLogFile, (const char*)"log.txt", "wt+");
#else
			mLogFile = fopen(mFileName, "wt+");
#endif
		}
	}

}

OYM_Log::~OYM_Log()
{
	printf("\n --------OYM_Log decontractor-------\n");
	mReference--;
	if (mReference == 0)
	{
		fclose(mLogFile);
	}
	delete mTag;
}
void OYM_Log::SetLogLevel(OYM_UINT8 logLevel)
{
	if (logLevel >= LOG_LEVEL_OFF)
		logLevel = LOG_LEVEL_OFF;

	mLogLevel = logLevel;
}

OYM_INT OYM_Log::PrintLog(OYM_UINT8 loglevel,OYM_CPCHAR fmt, ...)
{
	va_list argp;
	OYM_INT offset = 0;
	OYM_INT total = 0;
	if (mLogLevel > loglevel)
	{
		return total;
	}

	//get time of log
#ifdef WIN32
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	offset = sprintf_s((char*)mBuffer, LOG_BUFFER_LENGTH, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	total += offset;
#else
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	time_t t;
	
	gettimeofday(&tv, &tz);
	time(&t);
	tm = localtime(&t);
	
	offset = sprintf(mBuffer, "%02d:%02d:%02d.%03d", tm->tm_hour, tm->tm_min, tm->tm_sec, int(tv.tv_usec / 1000));
	total += offset;
#endif

	if (loglevel == LOG_LEVEL_DEBUG)
	{
#ifdef WIN32
		offset = sprintf_s((char*)mBuffer + offset, LOG_BUFFER_LENGTH-total, "[Debug]/%s:", mTag);
#else
		offset = sprintf((char*)mBuffer + offset, "[Debug]/%s:", modual);
#endif
	} 
	else if (loglevel == LOG_LEVEL_WARNING)
	{
#ifdef WIN32
		offset = sprintf_s((char*)mBuffer + offset, LOG_BUFFER_LENGTH - total, "[Warning]/%s:", mTag);
#else
		offset = sprintf((char*)mBuffer + offset, "[Debug]/%s:", modual);
#endif
	}
	else if (loglevel == LOG_LEVEL_ERROR)
	{
#ifdef WIN32
		offset = sprintf_s((char*)mBuffer + offset, LOG_BUFFER_LENGTH - total, "[Error]/%s:", mTag);
#else
		offset = sprintf((char*)mBuffer + offset, "[Debug]/%s:", modual);
#endif
	}
	total += offset;
	
	va_start(argp, fmt);
#ifdef WIN32
	offset = vsprintf_s((char*)mBuffer + total, LOG_BUFFER_LENGTH - total, (const char*)fmt, argp);
#else
	offset = vsnprintf((char*)mBuffer + total, LOG_BUFFER_LENGTH - total, fmt, argp);
#endif
	va_end(argp);

	mBuffer[offset + total] = '\n';

	fwrite(mBuffer, offset + total, 1, mLogFile);
	
	return offset + total + 1;
}

