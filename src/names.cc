/*
 *	names.cc
 *	BW & RQ sometime in 1993 or 1994.
 */


/*
This file is part of Yadex.

Yadex incorporates code from DEU 5.21 that was put in the public domain in
1994 by Raphaël Quinet and Brendon Wyber.

The rest of Yadex is Copyright © 1997-2003 André Majorel and others.

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
#include "game.h"
#include "objid.h"


/*
   get the name of an object type
*/
const char *GetObjectTypeName (int objtype)
{
switch (objtype)
   {
   case OBJ_THINGS:   return "thing";
   case OBJ_LINEDEFS: return "linedef";
   case OBJ_SIDEDEFS: return "sidedef";
   case OBJ_VERTICES: return "vertex";
   case OBJ_SEGS:     return "segment";
   case OBJ_SSECTORS: return "ssector";
   case OBJ_NODES:    return "node";
   case OBJ_SECTORS:  return "sector";
   case OBJ_REJECT:   return "reject";
   case OBJ_BLOCKMAP: return "blockmap";
   case OBJ_SCRIPT:   return "script";
   case OBJ_POLYOBJ:  return "polyobject";
   case OBJ_TID:      return "TID";
   case OBJ_TAG:      return "tag";
   }
return "< Bug! >";
}



/*
   what are we editing?
*/
const char *GetEditModeName (int objtype)
{
switch (objtype)
   {
   case OBJ_THINGS:   return "Things";
   case OBJ_LINEDEFS:
   case OBJ_SIDEDEFS: return "LD & SD";
   case OBJ_VERTICES: return "Vertices";
   case OBJ_SEGS:     return "Segments";
   case OBJ_SSECTORS: return "Seg-Sectors";
   case OBJ_NODES:    return "Nodes";
   case OBJ_SECTORS:  return "Sectors";
   }
return "< Bug! >";
}



/*
   get a short (16 char.) description of the type of a linedef
*/

string GetLineDefTypeName (int type)
{
  if (type >= 0x2f80 && type <= 0x7fff) { /* Generalized linedef; see boomref.txt */
    enum gt_e { gt_floor, gt_ceil, gt_door, gt_locked_door, gt_lift, gt_stairs, gt_crusher } gtype;
    unsigned trigger = 0, speed = 0, model = 0, monster = 0, change = 0;
    string name;

    if (type >= 0x6000) gtype = gt_floor;
    else if (type >= 0x4000) gtype = gt_ceil;
    else if (type >= 0x3c00) gtype = gt_door;
    else if (type >= 0x3800) gtype = gt_locked_door;
    else if (type >= 0x3400) gtype = gt_lift;
    else if (type >= 0x3000) gtype = gt_stairs;
    else gtype = gt_crusher;
    trigger = type & 7; speed = (type >> 3) & 3;
    if (gtype == gt_floor || gtype == gt_ceil) {
      change = (type >> 10) & 3;
      if (change) model = (type >> 5) & 1; else monster = (type >> 5) & 1;
    } else if (gtype == gt_lift || gtype == gt_stairs || gtype == gt_crusher) monster = (type >> 5) & 1;
    else if (gtype == gt_door) monster = (type >> 7) & 1;
    
    name += "WSGD"[trigger>>1]; name += "1R"[trigger&1]; if (monster) name += 'm';  name += ' ';
    if (gtype == gt_floor || gtype == gt_ceil) {
      static const char *const n_floor[] = {"F->HnF", "F->LnF", "F->NnF", "F->LnC", "F->C", "FbysT", "Fby24", "Fby32"};
      static const char *const n_ceil[] = {"C->HnC", "C->LnC", "C->NnC", "C->HnF", "C->F", "CbysT", "Cby24", "Cby32"};
      unsigned direction = (type >> 6) & 1, target = (type >> 7) & 7, crush = (type >> 12) & 1;
      name += (gtype == gt_floor ? n_floor : n_ceil)[target];
      name += (direction == 0) ? " Dn" : " Up"; name += "SNFT"[speed];
      if (change) { name += " c"; name += " 0TS"[change]; name += "tn"[model]; }
      if (crush) name += " Cr";
    } else if (gtype == gt_door || gtype == gt_locked_door) {
      static const char *const n_delay[] = {"1", "4", "9", "30"};
      unsigned kind = (gtype == gt_door) ? ((type >> 5) & 3) : ((type >> 5) & 1);
      unsigned delay = (gtype == gt_door) ? ((type >> 8) & 3) : 1;
      if (kind == 1 || kind == 3) name += (kind == 1) ? "Opn" : "Cls";
      else { name += (kind == 0) ? "OpnD" : "ClsD"; name += n_delay[delay]; name += (kind == 0) ? "Cls" : "Opn"; }
      name += ' '; name += "SNFT"[speed];
      if (gtype == gt_locked_door) {
	static const char *const n_lock_ne[] = {"Any", "RC", "BC", "YC", "RS", "BS", "YS", "All6"};
	static const char *const n_lock_e[] = {"Any", "RK", "BK", "YK", "RK", "BK", "YK", "All3"};
	unsigned lock = (type >> 6) & 7, sk_eq_ck = (type >> 9) & 1;
	name += ' '; name += (sk_eq_ck ? n_lock_e : n_lock_ne)[lock];
      }
    } else if (gtype == gt_lift) {
      static const char *const n_lift[] = {"F->LnFD", "F->NnFD", "F->LnCD", "HnF<->LnFD"};
      static const char *const n_delay[] = {"1", "3", "5", "10"};
      unsigned target = (type >> 8) & 3, delay = (type >> 6) & 3;
      name += "Lft "; name += n_lift[target]; name += n_delay[delay]; name += ' '; name += "SNFT"[speed];
    } else if (gtype == gt_stairs) {
      static const char *const n_step[] = {"s4", "s8", "s16", "s24"};
      unsigned step = (type >> 6) & 3, direction = (type >> 8) & 1, igntxt = (type >> 9) & 1;
      name += "Stair "; name += (direction == 0) ? "Dn " : "Up "; name += n_step[step]; name += ' '; name += "SNFT"[speed];
      if (igntxt) name += " Ign";
    } else { /* crusher */
      unsigned silent = (type >> 6) & 1;
      name += "Crusher "; name += "SNFT"[speed]; if (silent) name += " Silent";
    }
    return name;
  }
if (CUR_LDTDEF != NULL && CUR_LDTDEF->number == type)
  return CUR_LDTDEF->shortdesc;
for (al_lrewind (ldtdef); ! al_leol (ldtdef); al_lstep (ldtdef))
  if (CUR_LDTDEF->number == type)
    return CUR_LDTDEF->shortdesc;
return "??  UNKNOWN";
}


