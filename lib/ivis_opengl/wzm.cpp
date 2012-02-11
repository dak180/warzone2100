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

#include "lib/framework/frame.h"

#include <algorithm>

#define WZM_MODEL_DIRECTIVE_TEXTURE "TEXTURE"
#define WZM_MODEL_DIRECTIVE_TCMASK "TCMASK"
#define WZM_MODEL_DIRECTIVE_NORMALMAP "NORMALMAP"
#define WZM_MODEL_DIRECTIVE_MESHES "MESHES"

#define WZM_MESH_SIGNATURE "MESH"
#define WZM_MESH_DIRECTIVE_TEAMCOLOURS "TEAMCOLOURS"
#define WZM_MESH_DIRECTIVE_MINMAXTSCEN "MINMAX_TSCEN"
#define WZM_MESH_DIRECTIVE_VERTICES "VERTICES"
#define WZM_MESH_DIRECTIVE_INDICES "INDICES"
#define WZM_MESH_DIRECTIVE_VERTEXARRAY "VERTEXARRAY"
#define WZM_MESH_DIRECTIVE_INDEXARRAY "INDEXARRAY"
#define WZM_MESH_DIRECTIVE_CONNECTORS "CONNECTORS"

WZMesh::WZMesh():
	m_teamColours(false)
{
}

WZMesh::~WZMesh()
{
}

void WZMesh::clear()
{
	//std::fill(m_texpages.begin(), m_texpages.end(), iV_TEX_INVALID);

	m_vertexArray.clear();
	m_textureArray.clear();
	m_normalArray.clear();
	m_tangentArray.clear();

	// Set default values
	memset(m_material, 0, sizeof(m_material));

	m_material[LIGHT_AMBIENT][0] = 1.0f;
	m_material[LIGHT_AMBIENT][1] = 1.0f;
	m_material[LIGHT_AMBIENT][2] = 1.0f;
	m_material[LIGHT_AMBIENT][3] = 1.0f;
	m_material[LIGHT_DIFFUSE][0] = 1.0f;
	m_material[LIGHT_DIFFUSE][1] = 1.0f;
	m_material[LIGHT_DIFFUSE][2] = 1.0f;
	m_material[LIGHT_DIFFUSE][3] = 1.0f;
	m_material[LIGHT_SPECULAR][0] = 1.0f;
	m_material[LIGHT_SPECULAR][1] = 1.0f;
	m_material[LIGHT_SPECULAR][2] = 1.0f;
	m_material[LIGHT_SPECULAR][3] = 1.0f;
	m_shininess = 10;

	m_aabb_min = m_aabb_max = m_tightspherecenter = Vector3f(0., 0., 0.);
}

bool WZMesh::loadFromStream(std::istream &in)
{
	std::string str;
	unsigned i, vertices, indices;

	clear();

	in >> str >> m_name;
	if (in.fail() || str.compare(WZM_MESH_SIGNATURE) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_SIGNATURE, str.c_str());
		return false;
	}

	in >> str >> m_teamColours;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_TEAMCOLOURS) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_TEAMCOLOURS, str.c_str());
		return false;
	}

	in >> str;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_MINMAXTSCEN) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_MINMAXTSCEN, str.c_str());
		return false;
	}
	else
	{
		in >> m_aabb_min.x >> m_aabb_min.y >> m_aabb_min.z
		   >> m_aabb_max.x >> m_aabb_max.y >> m_aabb_max.z
		   >> m_tightspherecenter.x >> m_tightspherecenter.y >> m_tightspherecenter.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading minmaxtspcen values");
			return false;
		}
	}

	in >> str >> vertices;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_VERTICES) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_VERTICES, str.c_str());
		return false;
	}

	in >> str >> indices;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_INDICES) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_INDICES, str.c_str());
		return false;
	}

	in >> str;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_VERTEXARRAY) !=0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_VERTEXARRAY, str.c_str());
		return false;
	}

	m_vertexArray.reserve(vertices);
	m_textureArray.reserve(vertices);
	m_normalArray.reserve(vertices);
	m_tangentArray.reserve(vertices);

	Vector3f vert, normal;
	Vector4f tangent;
	Vector2f uv;

	for (; vertices > 0; --vertices)
	{
		in >> vert.x >> vert.y >> vert.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading vertex");
			return false;
		}
		m_vertexArray.push_back(vert);

		in >> uv.x >> uv.y;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading uv coords.");
			return false;
		}
		else if (uv.x > 1 || uv.y > 1)
		{
			debug(LOG_WARNING, "WZMesh - Error uv coords out of range");
			return false;
		}
		m_textureArray.push_back(uv);

		in >> normal.x >> normal.y >> normal.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading normal");
			return false;
		}
		m_normalArray.push_back(normal);

		in >> tangent.x >> tangent.y >> tangent.z >> tangent.w;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading tangent");
			return false;
		}
		m_tangentArray.push_back(tangent);
	}

	in >> str;
	if (str.compare(WZM_MESH_DIRECTIVE_INDEXARRAY) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_INDEXARRAY, str.c_str());
		return false;
	}

	m_indexArray.reserve(indices);

	for(; indices > 0; --indices)
	{
		Vector3i tri;

		in >> tri.x >> tri.y >> tri.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading indices");
			return false;
		}
		m_indexArray.push_back(tri);
	}

	in >> str >> i;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_CONNECTORS) != 0)
	{
		debug(LOG_WARNING, "WZMesh - Expected %s directive found %s", WZM_MESH_DIRECTIVE_CONNECTORS, str.c_str());
		return false;
	}

	if (i > 0)
	{
		Vector3f con;

		in >> con.x >> con.y >> con.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading connectors");
			return false;
		}
		m_connectorArray.push_back(con);
	}

	return true;
}

