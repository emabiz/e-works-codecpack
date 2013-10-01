#include "stdafx.h"
#include "misc_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	std::vector<std::string> Split(const std::string &src, char delim)
	{
		_ASSERT(delim != '\0');
		std::vector<std::string> vect;
		size_t last_pos = 0;
		size_t pos = src.find(delim, last_pos);
		while (std::string::npos != pos) {
			std::string s(&src[last_pos], &src[pos]);
			s = Trim(s);
			if (s.size() > 0)
				vect.push_back(s);
			last_pos = pos+1;
			pos = src.find(delim, last_pos);
		}

		if (last_pos != src.size()) {
			std::string s(&src[last_pos], &src[src.size()]);
			s = Trim(s);
			if (s.size() > 0)
				vect.push_back(s);
		}

		return vect;
	}

	bool SplitKeyValue(const std::string &src, char delim, std::string *name, std::string *value)
	{
		_ASSERT(NULL != name);
		_ASSERT(NULL != value);
		_ASSERT(delim != '\0');

		size_t pos = src.find(delim);
		if (std::string::npos == pos)
			return false;

		std::string s(&src[0], &src[pos]);
		s = Trim(s);
		if (s.size() == 0)
			return false;
		name->assign(s);

		s.assign(&src[pos+1], &src[src.size()]);
		s = Trim(s);
		if (s.size() == 0)
			return false;
		value->assign(s);
		return true;
	}

	std::string Trim(const std::string &src)
	{
		const char t = ' ';
		size_t left = src.find_first_not_of(t);
		if (std::string::npos == left)
			return "";
		size_t right = src.find_last_not_of(t);
		_ASSERT(std::string::npos != right);
		return std::string(&src[left], &src[right+1]);
	}

	int GetProcessorCount()
	{
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);
		return siSysInfo.dwNumberOfProcessors; /* number of _logical_ cores */
	}

} // namespace ew