/*
===========================================================================
        Copyright (c) 2015-2019 atrX of Raid Gaming
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team
    Copyright (C) 1999-2005 Id Software, Inc.

    This file is part of CoD4-Unleashed-Server source code.

    CoD4-Unleashed-Server source code is free software: you can redistribute it
and/or modify it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    CoD4-Unleashed-Server source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================
*/

#ifndef __XASSETS_H__
#define __XASSETS_H__

#include "q_shared.h"

typedef struct {
  const char* fastfile;
  int loadpriority;
  int notknown;
} XZoneInfo;

void R_Init();
void __cdecl DB_SetInitializing(qboolean);
byte __cdecl DB_FileExists(const char* filename, int mode);
qboolean __cdecl DB_ModFileExists(void);
void __cdecl DB_LoadXAssets(XZoneInfo*, unsigned int assetscount, int);
int __cdecl DB_GetXAssetTypeSize(int type);
void __cdecl XAnimInit(void);
void XAssets_PatchLimits(void);
void DB_LoadSounds();
void DB_LoadDObjs();
void Material_DirtyTechniqueSetOverrides();
void BG_FillInAllWeaponItems();
void DB_FreeUnusedResources();
void DB_LoadXAssets_Hook(XZoneInfo*, unsigned int assetscount);
void DB_BuildOverallocatedXAssetList(char* configstring, int len);
#endif
