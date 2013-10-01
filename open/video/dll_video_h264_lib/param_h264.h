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

#ifndef PARAM_H264_H_
#define PARAM_H264_H_

#include "param_video.h"

namespace ew {

	/** H264 profiles definitions.
		Values are those in H323 negotiation.
		At present only baseline is supported */
	enum H264Profile {
		H264Profile_baseline      = 1 << 6,
	/*
		H264Profile_main          = 1 << 5,
		H264Profile_extended      = 1 << 4,
		H264Profile_high          = 1 << 3,
		H264Profile_high10        = 1 << 2,
		H264Profile_high422       = 1 << 1,
		H264Profile_high420       = 1 << 0,
	*/
	};

	/** H264 levels */
	enum H264Level {
		H264Level_10    = 15,   ///< Level 1.0
		//H264Level_1B    = 19,   ///< Level 1B not supported by x264
		H264Level_11    = 22,   ///< Level 1.1
		H264Level_12    = 29,   ///< Level 1.2
		H264Level_13    = 36,   ///< Level 1.3
		H264Level_20    = 43,   ///< Level 2.0
		H264Level_21    = 50,   ///< Level 2.1
		H264Level_22    = 57,   ///< Level 2.2
	/*
		H264Level_30    = 64,   ///< Level 3.0
		H264Level_31    = 71,   ///< Level 3.1
		H264Level_32    = 78,   ///< Level 3.2
		H264Level_40    = 85,   ///< Level 4.0
		H264Level_41    = 92,   ///< Level 4.1
		H264Level_42    = 99,   ///< Level 4.2
		H264Level_50    =106,   ///< Level 5.0
		H264Level_51    =113    ///< Livello 5.1
	*/
		H264Level_Invalid = 0
	};

	struct H264ToX264Level {
		H264Level h264_level;
		int	x264_level;
	};

	extern const H264ToX264Level kH264ToX264LevelAry[]; // terminated by element {H264Level_Invalid, 0}
	int FindX264Level(H264Level h264_level);

	/** H264 fragmentation modes.
		At present only "Single NAL Unit" and "Non-Interleaved Mode" are supported */
	enum H264FragMode {
		H264FragMode_SingleNalUnit,         ///< Single NAL Unit
		H264FragMode_NonInterleavedMode,    ///< Non-Interleaved Mode
	/*
		H264FragMode_InterleavedMode,        ///< Interleaved Mode
	*/
		H264FragMode_Count
	};

	class ParamArchiveH264Enc : public ParamArchiveVideo
	{
	public:
		ParamArchiveH264Enc();
		virtual ~ParamArchiveH264Enc();

		H264Profile GetProfile() const;
		H264Level GetLevel() const;
		H264FragMode GetFragMode() const;

	private:
		ParamDescr *descr_profile_;
		ParamDescr *descr_level_;
		ParamDescr *descr_frag_mode_;
	};

} // namespace ew

#endif // ! PARAM_H264_H_