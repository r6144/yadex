#include "yadex.h"
#include "zinfo.h"
#include "game.h"

bool LineTypeHasTag(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return (type != 0);
  else return (type > 0 && ! (type == 181 /* slope */
			      || (type >= 80 && type <= 84) || type == 226 /* script */
			      || type == 121 /* lineid */));
}

bool LineTypeHasNonvisualAction(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return (type != 0);
  else return (type > 0 && ! (type == 181 /* slope */
			      || (type >= 100 && type <= 103) /* scroll */
			      || type == 121 /* lineid */));
}

bool IsACSSpecial(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return false;
  return (type >= 80 && type <= 84) || type == 226;
}

bool IsPolyobjSpecial(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return false;
  return ((type >= 1 && type <= 8) || (type >= 90 && type <= 93));
}

/* Which of the special's arguments are Thing IDs?  Bit 0..4 of the result corresponds to arg1,...,arg5. */
unsigned GetSpecialTIDMask(unsigned type)
{
  if (yg_level_format != YGLF_HEXEN) return 0;
  switch (type) {
  case 119: case 127: case 128: case 180: case 9081:
    return 1;
  case 76: case 125: case 176: case 177:
    return 3;
  case 77:
    return 7;
  case 139: case 9072: case 9073: case 9074:
    return 8;
  case 78:
    return 22;
  case 175: case 178:
    return 24;
  default: return 0;
  }
}

bool MatchSpecialArg(unsigned target, unsigned mask, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5)
{
  if (mask == 0) return false;
  return
    ((mask & 1) && arg1 == target)
    || ((mask & 2) && arg2 == target)
    || ((mask & 4) && arg3 == target)
    || ((mask & 8) && arg4 == target)
    || ((mask & 16) && arg5 == target);
}
