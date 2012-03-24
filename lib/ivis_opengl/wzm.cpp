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
#include "lib/framework/wzconfig.h"
#include "lib/ivis_opengl/piedef.h"
#include "lib/ivis_opengl/piestate.h"

#include <algorithm>

#define WZM_MODEL_DIRECTIVE_TEXTURE "TEXTURE"
#define WZM_MODEL_DIRECTIVE_TCMASK "TCMASK"
#define WZM_MODEL_DIRECTIVE_NORMALMAP "NORMALMAP"
#define WZM_MODEL_DIRECTIVE_SPECULARMAP "SPECULARMAP"
#define WZM_MODEL_DIRECTIVE_MATERIAL "MATERIAL"
#define WZM_MODEL_DIRECTIVE_MESHES "MESHES"

#define WZM_MESH_SIGNATURE "MESH"
#define WZM_MESH_DIRECTIVE_TEAMCOLOURS "TEAMCOLOURS"
#define WZM_MESH_DIRECTIVE_MINMAXTSCEN "MINMAX_TSCEN"
#define WZM_MESH_DIRECTIVE_VERTICES "VERTICES"
#define WZM_MESH_DIRECTIVE_INDICES "INDICES"
#define WZM_MESH_DIRECTIVE_VERTEXARRAY "VERTEXARRAY"
#define WZM_MESH_DIRECTIVE_INDEXARRAY "INDEXARRAY"
#define WZM_MESH_DIRECTIVE_CONNECTORS "CONNECTORS"

// Global WZM renderer instance
WZRenderer g_wzmRenderer;

bool WZRenderer::loadMaterialDefaults(const char *pFileName)
{
	WzConfig ini(pFileName);
	if (ini.status() != QSettings::NoError)
	{
		debug(LOG_ERROR, "Could not open %s", pFileName);
		return false;
	}

	Vector3f vec;

	vec = ini.vector3f("Lightning/DefaultMaterial_Ambient");
	mat_default_reflections[LIGHT_AMBIENT][0] = vec.x; mat_default_reflections[LIGHT_AMBIENT][1] = vec.y; mat_default_reflections[LIGHT_AMBIENT][2] = vec.z;
	mat_default_reflections[LIGHT_AMBIENT][3] = 1.0f;

	vec = ini.vector3f("Lightning/DefaultMaterial_Diffuse");
	mat_default_reflections[LIGHT_DIFFUSE][0] = vec.x; mat_default_reflections[LIGHT_DIFFUSE][1] = vec.y; mat_default_reflections[LIGHT_DIFFUSE][2] = vec.z;
	mat_default_reflections[LIGHT_DIFFUSE][3] = 1.0f;

	vec = ini.vector3f("Lightning/DefaultMaterial_Specular");
	mat_default_reflections[LIGHT_SPECULAR][0] = vec.x; mat_default_reflections[LIGHT_SPECULAR][1] = vec.y; mat_default_reflections[LIGHT_SPECULAR][2] = vec.z;
	mat_default_reflections[LIGHT_SPECULAR][3] = 1.0f;

	vec = ini.vector3f("Lightning/DefaultMaterial_Emissive");
	mat_default_reflections[LIGHT_EMISSIVE][0] = vec.x; mat_default_reflections[LIGHT_EMISSIVE][1] = vec.y; mat_default_reflections[LIGHT_EMISSIVE][2] = vec.z;
	mat_default_reflections[LIGHT_EMISSIVE][3] = 1.0f;

	mat_default_shininess = ini.value("Lightning/DefaultMaterial_Shininess").toFloat();

	return true;
}

bool WZRenderer::init()
{
	return true;
}

void WZRenderer::shutdown()
{
	debug(LOG_3D, "WZRenderer - shutdown, geometry count was %d", (int)m_vertexArray.size());

	resetDrawList();

	m_vertexArray.clear();
	m_textureArray.clear();
	m_normalArray.clear();
	m_tangentArray.clear();
}

void WZRenderer::addNodeToDrawList(const WZRenderListNode &node)
{
	m_drawList.push_back(node);
}

void WZRenderer::resetDrawList()
{
	m_drawList.clear();
}

