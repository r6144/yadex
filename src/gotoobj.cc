/*
 *	gotoobj.cc
 *	AYM 1998-09-06
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Rapha�l Quinet and Brendon Wyber.

The rest of Yadex is Copyright � 1997-2003 Andr� Majorel and others.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#include "yadex.h"
#include "gfx.h"
#include "gotoobj.h"
#include "levels.h"
#include "objects.h"
#include "objid.h"
#include "x_hover.h"


/*
 *	CenterMapAroundCoords
 *	Change the view so that the map coordinates (xpos, ypos)
 *	appear at the centre of the window
 */
void CenterMapAroundCoords (int xpos, int ypos)
{
OrigX = xpos;
OrigY = ypos;
/* AYM 1998-09-20 This is a DOS-ism. IIRC, the ICCCM says an
   application program should generally not mess with the
   pointer. */
}


/*
 *	focus_on_map_coords
 *	Change the view so that the map coordinates (xpos, ypos)
 *	appear under the pointer
 */
void focus_on_map_coords (int x, int y)
{
OrigX = x - (MAPX (is.x) - OrigX);
OrigY = y - (MAPY (is.y) - OrigY);
}


/*
 *	sector_under_pointer
 *	Convenience function
 */
inline int sector_under_pointer ()
{
Objid o;
GetCurObject (o, OBJ_SECTORS, MAPX (is.x), MAPY (is.y));
return o.num;
}


/*
  centre the map around the object and zoom in if necessary
*/

void GoToObject (const Objid& objid) /* SWAP! */
{
int   xpos, ypos;
int   xpos2, ypos2;
int   sd1, sd2;
float oldscale;

GetObjectCoords (objid.type, objid.num, &xpos, &ypos);
focus_on_map_coords (xpos, ypos);
oldscale = Scale;

/* I ifdef'd out this block because the only thing it really
   does is (uselessly) messing the zoom factor up when there are
   superimposed objects and we're trying to focus on the
   highest-numbered (and therefore hidden) one. AYM 1999-07-26 */

/* Special case for sectors: if a sector contains other sectors,
   or if its shape is such that it does not contain its own
   geometric centre, zooming in on the centre won't help. So I
   choose a linedef that borders the sector and focus on a point
   between the centre of the linedef and the centre of the
   sector. If that doesn't help, I try another linedef.

   This algorithm is not perfect but it works rather well with
   most well-constituted sectors. It does not work so well for
   unclosed sectors, though (but it's partly GetCurObject()'s
   fault). */
if (objid.type == OBJ_SECTORS && sector_under_pointer () != objid.num)
   {
   /* restore the Scale */
   Scale = oldscale;
   for (int n = 0; n < NumLineDefs; n++)
      {
      ObjectsNeeded (OBJ_LINEDEFS, 0);
      sd1 = LineDefs[n].sidedef1;
      sd2 = LineDefs[n].sidedef2;
      ObjectsNeeded (OBJ_SIDEDEFS, 0);
      if (sd1 >= 0 && SideDefs[sd1].sector == objid.num
	|| sd2 >= 0 && SideDefs[sd2].sector == objid.num)
	 {
	 GetObjectCoords (OBJ_LINEDEFS, n, &xpos2, &ypos2);
	 int d = ComputeDist (abs (xpos - xpos2), abs (ypos - ypos2)) / 7;
	 if (d <= 1)
	   d = 2;
	 xpos = xpos2 + (xpos - xpos2) / d;
	 ypos = ypos2 + (ypos - ypos2) / d;
	 focus_on_map_coords (xpos, ypos);
	 if (sector_under_pointer () == objid.num)
	    break;
	 }
      }
   }
}
