#include "yadex.h"
#include "zinfo.h"
#include "game.h"

bool LineTypeHasTag(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return (type != 0);
  else return (type > 0 && ! (type == 181 /* slope */
			      || (type >= 80 && type <= 84) /* script */
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
  return (type >= 80 && type <= 84);
}

bool IsPolyobjSpecial(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return false;
  return ((type >= 1 && type <= 8) || (type >= 90 && type <= 93));
}