void WZRenderer::renderDrawList()
{
	bool shaders = pie_GetShaderAvailability();

	if (m_drawList.empty())
		return;

	std::sort(m_drawList.begin(), m_drawList.end());
	WZRenderListNode& node = m_drawList.front();

	glErrors();

	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, 0, &m_textureArray[0]);
	glNormalPointer(GL_FLOAT, 0, &m_normalArray[0]);
	glVertexPointer(3, GL_FLOAT, 0, &m_vertexArray[0]);

	if (shaders)
	{
		pie_ActivateShader(SHADER_COMPONENT, node.shape, node.teamcolour, node.colour);
		pie_SetShaderTangentAttribute(&m_tangentArray[0]);
	}
	else
	{
		pie_ActivateFallback(SHADER_COMPONENT, node.shape, node.teamcolour, node.colour);
	}

	pie_SetAlphaTest(true);
	pie_SetFogStatus(true);
	pie_SetRendMode(REND_OPAQUE);

	glErrors();

	for (std::vector<WZRenderListNode>::const_iterator nodeit = m_drawList.begin(); nodeit != m_drawList.end(); ++nodeit)
	{
		glLoadMatrixf(nodeit->matrix);
		nodeit->shape->drawFast(nodeit->colour, nodeit->teamcolour, nodeit->flag);
	}

	if (shaders)
	{
		pie_DeactivateShader();
	}
	else
	{
		pie_DeactivateFallback();
	}

	glErrors();

	glPopClientAttrib();
	glPopAttrib();
}

void WZMesh::mirrorVertexFromPoint(Vector3f &vertex, const Vector3f &point, int axis)
{
	switch (axis)
	{
	case 0:
		vertex.x = -vertex.x + 2 * point.x;
		break;
	case 1:
		vertex.y = -vertex.y + 2 * point.y;
		break;
	default:
		vertex.z = -vertex.z + 2 * point.z;
	}
}

WZMesh::WZMesh():
	m_teamColours(false), m_vertices_count(0)
{
}

WZMesh::~WZMesh()
{
}

void WZMesh::clear()
{
	m_name.clear();
	m_teamColours = false;
	m_vertices_count = 0;

	m_tightspherecenter = Vector3f(0., 0., 0.);
	std::fill_n(m_aabb, WZM_AABB_SIZE, Vector3f(0., 0., 0.));

	m_indexArray.clear();
	m_connectorArray.clear();
}

bool WZMesh::loadHeader(std::istream &in)
{
	std::string str;
	unsigned indices;

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
		Vector3f vec;
		in >> vec.x >> vec.y >> vec.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading minmaxtspcen min value");
			return false;
		}
		setAABBminmax(true, vec);

		in >> vec.x >> vec.y >> vec.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading minmaxtspcen max value");
			return false;
		}
		setAABBminmax(false, vec);

		in >> m_tightspherecenter.x >> m_tightspherecenter.y >> m_tightspherecenter.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZMesh - Error reading minmaxtspcen tspcen value");
			return false;
		}

		recalcAABB();
	}

	in >> str >> m_vertices_count;
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

	m_indexArray.resize(indices);

	return true;
}

bool WZMesh::loadGeometry(std::istream &in, WZRenderer& renderer)
{
	Vector3f vert, normal, con;
	Vector4f tangent;
	Vector2f uv;
	Vector3ui tri;
	std::string str;
	unsigned i;

	// Here we merge geometry data from all meshes into continuos arrays
	// FIXME: check if m_vertexArray.size() overflows unsigned int!
	unsigned int indicesoffset = renderer.m_vertexArray.size();

	m_index_min = indicesoffset;
	m_index_max = indicesoffset + m_vertices_count - 1;

	renderer.m_vertexArray.reserve(indicesoffset + m_vertices_count);
	renderer.m_textureArray.reserve(indicesoffset + m_vertices_count);
	renderer.m_normalArray.reserve(indicesoffset + m_vertices_count);
	renderer.m_tangentArray.reserve(indicesoffset + m_vertices_count);

	for (unsigned int i = m_vertices_count; i > 0; --i)
	{
		in >> vert.x >> vert.y >> vert.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading vertex");
			return false;
		}
		renderer.m_vertexArray.push_back(vert);

		in >> uv.x >> uv.y;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading uv coords.");
			return false;
		}
		else if (uv.x > 1 || uv.y > 1)
		{
			debug(LOG_WARNING, "loadGeometry - Error uv coords out of range");
			return false;
		}
		renderer.m_textureArray.push_back(uv);

		in >> normal.x >> normal.y >> normal.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading normal");
			return false;
		}
		renderer.m_normalArray.push_back(normal);

		in >> tangent.x >> tangent.y >> tangent.z >> tangent.w;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading tangent");
			return false;
		}
		renderer.m_tangentArray.push_back(tangent);
	}

	in >> str;
	if (str.compare(WZM_MESH_DIRECTIVE_INDEXARRAY) != 0)
	{
		debug(LOG_WARNING, "loadGeometry - Expected %s directive found %s", WZM_MESH_DIRECTIVE_INDEXARRAY, str.c_str());
		return false;
	}

	for (unsigned indices = 0; indices < m_indexArray.size(); ++indices)
	{
		in >> tri.x >> tri.y >> tri.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading indices");
			return false;
		}

		tri.x += indicesoffset;
		tri.y += indicesoffset;
		tri.z += indicesoffset;

		m_indexArray[indices] = tri;
	}

	in >> str >> i;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_CONNECTORS) != 0)
	{
		debug(LOG_WARNING, "loadGeometry - Expected %s directive found %s", WZM_MESH_DIRECTIVE_CONNECTORS, str.c_str());
		return false;
	}


	for(; i > 0; --i)
	{
		in >> con.x >> con.y >> con.z;
		if (in.fail())
		{
			debug(LOG_WARNING, "loadGeometry - Error reading connectors");
			return false;
		}
		m_connectorArray.push_back(con);
	}

	return true;
}

