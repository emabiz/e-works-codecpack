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

#include "stdafx.h"
#include "param_h263.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	ParamArchiveH263Enc::ParamArchiveH263Enc()
	{
		descr_plus_ = ParamDescr::CreateParamDescrInt("PLUS", 0, 0, 1);
		descr_tsto_ = ParamDescr::CreateParamDescrInt("TSTO", 31, 0, 31);
		descr_annex_d_ = ParamDescr::CreateParamDescrInt("ANNEX_D", 0, 0, 1);
		descr_annex_i_ = ParamDescr::CreateParamDescrInt("ANNEX_I", 0, 0, 1);
		descr_annex_j_ = ParamDescr::CreateParamDescrInt("ANNEX_J", 0, 0, 1);
		descr_split_oversize_frag_ = ParamDescr::CreateParamDescrInt("SPLIT_OVERSIZE_FRAG", 1, 0, 1);
		
		AppendDescr(descr_plus_);
		AppendDescr(descr_tsto_);
		AppendDescr(descr_annex_d_);
		AppendDescr(descr_annex_i_);
		AppendDescr(descr_annex_j_);
		AppendDescr(descr_split_oversize_frag_);
	}

	ParamArchiveH263Enc::~ParamArchiveH263Enc()
	{
		delete descr_split_oversize_frag_;
		delete descr_annex_j_;
		delete descr_annex_i_;
		delete descr_annex_d_;
		delete descr_tsto_;
		delete descr_plus_;
	}

	int ParamArchiveH263Enc::GetPlus() const
	{
		return GetValueInt(descr_plus_);
	}

	int ParamArchiveH263Enc::GetTsto() const
	{
		return GetValueInt(descr_tsto_);
	}

	int ParamArchiveH263Enc::GetAnnexD() const
	{
		return GetValueInt(descr_annex_d_);
	}

	int ParamArchiveH263Enc::GetAnnexI() const
	{
		return GetValueInt(descr_annex_i_);
	}

	int ParamArchiveH263Enc::GetAnnexJ() const
	{
		return GetValueInt(descr_annex_j_);
	}

	int ParamArchiveH263Enc::GetSplitOversizeFrag() const
	{
		return GetValueInt(descr_split_oversize_frag_);
	}

} // namespace ew
