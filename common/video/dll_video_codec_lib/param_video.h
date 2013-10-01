#ifndef PARAM_VIDEO_H_
#define PARAM_VIDEO_H_

#include "param_parser.h"

namespace ew {

	class ParamArchiveVideo : public ParamArchive
	{
	public:
		ParamArchiveVideo();
		virtual ~ParamArchiveVideo();

		int GetGopSize() const;

	private:
		ParamDescr *descr_gop_size_;
	};

} // namespace ew

#endif // ! PARAM_VIDEO_H_