inline void WZMesh::setAABBminmax(bool ismin, const Vector3f &value)
{
	if (ismin)
	{
		m_aabb[0] = value;
	}
	else
	{
		m_aabb[4] = value;
	}
}

void WZMesh::recalcAABB()
{
	Vector3f center = getAABBcenter();

	m_aabb[3] = m_aabb[2] = m_aabb[1] = m_aabb[0];
	mirrorVertexFromPoint(m_aabb[1], center, 0);
	mirrorVertexFromPoint(m_aabb[2], center, 1);
	mirrorVertexFromPoint(m_aabb[3], center, 2);

	m_aabb[7] = m_aabb[6] = m_aabb[5] = m_aabb[4];
	mirrorVertexFromPoint(m_aabb[5], center, 0);
	mirrorVertexFromPoint(m_aabb[6], center, 1);
	mirrorVertexFromPoint(m_aabb[7], center, 2);
}

Vector3f WZMesh::getAABBcenter()
{
	Vector3f center;

	center.x = (m_aabb[0].x + m_aabb[4].x) / 2;
	center.y = (m_aabb[0].y + m_aabb[4].y) / 2;
	center.z = (m_aabb[0].z + m_aabb[4].z) / 2;

	return center;
}

/// iIMDShape ********************************************************

iIMDShape::iIMDShape():
	flags(0), numFrames(0), animInterval(1),

	npoints(0), points(0),
	npolys(0), polys(0),
	nconnectors(0), connectors(0),
	nShadowEdges(0), shadowEdgeList(0),

	next(0),
	m_texpages(static_cast<int>(WZM_TEX__LAST), iV_TEX_INVALID)
{
	clear(); // FIXME: remove when PIE is no more, should be called on purpose, such as loading from stream
}

