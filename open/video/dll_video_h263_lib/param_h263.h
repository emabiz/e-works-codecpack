/*****************************************************************************
 * H263 parameters
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

#ifndef PARAM_H263_H_
#define PARAM_H263_H_

#include "param_video.h"

namespace ew {

	class ParamArchiveH263Enc : public ParamArchiveVideo
	{
	public:
		ParamArchiveH263Enc();
		virtual ~ParamArchiveH263Enc();

		int GetPlus() const; //< h263p
		int GetTsto() const; //< temporal spatial trade off 0=image_quality .. 31=bandwidth_accuracy
		int GetAnnexD() const; //< Annex D: Unrestructed Motion Vectors: Level 2+ 
		int GetAnnexI() const; //< Annex I: Advanced Intra Coding: Level 3+, h263p
		int GetAnnexJ() const; //< Annex J: Deblocking Filter, h263p
		int GetSplitOversizeFrag() const; //< Split oversize fragment using rfc2190 mode c non necessarily at mb boundary

	private:
		ParamDescr *descr_plus_;
		ParamDescr *descr_tsto_;
		ParamDescr *descr_annex_d_;
		ParamDescr *descr_annex_i_;
		ParamDescr *descr_annex_j_;
		ParamDescr *descr_split_oversize_frag_;
	};

} // namespace ew

#endif // ! PARAM_H263_H_
