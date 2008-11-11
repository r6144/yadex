#include "yadex.h"
#include "zinfo.h"
#include "game.h"

bool LineTypeHasTag(wad_ldtype_t type)
{
  return GetSpecialTagMask(type) & 1;
}

/* FIXME: Should exclude most non-activated action specials */
bool LineTypeHasNonvisualAction(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return (type != 0);
  else return (type > 0 && ! ((type >= 181 && type <= 189) /* slope, mirror, panning, etc. */
			      || type == 9 || type == 160 /* horizon, 3D floor */
			      || (type >= 100 && type <= 103) /* scrolling the current line */
			      || (type >= 209 && type <= 214) || type == 216 || (type >= 218 && type <= 220)
			      || type == 50 /* transfer sector parameters */));
}

// Is it a level exit?
bool IsExitSpecial(wad_ldtype_t type)
{
  if (yg_level_format != YGLF_HEXEN) return (type == 11 || type == 51 || type == 52 || type == 124);
  else return (type == 243 || type == 244);
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
  /* FIXME: Also Thing type 9081 */
  if (yg_level_format != YGLF_HEXEN) return 0;
  switch (type) {
  case 39: case 70: case 71:
  case 119: case 127: case 128: case 130: case 131: case 132: case 133: case 134: case 135: case 136: case 137: case 180: case 237:
    return 1;
  case 227:
    return 2;
  case 76: case 125: case 176: case 177: case 229:
    return 3;
  case 77:
    return 7;
  case 139: case 9072: case 9073: case 9074:
    return 8;
  case 120:
    return 16;
  case 78:
    return 22;
  case 175: case 178:
    return 24;
  default: return 0;
  }
}

/* "tag" refers to sector tags only; Note that non-special things (e.g. slopes) may also have sector tags */
unsigned GetSpecialTagMask(unsigned type)
{
  if (yg_level_format != YGLF_HEXEN) return (type > 0) ? 1 : 0;
  switch (type) {
  case 10: case 11: case 12: case 13: case 14:
  case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29:
  case 30: case 31: case 32: case 34: case 35: case 36: case 38:
  case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 50:
  case 60: case 61: case 62: case 63: case 64: case 65: case 66: case 67: case 68: case 69:
  case 78: case 94: case 95: case 96:
  case 110: case 111: case 112: case 113: case 114: case 115: case 116: case 117:
  case 138: case 140: case 172: case 185: case 186: case 187: case 188: case 189: case 190:
  case 192: case 193: case 194: case 195: case 196: case 197: case 198: case 199: case 200: case 201:
  case 202: case 203: case 204: case 205: case 206: case 207: case 209: case 210: case 211:
  case 212: case 213: case 214: case 216: case 217: case 218: case 219: case 220:
  case 227: case 228: case 230: case 231: case 232: case 233: case 234:
  case 235: case 236: case 238: case 239: case 240: case 241: case 242: case 245: case 246: case 247:
  case 249: case 250: case 251: case 252: case 253: case 254: case 255:
    return 1;
  case 39: case 70:
    return 2;
  case 71:
    return 4;
  default: return 0;
  }
  
}

unsigned GetSpecialLineIDMask(unsigned type)
{
  /* FIXME: Thing types 9500 and 9501 also uses a line ID, but it is only for sloping */
  if (yg_level_format != YGLF_HEXEN) return 0;
  switch (type) {
  case 121: case 183: case 184: case 208: case 221: case 222: case 223: case 224:
    return 1;
  case 215: return 3;
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
