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

#ifndef __INCLUDED_SRC_BUCKET3D_H__
#define __INCLUDED_SRC_BUCKET3D_H__

enum RENDER_TYPE
{
	RENDER_DROID,
	RENDER_STRUCTURE,
	RENDER_FEATURE,
	RENDER_PROXMSG,
	RENDER_PROJECTILE,
	RENDER_SHADOW,
	RENDER_ANIMATION,
	RENDER_EFFECT,
	RENDER_DELIVPOINT,
	RENDER_PARTICLE
};

/** Add an object to the list.
 * Note: Droids, shadows, structures,
 * animations and features which are not
 * in the view frustum should be culled before hand.
 */
void bucketAddTypeToList(RENDER_TYPE objectType, void *object);

/// Render Objects in list in reverse Z order
void bucketRenderCurrentList(void);

#endif // __INCLUDED_SRC_BUCKET3D_H__