iIMDShape::~iIMDShape()
{
	if (isWZMFormat())
	{
		if (connectors)
			delete[] connectors;
	}
	else
	{
		unsigned int i;

		if (next)
			delete next;

		if (points)
			free(points);

		if (connectors)
			free(connectors);

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
}

void iIMDShape::clear()
{
	std::fill(m_texpages.begin(), m_texpages.end(), iV_TEX_INVALID);
	std::fill_n(m_aabb, WZM_AABB_SIZE, Vector3f(0., 0., 0.));

	// Set default values
	memcpy(material, g_wzmRenderer.mat_default_reflections, sizeof(material));
	shininess = g_wzmRenderer.mat_default_shininess;
}

bool iIMDShape::loadFromStream(std::istream &in)
{
	std::string str;
	int i, meshes;

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

	// SPECULARMAP ?
	if (!str.compare(WZM_MODEL_DIRECTIVE_SPECULARMAP))
	{
		in >> str;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZM::read - Error reading SPECULARMAP name");
			return false;
		}

		m_texpages[WZM_TEX_SPECULAR] = iV_GetTexture(str.c_str());
		ASSERT_OR_RETURN(NULL, m_texpages[WZM_TEX_SPECULAR] > iV_TEX_INVALID, "Could not load specularmap page %s", str.c_str());

		// pre read next token
		in >> str;
	}

	// MATERIAL ?
	if (!str.compare(WZM_MODEL_DIRECTIVE_MATERIAL))
	{
		in >> material[LIGHT_EMISSIVE][0] >> material[LIGHT_EMISSIVE][1] >> material[LIGHT_EMISSIVE][2] >>
		      material[LIGHT_AMBIENT][0] >> material[LIGHT_AMBIENT][1] >> material[LIGHT_AMBIENT][2] >>
		      material[LIGHT_DIFFUSE][0] >> material[LIGHT_DIFFUSE][1] >> material[LIGHT_DIFFUSE][2] >>
		      material[LIGHT_SPECULAR][0] >> material[LIGHT_SPECULAR][1] >> material[LIGHT_SPECULAR][2] >>
		      shininess;
		if (in.fail())
		{
			debug(LOG_WARNING, "WZM::read - Error reading material values");
			return false;
		}

		// pre read next token
		in >> str;
	}
	else
	{
		// Set default values
		memcpy(material, g_wzmRenderer.mat_default_reflections, sizeof(material));
		shininess = g_wzmRenderer.mat_default_shininess;
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
		m_meshes.push_back(WZMesh());
		WZMesh& mesh = m_meshes.back();

		if ( !(mesh.loadHeader(in) &&
		      mesh.loadGeometry(in, g_wzmRenderer)) ) // FIXME: renderer should be a property?
		{
			m_meshes.pop_back();
			return false;
		}
	}

	// get some compat values from the first mesh

	WZMesh& mesh0 = m_meshes.front();

	// FIXME: should be derived from all sub-meshes
	memcpy(m_aabb, mesh0.m_aabb, sizeof(m_aabb));

	float xmax = MAX(getAABBminmax(false).x, -getAABBminmax(true).x);
	float ymax = MAX(getAABBminmax(false).y, -getAABBminmax(true).y);
	float zmax = MAX(getAABBminmax(false).z, -getAABBminmax(true).z);

	radius = MAX(xmax, MAX(ymax, zmax));
	sradius = sqrtf(xmax*xmax + ymax*ymax + zmax*zmax);
	min = getAABBminmax(true);
	max = getAABBminmax(false);
	ocen = mesh0.m_tightspherecenter;

	nconnectors = mesh0.m_connectorArray.size();
	if (nconnectors)
	{
		connectors = new Vector3i[nconnectors];

		for (unsigned int i = 0; i < nconnectors; ++i)
		{
			connectors[i] = Vector3i(mesh0.m_connectorArray[i].x, mesh0.m_connectorArray[i].y, mesh0.m_connectorArray[i].z);
		}
	}

	return true;
}

Vector3f iIMDShape::getAABBcenter()
{
	Vector3f center;

	center.x = (m_aabb[0].x + m_aabb[4].x) / 2;
	center.y = (m_aabb[0].y + m_aabb[4].y) / 2;
	center.z = (m_aabb[0].z + m_aabb[4].z) / 2;

	return center;
}

