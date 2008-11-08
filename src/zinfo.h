#ifndef ZINFO_H
#define ZINFO_H

#include "wstructs.h"

bool LineTypeHasTag(wad_ldtype_t type);
bool LineTypeHasNonvisualAction(wad_ldtype_t type);
bool IsExitSpecial(wad_ldtype_t type);
bool IsACSSpecial(wad_ldtype_t type);
bool IsPolyobjSpecial(wad_ldtype_t type);
unsigned GetSpecialTIDMask(unsigned type);
unsigned GetSpecialTagMask(unsigned type);
bool MatchSpecialArg(unsigned target, unsigned mask, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5);

#endif
