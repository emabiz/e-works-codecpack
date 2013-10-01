#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include "critical_section.h"

namespace ew {

	class FileLogger
	{
	public:
		enum Level {
			kLevelNoLog,
			kLevelError,
			kLevelWarning,
			kLevelInfo,
			kLevelDebug,

			kLevelCount
		};

		FileLogger();
		~FileLogger();
		
		bool Init(Level log_level, const std::string &basename);
		bool Log(Level info_level, const char *fmt, va_list args);
		Level log_level() const { return log_level_; }

	private:
		FILE *fd_;
		Level log_level_;
		CriticalSection critical_section_;
	};

} // namespace ew

#endif // ! FILE_LOGGER_H_