#include "stdafx.h"

#include "param_parser.h"
#include "misc_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	//////////////////////////////////////////////////////////////////////
	// ParamDescrInt
	//////////////////////////////////////////////////////////////////////

	class ParamDescrInt : public ParamDescr
	{
	public:
		ParamDescrInt(const std::string &name, int value_default, int value_min, int value_max);
		virtual std::string GetValueDefaultString() const;
		virtual ParamValue GetValueDefault() const;
		virtual bool ConvertIfAllowed(const std::string &s, ParamValue *v) const;

	private:
		int value_default_;
		int value_min_;
		int value_max_;
	};

	ParamDescrInt::ParamDescrInt(const std::string &name, int value_default, int value_min, int value_max)
		:	ParamDescr(name, kParamValueTypeInt),
			value_default_(value_default),
			value_min_(value_min),
			value_max_(value_max)
	{
		_ASSERT(value_min <= value_default);
		_ASSERT(value_default <= value_max);
	}

	std::string ParamDescrInt::GetValueDefaultString() const
	{
		std::stringstream ss;
		ss << value_default_;
		return ss.str();
	}

	ParamValue ParamDescrInt::GetValueDefault() const
	{
		ParamValue v;
		v.i = value_default_;
		return v;
	}

	bool ParamDescrInt::ConvertIfAllowed(const std::string &s, ParamValue *v) const
	{
		if (s.empty())
			return false;

		const char *start = s.c_str();
		char *end = NULL;
		int n = static_cast<int>(strtol(start, &end, 10));
		if (static_cast<size_t>(end - start) != s.size())
			return false;

		if (n < value_min_ || n > value_max_)
			return false;
		
		if (NULL != v)
			v->i = n;
		return true;
	}

	//////////////////////////////////////////////////////////////////////
	// ParamDescrIntList
	//////////////////////////////////////////////////////////////////////

	class ParamDescrIntList : public ParamDescr
	{
	public:
		ParamDescrIntList(const std::string &name, int value_default,
			const int *value_ary, size_t size);
		virtual std::string GetValueDefaultString() const;
		virtual ParamValue GetValueDefault() const;
		virtual bool ConvertIfAllowed(const std::string &s, ParamValue *v) const;

	private:
		int value_default_;
		std::vector<int> value_allowed_vect_;

		bool IsIntAllowed(int value) const;
	};

	ParamDescrIntList::ParamDescrIntList(const std::string &name, int value_default,
			const int *value_ary, size_t size)
		:	ParamDescr(name, kParamValueTypeInt),
			value_default_(value_default)
	{
		_ASSERT(value_ary);
		value_allowed_vect_.resize(size);
		memcpy(&value_allowed_vect_[0], value_ary, size*sizeof(int));
		std::sort(value_allowed_vect_.begin(), value_allowed_vect_.end());
		if (!IsIntAllowed(value_default)) {
			_ASSERT(0);
			value_default_ = -1;
			value_allowed_vect_.clear();
		}
	}

	std::string ParamDescrIntList::GetValueDefaultString() const
	{
		std::stringstream ss;
		ss << value_default_;
		return ss.str();
	}

	ParamValue ParamDescrIntList::GetValueDefault() const
	{
		ParamValue v;
		v.i = value_default_;
		return v;
	}

	bool ParamDescrIntList::ConvertIfAllowed(const std::string &s, ParamValue *v) const
	{
		if (s.empty())
			return false;

		const char *start = s.c_str();
		char *end = NULL;
		int n = static_cast<int>(strtol(start, &end, 10));
		if (static_cast<size_t>(end - start) != s.size())
			return false;

		if (!IsIntAllowed(n))
			return false;

		if (NULL != v)
			v->i = n;

		return true;
	}

	bool ParamDescrIntList::IsIntAllowed(int value) const
	{
		std::vector<int>::const_iterator it = std::lower_bound(
			value_allowed_vect_.begin(), value_allowed_vect_.end(), value);

		return !(it == value_allowed_vect_.end() || *it != value);
	}

	//////////////////////////////////////////////////////////////////////
	// ParamDescr
	//////////////////////////////////////////////////////////////////////

	ParamDescr::ParamDescr(const std::string &name, ParamValueType value_type)
		: name_(name), value_type_(value_type)
	{
		std::transform(name_.begin(), name_.end(), name_.begin(), toupper);
	}

	ParamDescr::~ParamDescr()
	{
	}

	std::string ParamDescr::FormatDefault() const
	{
		std::stringstream ss;
		ss << name_ << "=" << GetValueDefaultString();
		return ss.str();
	}

	ParamDescr *ParamDescr::CreateParamDescrInt(const std::string &name, int value_default,
		int value_min, int value_max)
	{
		return new ParamDescrInt(name, value_default, value_min, value_max);
	}

	ParamDescr *ParamDescr::CreateParamDescrIntList(const std::string &name, int value_default,
			const int *value_ary, size_t size)
	{
		return new ParamDescrIntList(name, value_default, value_ary, size);
	}

