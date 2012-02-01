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
#ifndef _WZM_
#define _WZM_

//#include <map>
#include <vector>

#include "lib/framework/frame.h"
#include "pietypes.h"

#define WZM_MODEL_SIGNATURE "WZM"
#define WZM_MODEL_DIRECTIVE_TEXTURE "TEXTURE"
#define WZM_MODEL_DIRECTIVE_TCMASK "TCMASK"
#define WZM_MODEL_DIRECTIVE_NORMALMAP "NORMALMAP"
#define WZM_MODEL_DIRECTIVE_MESHES "MESHES"

enum wzm_texture_type_t {WZM_TEX_DIFFUSE = 0, WZM_TEX_TCMASK, WZM_TEX_NORMALMAP, WZM_TEX_SPECULAR,
			 WZM_TEX__LAST, WZM_TEX__FIRST = WZM_TEX_DIFFUSE};

//**************************PIE CRAP**************************************

#define PIE_NAME				"PIE"  // Pumpkin image export data file
#define PIE_VER				2
#define PIE_FLOAT_VER		3

// PIE model flags
#define iV_IMD_NOSTRETCH     0x00001000
#define iV_IMD_TCMASK        0x00010000

#define iV_IMD_TEX 0x00000200		// this is both a polygon and pie flag
#define iV_IMD_TEXANIM 0x00004000 // iV_IMD_TEX must be set also

//*************************************************************************
//
// imd structures
//
//*************************************************************************

/// Stores the from and to verticles from an edge
struct EDGE
{
	int from, to;
};

struct iIMDPoly
{
	uint32_t flags;

	unsigned int npnts;
	Vector3f normal;
	int pindex[3];
	Vector2f *texCoord;
	Vector2f texAnim;
};

struct iIMDShape
{
	unsigned int flags;

	int sradius, radius;
	Vector3i min, max;

	Vector3f ocen;
	unsigned short numFrames;
	unsigned short animInterval;

	unsigned int npoints;
	Vector3f *points;

	unsigned int npolys;
	iIMDPoly *polys;

	unsigned int nconnectors;
	Vector3i *connectors;

	unsigned int nShadowEdges;
	EDGE *shadowEdgeList;
	float material[LIGHT_MAX][4];
	float shininess;

	iIMDShape *next;  // next pie in multilevel pies (NULL for non multilevel !)

private:
	std::vector<int> texpages;

public:
	iIMDShape();
	~iIMDShape();

	int getTexturePage(wzm_texture_type_t type) const {return texpages[type];}
	void setTexturePage(wzm_texture_type_t type, int texpage) {texpages[type] = texpage;}
};

#endif //_WZM_