void iIMDShape::draw(PIELIGHT colour, PIELIGHT teamcolour, int pieFlag, int pieFlagData)
{
	bool light = true;
	bool shaders = pie_GetShaderAvailability();
	SHADER_MODE shaderMode = SHADER_NONE;

	glErrors();

	pie_SetAlphaTest((pieFlag & pie_PREMULTIPLIED) == 0);

	/* Set fog status */
	if (!(pieFlag & pie_FORCE_FOG) &&
		(pieFlag & pie_ADDITIVE || pieFlag & pie_TRANSLUCENT || pieFlag & pie_BUTTON || pieFlag & pie_PREMULTIPLIED))
	{
		pie_SetFogStatus(false);
	}
	else
	{
		pie_SetFogStatus(true);
	}

	/* Set tranlucency */
	if (pieFlag & pie_ADDITIVE)
	{
		pie_SetRendMode(REND_ADDITIVE);
		colour.byte.a = (UBYTE)pieFlagData;
		light = false;
	}
	else if (pieFlag & pie_TRANSLUCENT)
	{
		pie_SetRendMode(REND_ALPHA);
		colour.byte.a = (UBYTE)pieFlagData;
		light = false;
	}
	else if (pieFlag & pie_PREMULTIPLIED)
	{
		pie_SetRendMode(REND_PREMULTIPLIED);
		light = false;
	}
	else
	{
		if (pieFlag & pie_BUTTON)
		{
			shaderMode = SHADER_BUTTON;
			light = false;

			pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);

			if (shaders)
			{
				pie_ActivateShader(shaderMode, this, teamcolour, colour);
			}
			else
			{
				pie_ActivateFallback(shaderMode, this, teamcolour, colour);
			}
		}
		pie_SetRendMode(REND_OPAQUE);
	}
	if (pieFlag & pie_ECM)
	{
		pie_SetRendMode(REND_ALPHA);
		light = true;
		pie_SetShaderEcmEffect(true);
	}

	if (light)
	{
		shaderMode = SHADER_COMPONENT;

		if (shaders)
		{
			pie_ActivateShader(shaderMode, this, teamcolour, colour);

		}
		else
		{
			pie_ActivateFallback(shaderMode, this, teamcolour, colour);
		}
	}
	else
	{
		glColor4ubv(colour.vector);     // Only need to set once for entire model
		pie_SetTexturePage(getTexturePage(WZM_TEX_DIFFUSE));
	}

	if (pieFlag & pie_HEIGHT_SCALED)	// construct
	{
		glScalef(1.0f, (float)pieFlagData / (float)pie_RAISE_SCALE, 1.0f);
	}
	if (pieFlag & pie_RAISE)		// collapse
	{
		glTranslatef(1.0f, (-max.y * (pie_RAISE_SCALE - pieFlagData)) * (1.0f / pie_RAISE_SCALE), 1.0f);
	}

	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glErrors();

	if (light)
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT, material[LIGHT_AMBIENT]);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material[LIGHT_DIFFUSE]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, material[LIGHT_SPECULAR]);
		glMaterialfv(GL_FRONT, GL_EMISSION, material[LIGHT_EMISSIVE]);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	}

	if (shaders && shaderMode == SHADER_COMPONENT)
	{
		pie_SetShaderTangentAttribute(&g_wzmRenderer.m_tangentArray[0]);
	}
	glTexCoordPointer(2, GL_FLOAT, 0, &g_wzmRenderer.m_textureArray[0]);
	glNormalPointer(GL_FLOAT, 0, &g_wzmRenderer.m_normalArray[0]);
	glVertexPointer(3, GL_FLOAT, 0, &g_wzmRenderer.m_vertexArray[0]);

	for (std::list<WZMesh>::iterator it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		const WZMesh& msh = *it;
		glDrawElements(GL_TRIANGLES, msh.m_indexArray.size() * 3, GL_UNSIGNED_INT, &msh.m_indexArray[0]);
	}

	glErrors();

	glPopClientAttrib();
	glPopAttrib();

	if (light || (pieFlag & pie_BUTTON))
	{
		if (shaders)
		{
			pie_DeactivateShader();
		}
		else
		{
			pie_DeactivateFallback();
		}
	}
	pie_SetShaderEcmEffect(false);

	if (pieFlag & pie_BUTTON)
	{
		pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
	}
}

void iIMDShape::drawFast(PIELIGHT colour, PIELIGHT teamcolour, int pieFlag)
{
	bool shaders = pie_GetShaderAvailability();

	if (shaders)
	{
		pie_ActivateShader(SHADER_COMPONENT, this, teamcolour, colour);
	}
	else
	{
		pie_ActivateFallback(SHADER_COMPONENT, this, teamcolour, colour);
	}

	if (pieFlag & pie_ECM)
	{
		pie_SetRendMode(REND_ALPHA);
		pie_SetShaderEcmEffect(true);
	}

	glMaterialfv(GL_FRONT, GL_AMBIENT, material[LIGHT_AMBIENT]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material[LIGHT_DIFFUSE]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material[LIGHT_SPECULAR]);
	glMaterialfv(GL_FRONT, GL_EMISSION, material[LIGHT_EMISSIVE]);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	for (std::list<WZMesh>::const_iterator mshit = m_meshes.begin(); mshit != m_meshes.end(); ++mshit)
	{
		const WZMesh& msh = *mshit;
		glDrawRangeElements(GL_TRIANGLES, msh.m_index_min, msh.m_index_max, msh.m_indexArray.size() * 3, GL_UNSIGNED_INT, &msh.m_indexArray[0]);
	}

	if (pieFlag & pie_ECM)
	{
		pie_SetRendMode(REND_OPAQUE);
		pie_SetShaderEcmEffect(false);
	}
}
