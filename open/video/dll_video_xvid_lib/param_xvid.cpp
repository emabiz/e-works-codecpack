/*****************************************************************************
 * Xvid parameters
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
#include "param_xvid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace ew {

	ParamArchiveXvidEnc::ParamArchiveXvidEnc()
	{
		descr_quality_ = ParamDescr::CreateParamDescrInt("QUALITY", 2, 0, 4);
		AppendDescr(descr_quality_);
		descr_qmin_ = ParamDescr::CreateParamDescrInt("QMIN", 2, 2, 31);
		AppendDescr(descr_qmin_);
		descr_qmax_ = ParamDescr::CreateParamDescrInt("QMAX", 31, 2, 31);
		AppendDescr(descr_qmax_);
	}

	ParamArchiveXvidEnc::~ParamArchiveXvidEnc()
	{
		delete descr_qmax_;
		delete descr_qmin_;
		delete descr_quality_;
	}

	int ParamArchiveXvidEnc::GetQuality() const
	{
		return GetValueInt(descr_quality_);
	}

	int ParamArchiveXvidEnc::GetQMin() const
	{
		int qmin = GetValueInt(descr_qmin_);
		int qmax = GetValueInt(descr_qmax_);
		return qmin <= qmax ? qmin : descr_qmin_->GetValueDefault().i;
	}

	int ParamArchiveXvidEnc::GetQMax() const
	{
		int qmin = GetValueInt(descr_qmin_);
		int qmax = GetValueInt(descr_qmax_);
		return qmin <= qmax ? qmax : descr_qmax_->GetValueDefault().i;
	}

} // namespace ew