/*
   get a short description of the flags of a linedef
*/

const char *GetLineDefFlagsName (int flags)
{
static char buf[20];
// "P" is a Boom extension ("pass through")
// "T" is for Strife ("translucent")
const char *flag_chars = (yg_level_format != YGLF_HEXEN) ? "???T??PANBSLU2MI" : "e?MaaaRANBSLU2MI";
int n;

char *p = buf;
for (n = 0; n < 16; n++)
   {
   if (n != 0 && n % 4 == 0)
      *p++ = ' ';
   if (flags & (0x8000u >> n))
      *p++ = flag_chars[n];
   else
      *p++ = '-';
   }
*p = '\0';
return buf;
}



/*
   get a long description of one linedef flag
*/

const char *GetLineDefFlagsLongName (int flags)
{
if (flags & 0x1000) return "Translucent [Strife]";
if (flags & 0x200)  return "Pass-through [Boom]";
if (flags & 0x100)  return "Always shown on the map";
if (flags & 0x80)   return "Never shown on the map";
if (flags & 0x40)   return "Blocks sound";
if (flags & 0x20)   return "Secret (shown as normal on the map)";
if (flags & 0x10)   return "Lower texture is \"unpegged\"";
if (flags & 0x08)   return "Upper texture is \"unpegged\"";
if (flags & 0x04)   return "Two-sided (may be transparent)";
if (flags & 0x02)   return "Monsters cannot cross this line";
if (flags & 0x01)   return "Impassible";
return "UNKNOWN";
}



/*
   get a short (14 char.) description of the type of a sector
*/

const char *GetSectorTypeName (int type)
{
/* KLUDGE: To avoid the last element which is bogus */
if (al_ltell (stdef) == al_lcount (stdef) - 1)
  al_lrewind (stdef);

if (CUR_STDEF != NULL && CUR_STDEF->number == type)
  return CUR_STDEF->shortdesc;
for (al_lrewind (stdef); ! al_leol (stdef); al_lstep (stdef))
  if (CUR_STDEF->number == type)
    return CUR_STDEF->shortdesc;
static char buf[30];
sprintf (buf, "UNKNOWN (%d)", type);
return buf;
}



/*
   get a long description of the type of a sector
*/

const char *GetSectorTypeLongName (int type)
{
/* KLUDGE: To avoid the last element which is bogus */
if (al_ltell (stdef) == al_lcount (stdef) - 1)
  al_lrewind (stdef);

if (CUR_STDEF != NULL && CUR_STDEF->number == type)
  return CUR_STDEF->longdesc;
for (al_lrewind (stdef); ! al_leol (stdef); al_lstep (stdef))
  if (CUR_STDEF->number == type)
    return CUR_STDEF->longdesc;
static char buf[30];
sprintf (buf, "UNKNOWN (%d)", type);
return buf;
}
