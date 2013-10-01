#ifndef CRITICAL_SECTION_H_
#define CRITICAL_SECTION_H_

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// CriticalSection
	//////////////////////////////////////////////////////////////////////

	class CriticalSection
	{
	public:
		CriticalSection();
		~CriticalSection();
		void Enter();
		void Leave();
	private:
		CRITICAL_SECTION critical_section_win32_;
	};


	//////////////////////////////////////////////////////////////////////
	// CriticalSectionLocker
	//////////////////////////////////////////////////////////////////////

	class CriticalSectionLocker
	{
	public:
		CriticalSectionLocker(CriticalSection &critical_section);
		~CriticalSectionLocker();
	private:
		CriticalSection &critical_section_;
		CriticalSectionLocker operator =(const CriticalSectionLocker &other);
	};

} // namespace ew

#endif // ! CRITICAL_SECTION_H_