#if 0
	ParamDescr *ParamDescr::CreateParamDescrDouble(const std::string &name, double value_default,
		double value_min, double value_max)
	{
		return NULL;
	}

	ParamDescr *ParamDescr::CreateParamDescrStringList(const std::string &name, const char *value_ary[],
			size_t size)
	{
		return NULL;
	}
#endif

	//////////////////////////////////////////////////////////////////////
	// ParamArchive
	//////////////////////////////////////////////////////////////////////

	ParamArchive::ParamArchive()
	{
	}

	ParamArchive::~ParamArchive()
	{
		_ASSERT(descr_vect_.size() == value_vect_.size());

#if 0
		for (size_t i = 0; i < descr_vect_.size(); ++i) {
			const ParamDescr *descr = descr_vect_[i];
			if (kParamValueTypeString == descr->value_type()) {
				delete value_vect_[i].s;
			}
		}
#endif
	}

	void ParamArchive::AppendDescr(const ParamDescr *descr)
	{
		_ASSERT(descr);
		descr_vect_.push_back(descr);
		
		ParamValue v = descr->GetValueDefault();
#if 0 // Encapsulate class copy
		if (kParamValueTypeString == descr->value_type()) {
			const char *s = v.s;
			_ASSERT(NULL != s);
			size_t len = strlen(s) + 1;
			v.s = new char[len];
			_VERIFY(0 == strcpy_s(v.s, len, s));
		}
#endif
		value_vect_.push_back(v);
	}

	std::string ParamArchive::GetParamsDefault() const
	{
		std::stringstream ss;
		for (std::vector<const ParamDescr *>::const_iterator it = descr_vect_.begin();
			it != descr_vect_.end(); ++it) {
				if (it != descr_vect_.begin())
					ss << ";";
				ss << (*it)->name() << "=" << (*it)->GetValueDefaultString();
		}

		return ss.str();
	}

	

	bool ParamArchive::Parse(const std::string &s)
	{
		std::vector<std::string> param_vect(Split(s, ';'));
		for (std::vector<std::string>::const_iterator it = param_vect.begin();
			it != param_vect.end(); ++it) {
				std::string name;
				std::string value;
				if (!SplitKeyValue(*it, '=', &name, &value))
					return false;
				const ParamDescr *descr = FindParamDescr(name);
				if (NULL == descr)
					return false;
				ParamValue v;
				if (!descr->ConvertIfAllowed(value, &v))
					return false;

				// WARN: this search is redundant
				int index = GetParamDescrIndex(descr);
				_ASSERT(index >= 0);
				value_vect_[index] = v;
		}
		return true;
	}

	int ParamArchive::GetValueInt(const ParamDescr *descr) const
	{
		_ASSERT(NULL != descr);
		_ASSERT(kParamValueTypeInt == descr->value_type());
		int index = GetParamDescrIndex(descr);
		_ASSERT(index >= 0);
		return value_vect_[index].i;
	}

	double ParamArchive::GetValueDouble(const ParamDescr *descr) const
	{
		_ASSERT(NULL != descr);
		_ASSERT(kParamValueTypeDouble == descr->value_type());
		int index = GetParamDescrIndex(descr);
		_ASSERT(index >= 0);
		return value_vect_[index].d;
	}

#if 0
	std::string ParamArchive::GetValueString(const ParamDescr *descr) const
	{
		_ASSERT(NULL != descr);
		_ASSERT(kParamValueTypeString == descr->value_type());
		int index = GetParamDescrIndex(descr);
		_ASSERT(index >= 0);
		return value_vect_[index].s;
	}
#endif

	int ParamArchive::GetParamDescrIndex(const ParamDescr *descr) const
	{
		_ASSERT(NULL != descr);
		for (size_t i = 0; i < descr_vect_.size(); ++i) {
			if (descr == descr_vect_[i])
				return i;
		}
		
		return -1;
	}

	const ParamDescr *ParamArchive::FindParamDescr(const std::string &name) const
	{
		std::string name_up(name);
		std::transform(name_up.begin(), name_up.end(), name_up.begin(), toupper);
		for (std::vector<const ParamDescr *>::const_iterator it = descr_vect_.begin();
			it != descr_vect_.end(); ++it) {
				if (name_up == (*it)->name()) {
					return *it;
				}
		}
		return NULL;
	}

	void ParamArchive::SetDefaults()
	{
		_ASSERT(descr_vect_.size() == value_vect_.size());
		for (size_t i = 0; i < descr_vect_.size(); ++i) {
			value_vect_[i] = descr_vect_[i]->GetValueDefault();
		}
	}

} // namespace ew