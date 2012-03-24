/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2012  Warzone 2100 Project

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

#include "lib/ivis_opengl/ivi.h"
#include "lib/ivis_opengl/piemode.h"
#include "lib/ivis_opengl/tex.h"
#include "lib/ivis_opengl/textdraw.h"
#include "lib/ivis_opengl/wzm.h"

extern WZRenderer g_wzmRenderer;

// pass in true to reset the palette too.
bool iV_Reset()
{
	_TEX_INDEX = 0;

	// Reset WZM renderer
	g_wzmRenderer.shutdown();
	if ( !(g_wzmRenderer.init() && g_wzmRenderer.loadMaterialDefaults("3dview.ini")) )
	{
		return false;
	}

	return true;
}

void iV_ShutDown()
{
	g_wzmRenderer.shutdown();
	pie_ShutDown();
	pie_TexShutDown();
	iV_TextShutdown();
	debug(LOG_3D, "iVi[ShutDown] = successful");
}
