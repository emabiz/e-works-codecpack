#ifndef PARAM_PARSER_H_
#define PARAM_PARSER_H_

namespace ew {

	enum ParamValueType {
		kParamValueTypeInt,
		kParamValueTypeDouble,
		//kParamValueTypeString
	};

	union ParamValue {
		int i;
		double d;
		//char *s;
	};

	//////////////////////////////////////////////////////////////////////
	// ParamDescr
	//////////////////////////////////////////////////////////////////////

	class ParamDescr
	{
	public:
		virtual ~ParamDescr();
		virtual std::string GetValueDefaultString() const = 0;
		virtual ParamValue GetValueDefault() const = 0;
		virtual bool ConvertIfAllowed(const std::string &s, ParamValue *v) const = 0;

		std::string name() const { return name_; }
		ParamValueType value_type() const { return value_type_; }
		std::string FormatDefault() const;

		static ParamDescr *CreateParamDescrInt(const std::string &name, int value_default, int value_min, int value_max);
		static ParamDescr *CreateParamDescrIntList(const std::string &name, int value_default,
			const int *value_ary, size_t size);
		static ParamDescr *CreateParamDescrDouble(const std::string &name, double value_default, double value_min, double value_max);
		//static ParamDescr *CreateParamDescrStringList(const std::string &name, const char *value_ary[],
		//	size_t size);

	protected:
		ParamDescr(const std::string &name, ParamValueType value_type);
	
	private:
		std::string name_;
		ParamValueType value_type_;
	};

	//////////////////////////////////////////////////////////////////////
	// ParamArchive
	//////////////////////////////////////////////////////////////////////

	class ParamArchive
	{
	public:
		ParamArchive();
		virtual ~ParamArchive();
		
		void AppendDescr(const ParamDescr *descr);
		std::string GetParamsDefault() const;

		bool Parse(const std::string &s);

		int GetValueInt(const ParamDescr *descr) const;
		double GetValueDouble(const ParamDescr *descr) const;
		//std::string GetValueString(const ParamDescr *descr) const;
		void SetDefaults();

	private:
		std::vector<const ParamDescr *> descr_vect_;
		std::vector<ParamValue> value_vect_;

		int GetParamDescrIndex(const ParamDescr *descr) const;
		const ParamDescr *FindParamDescr(const std::string &name) const;
	};

} // namespace ew

#endif // ! PARAM_PARSER_H_