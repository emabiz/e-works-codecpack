/*****************************************************************************
 * H264 parameters
 *****************************************************************************
 * Copyright (C) 2010-2011 Yuri Valentini
 *
 * Authors: Yuri Valentini <yv@opycom.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

#include "stdafx.h"
#include "param_h264.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	const H264ToX264Level kH264ToX264LevelAry[] =
	{
		{ H264Level_10, 10 },
		{ H264Level_11, 11 },
		{ H264Level_12, 12 },
		{ H264Level_13, 13 },
		{ H264Level_20, 20 },
		{ H264Level_21, 21 },
		{ H264Level_22, 22 },
		{ H264Level_Invalid, 0 }
	};

	int FindX264Level(H264Level h264_level)
	{
		for (const H264ToX264Level *H264ToX264 = kH264ToX264LevelAry;
			H264ToX264->h264_level != H264Level_Invalid; ++H264ToX264) {
				if (H264ToX264->h264_level == h264_level)
					return H264ToX264->x264_level;
		}

		return -1; // default value as in x264
	}


	ParamArchiveH264Enc::ParamArchiveH264Enc()
	{
		descr_profile_ = ParamDescr::CreateParamDescrInt("PROFILE", H264Profile_baseline,
			H264Profile_baseline, H264Profile_baseline);
		int level_ary[] = {
			H264Level_10,
			//H264Level_1B,
			H264Level_11,
			H264Level_12,
			H264Level_13,
			H264Level_20,
			H264Level_21,
			H264Level_22
		};
		descr_level_ = ParamDescr::CreateParamDescrIntList("LEVEL", H264Level_22,
			level_ary, sizeof(level_ary)/sizeof(level_ary[0]));
		descr_frag_mode_ = ParamDescr::CreateParamDescrInt("FRAGMODE", H264FragMode_SingleNalUnit,
			H264FragMode_SingleNalUnit, H264FragMode_NonInterleavedMode);

		AppendDescr(descr_profile_);
		AppendDescr(descr_level_);
		AppendDescr(descr_frag_mode_);
	}

	ParamArchiveH264Enc::~ParamArchiveH264Enc()
	{
		delete descr_frag_mode_;
		delete descr_level_;
		delete descr_profile_;
	}

	H264Profile ParamArchiveH264Enc::GetProfile() const
	{
		return static_cast<H264Profile>(GetValueInt(descr_profile_));
	}

	H264Level ParamArchiveH264Enc::GetLevel() const
	{
		return static_cast<H264Level>(GetValueInt(descr_level_));
	}

	H264FragMode ParamArchiveH264Enc::GetFragMode() const
	{
		return static_cast<H264FragMode>(GetValueInt(descr_frag_mode_));
	}

} // namespace ew
