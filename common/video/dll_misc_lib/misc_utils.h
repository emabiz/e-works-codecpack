#ifndef MISC_UTILS_H_
#define MISC_UTILS_H_

namespace ew {
		
	//! Splits a string at delimiter
	/*!
	  \param[in] src source string
	  \param[in] delim character to use as the delimiter
	  \return vector of splitted strings
	*/
	std::vector<std::string> Split(const std::string &src, char delim);

	//! Splits a string in two parts (name, value) at delimiter
	/*!
	  \param[in] src source string
	  \param[in] delim character to use as the delimiter
	  \param[out] name part before delimiter
	  \param[out] value part after delimiter
	  \return \a true if delimiter has been found (the split was successful)
	*/
	bool SplitKeyValue(const std::string &src, char delim, std::string *name, std::string *value);

	//! Trims spaces at string beginning and end
	/*!
	  \param[in] src source string
	  \return trimmed string
	*/
	std::string Trim(const std::string &src);
	
	//! Gets number of processor cores on computer, as reported by operating system
	/*!
	  \return processor cores count
	*/
	int GetProcessorCount();

} // namespace ew

#endif // ! MISC_UTILS_H_