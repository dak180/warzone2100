/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2011  Warzone 2100 Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
#include "wzm.h"
#include "ivisdef.h"
#include "tex.h"
#include "pietypes.h"

iIMDShape::iIMDShape():
	texpages(static_cast<int>(WZM_TEX__LAST), iV_TEX_INVALID)
{
}

iIMDShape::~iIMDShape()
{
	unsigned int i;

	if (next)
		delete next;

	if (points)
	{
		free(points);
	}
	if (connectors)
	{
		free(connectors);
	}
	if (polys)
	{
		for (i = 0; i < npolys; ++i)
		{
			if (polys[i].texCoord)
			{
				free(polys[i].texCoord);
			}
		}
		free(polys);
	}
	if (shadowEdgeList)
	{
		free(shadowEdgeList);
		shadowEdgeList = NULL;
	}
}
