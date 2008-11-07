#ifndef ZINFO_H
#define ZINFO_H

#include "wstructs.h"

bool LineTypeHasTag(wad_ldtype_t type);
bool LineTypeHasNonvisualAction(wad_ldtype_t type);
bool IsACSSpecial(wad_ldtype_t type);
bool IsPolyobjSpecial(wad_ldtype_t type);

#endif
