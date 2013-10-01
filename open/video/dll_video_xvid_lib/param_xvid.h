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

#ifndef PARAM_XVID_H_
#define PARAM_XVID_H_

#include "param_video.h"

namespace ew {

	class ParamArchiveXvidEnc : public ParamArchiveVideo
	{
	public:
		ParamArchiveXvidEnc();
		virtual ~ParamArchiveXvidEnc();

		int GetQuality() const; //< Compression quality
		int GetQMin() const;
		int GetQMax() const;

	private:
		ParamDescr *descr_quality_;
		ParamDescr *descr_qmin_;
		ParamDescr *descr_qmax_;
	};

} // namespace ew

#endif // ! PARAM_XVID_H_
