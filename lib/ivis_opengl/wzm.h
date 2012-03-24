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

#include <string>
#include <vector>
#include <list>

#include "lib/framework/frame.h"
#include "lib/ivis_opengl/piedef.h"

//************************** PIE **************************************

#define PIE_NAME				"PIE"  // Pumpkin image export data file
#define PIE_VER				2
#define PIE_FLOAT_VER		3

// PIE model flags
#define iV_IMD_NOSTRETCH     0x00001000
#define iV_IMD_TCMASK        0x00010000
#define iV_IMD_TEX           0x00000200	// this is both a polygon and pie flag
#define iV_IMD_TEXANIM       0x00004000 // iV_IMD_TEX must be set also

// premultiplied implies additive
#define iV_IMD_NO_ADDITIVE     0x00000001
#define iV_IMD_ADDITIVE        0x00000002
#define iV_IMD_PREMULTIPLIED   0x00000004
// pitch to camera implies roll to camera
#define iV_IMD_ROLL_TO_CAMERA  0x00000010
#define iV_IMD_PITCH_TO_CAMERA 0x00000020
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

// old stuff above ^^ REMOVE ON WZM COMPLETION


//************************** WZRenderer **************************************

/*
*    Whole WZRenderer-iIMDShape thingy is NOT thread safe currently :(
*    Shouln't be a big problem for now as res loading is sequential from primary thread.
*/

//union PIELIGHT;
struct iIMDShape;

struct WZRenderListNode
{
	float		matrix[16];
	iIMDShape*	shape;
	PIELIGHT	colour;
	PIELIGHT	teamcolour;
	int		flag;
	int		flag_data;

	bool operator < (const WZRenderListNode& rhs) const
	{
		if (shape < rhs.shape)
			return true;
		else
			return false;
	}
};

struct WZRenderer
{
	std::vector<WZRenderListNode> m_drawList;

	std::vector<Vector3f> m_vertexArray;
	std::vector<Vector2f> m_textureArray;
	std::vector<Vector3f> m_normalArray;
	std::vector<Vector4f> m_tangentArray;

	// Default material values
	float mat_default_reflections[LIGHT_MAX][4];
	float mat_default_shininess;

public:
	bool loadMaterialDefaults(const char *pFileName);

	bool init();
	void shutdown();

	void addNodeToDrawList(const WZRenderListNode& node);
	void resetDrawList();

	void renderDrawList();
};

//************************** WZM **************************************

#define WZM_AABB_SIZE 8

#define WZM_MODEL_SIGNATURE "WZM"
#define WZM_MODEL_VERSION_FD 3 // First draft version

enum wzm_texture_type_t {WZM_TEX_DIFFUSE = 0, WZM_TEX_TCMASK, WZM_TEX_NORMALMAP, WZM_TEX_SPECULAR,
			 WZM_TEX__LAST, WZM_TEX__FIRST = WZM_TEX_DIFFUSE};

class WZMesh
{
	friend struct iIMDShape;

	std::string m_name;
	bool m_teamColours;
	unsigned int m_vertices_count;

	unsigned int m_index_min, m_index_max;

	std::vector<Vector3ui> m_indexArray;
	std::vector<Vector3f> m_connectorArray;

	Vector3f m_tightspherecenter;
	Vector3f m_aabb[WZM_AABB_SIZE];

	void setAABBminmax(bool ismin, const Vector3f& value);
	void recalcAABB();

public:
	WZMesh();
	~WZMesh();

	void clear();

	bool loadHeader(std::istream& in);
	bool loadGeometry(std::istream &in, WZRenderer& renderer);

	const Vector3f& getAABBminmax(bool ismin) const {return (ismin) ? m_aabb[0] : m_aabb[4];}
	Vector3f getAABBcenter();
	const Vector3f& getAABBpoint(unsigned int idx) const {return m_aabb[idx];}

	static void mirrorVertexFromPoint(Vector3f &vertex, const Vector3f &point, int axis); // x == 0, y == 1, z == 2
};

struct iIMDShape
{
	unsigned int flags;

	int sradius, radius;
	Vector3f min, max;
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

	// old stuff above ^^ REMOVE ON WZM COMPLETION

	std::string m_resname; // for debug
private:
	std::vector<int> m_texpages;
	std::list<WZMesh> m_meshes;

	Vector3f m_aabb[WZM_AABB_SIZE];

public:
	iIMDShape();
	~iIMDShape();

	void clear();
	void draw(PIELIGHT colour, PIELIGHT teamcolour, int pieFlag, int pieFlagData);
	void drawFast(PIELIGHT colour, PIELIGHT teamcolour, int pieFlag);

	bool loadFromStream(std::istream& in);

	int getTexturePage(wzm_texture_type_t type) const {return m_texpages[type];}
	void setTexturePage(wzm_texture_type_t type, int texpage) {m_texpages[type] = texpage;}

	const Vector3f& getAABBminmax(bool ismin) const {return (ismin) ? m_aabb[0] : m_aabb[4];}
	Vector3f getAABBcenter();
	const Vector3f& getAABBpoint(unsigned int idx) const {return m_aabb[idx];}

	bool isWZMFormat() const {return !m_meshes.empty();}
};

#endif //_WZM_