iIMDShape::iIMDShape():
	flags(0), numFrames(0), animInterval(1),

	npoints(0), points(0),
	npolys(0), polys(0),
	nconnectors(0), connectors(0),
	nShadowEdges(0), shadowEdgeList(0),

	next(0),
	m_texpages(static_cast<int>(WZM_TEX__LAST), iV_TEX_INVALID)
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

void iIMDShape::clear()
{
	std::fill(m_texpages.begin(), m_texpages.end(), iV_TEX_INVALID);
}

bool iIMDShape::loadFromStream(std::istream &in)
{
	std::string str;
	int i, meshes;

	clear();
	in >> str;
	if (in.fail() || str.compare(WZM_MODEL_SIGNATURE) != 0)
	{
		debug(LOG_WARNING, "WZM::read - missing header");
		return false;
	}

	in >> i;
	if (in.fail())
	{
		debug(LOG_WARNING, "WZM::read - error reading version");
		return false;
	}
	else if(i != WZM_MODEL_VERSION_FD)
	{
		debug(LOG_WARNING, "WZM::read - unsupported version %d ", i);
		return false;
	}

	// TEXTURE %s
	in >> str;
	if (str.compare(WZM_MODEL_DIRECTIVE_TEXTURE) != 0)
	{
		debug(LOG_WARNING, "WZM::read - Expected %s directive but got %s", WZM_MODEL_DIRECTIVE_TEXTURE, str.c_str());
		return false;
	}
	in >> str;
	if (in.fail())
	{
		debug(LOG_WARNING, "WZM::read - Error reading texture name");
		return false;
	}

	m_texpages[WZM_TEX_DIFFUSE] = iV_GetTexture(str.c_str());
	ASSERT_OR_RETURN(NULL, m_texpages[WZM_TEX_DIFFUSE] > iV_TEX_INVALID, "Could not load tex page %s", str.c_str());

	// read next token
	in >> str;

	// TCMASK ?
	if (!str.compare(WZM_MODEL_DIRECTIVE_TCMASK))
	{
		in >> str;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZM::read - Error reading TCMask name");
			return false;
		}

		m_texpages[WZM_TEX_TCMASK] = iV_GetTexture(str.c_str());
		ASSERT_OR_RETURN(NULL, m_texpages[WZM_TEX_TCMASK] > iV_TEX_INVALID, "Could not load TCMask page %s", str.c_str());

		// pre read next token
		in >> str;
	}

	// NORMALMAP ?
	if (!str.compare(WZM_MODEL_DIRECTIVE_NORMALMAP))
	{
		in >> str;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZM::read - Error reading NORMALMAP name");
			return false;
		}

		m_texpages[WZM_TEX_NORMALMAP] = iV_GetTexture(str.c_str());
		ASSERT_OR_RETURN(NULL, m_texpages[WZM_TEX_NORMALMAP] > iV_TEX_INVALID, "Could not load normalmap page %s", str.c_str());

		// pre read next token
		in >> str;
	}

	// token was pre read here
	// MESHES %u
	in >> meshes;
	if (in.fail() || str.compare("MESHES") != 0)
	{
		debug(LOG_WARNING, "WZM::read - Expected MESHES directive but got %s", str.c_str());
		return false;
	}

	if (meshes < 1)
	{
		debug(LOG_WARNING, "WZM::read - Expected some MESHES there");
		return false;
	}

	for(; meshes > 0; --meshes)
	{
		WZMesh mesh;
		if (!mesh.loadFromStream(in))
		{
			return false;
		}
		m_meshes.push_back(mesh);
	}

	// get some compat values from the first mesh

	WZMesh& mesh0 = m_meshes.front();

	float xmax = MAX(mesh0.m_aabb_max.x, -mesh0.m_aabb_min.x);
	float ymax = MAX(mesh0.m_aabb_max.y, -mesh0.m_aabb_min.y);
	float zmax = MAX(mesh0.m_aabb_max.z, -mesh0.m_aabb_min.z);

	radius = MAX(xmax, MAX(ymax, zmax));
	sradius = sqrtf(xmax*xmax + ymax*ymax + zmax*zmax);
	min = mesh0.m_aabb_min;
	max = mesh0.m_aabb_max;
	ocen = mesh0.m_tightspherecenter;

	return true;
}
