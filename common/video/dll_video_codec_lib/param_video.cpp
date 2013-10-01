#include "stdafx.h"
#include "param_video.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	ParamArchiveVideo::ParamArchiveVideo()
	{
		descr_gop_size_ = ParamDescr::CreateParamDescrInt("GOPSIZE", 100, 1, 1000);
		AppendDescr(descr_gop_size_);
	}

	ParamArchiveVideo::~ParamArchiveVideo()
	{
		delete descr_gop_size_;
	}

	int ParamArchiveVideo::GetGopSize() const
	{
		return GetValueInt(descr_gop_size_);
	}

} // namespace ew