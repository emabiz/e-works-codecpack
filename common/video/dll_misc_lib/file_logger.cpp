#include "stdafx.h"
#include "file_logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	FileLogger::FileLogger()
		: fd_(NULL), log_level_(kLevelNoLog)
	{
	}

	FileLogger::~FileLogger()
	{
		if (NULL != fd_)
			fclose(fd_);
	}

	bool FileLogger::Init(Level log_level, const std::string &basename)
	{
		_ASSERT(0 <= log_level && log_level < kLevelCount);

		if (NULL != fd_)
			return false;
		log_level_ = log_level;
		
		if (log_level_ == kLevelNoLog)
			return true;
			
		std::stringstream ss;
		ss << basename << ".txt";
		
		if (0 != fopen_s(&fd_, ss.str().c_str(), "wt"))
			return false;

		return true;
	}

	static const char *g_level_name_ary[] = {
		"",
		"ERR",
		"WRN",
		"INF",
		"DBG",
		NULL
	};

	bool FileLogger::Log( Level info_level, const char *fmt, va_list args)
	{
		_ASSERT(0 <= info_level && info_level < kLevelCount);
		if (NULL == fd_ || kLevelNoLog == info_level)
			return true;

		time_t t = time(NULL);
		struct tm ltime;
		_VERIFY(0 == localtime_s(&ltime, &t));

		CriticalSectionLocker locker(critical_section_);

		int err = fprintf_s(fd_, "%04d%02d%02d %02d%02d%02d %s ",
			ltime.tm_year+1900, ltime.tm_mon+1, ltime.tm_mday,
			ltime.tm_hour, ltime.tm_min, ltime.tm_sec,
			g_level_name_ary[info_level]);
		if (err < 0)
			return false;
		err = vfprintf_s(fd_, fmt, args);
		if (EOF == err)
			return false;
		if (EOF == fprintf_s(fd_, "\n"))
			return false;
		if (EOF == fflush(fd_))
			return false;

		return true;
	}

} // namespace ew