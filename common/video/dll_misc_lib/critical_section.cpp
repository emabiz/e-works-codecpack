#include "stdafx.h"
#include "critical_section.h"

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// CriticalSection
	//////////////////////////////////////////////////////////////////////

	CriticalSection::CriticalSection()
	{
		InitializeCriticalSection(&critical_section_win32_);
	}

	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(&critical_section_win32_);
	}

	void CriticalSection::Enter()
	{
		EnterCriticalSection(&critical_section_win32_);
	}

	void CriticalSection::Leave()
	{
		LeaveCriticalSection(&critical_section_win32_);
	}


	//////////////////////////////////////////////////////////////////////
	// CriticalSectionLocker
	//////////////////////////////////////////////////////////////////////

	CriticalSectionLocker::CriticalSectionLocker(CriticalSection &critical_section)
		: critical_section_(critical_section)
	{
		critical_section_.Enter();
	}

	CriticalSectionLocker::~CriticalSectionLocker()
	{
		critical_section_.Leave();
	}

} // namespace ew
