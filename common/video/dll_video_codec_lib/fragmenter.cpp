#include "stdafx.h"

#include "fragmenter.h"
#include "frag_helper.h"
#include "video_codec_def.h"
#include "dll_def.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	Fragmenter::Fragmenter(int fragsize, FragWriter *fragwriter)
		: fragsize_(fragsize), fragwriter_(fragwriter)
	{
		_ASSERT(fragsize >= kFragsizeMin);
		_ASSERT(NULL != fragwriter);
	}

} // namespace ew