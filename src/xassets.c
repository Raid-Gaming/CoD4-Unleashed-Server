/*
===========================================================================
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team

    This file is part of CoD4X17a-Server source code.

    CoD4X17a-Server source code is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    CoD4X17a-Server source code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================
*/



#include "q_shared.h"
#include "qcommon_io.h"
#include "cvar.h"
#include "xassets.h"
#include "sys_patch.h"
#include "qcommon_mem.h"
#include "cmd.h"
#include "xassets/xmodel.h"

#include <string.h>

void XAssetUsage_f();
void XAssetUsage2_f();
void XModelList_f();

void R_Init(){

        int XAssetscount;
        XZoneInfo XZoneInfoStack[6];

        XZoneInfoStack[4].fastfile = "localized_common_mp";
        XZoneInfoStack[4].loadpriority = 1;
        XZoneInfoStack[4].notknown = 0;
        XZoneInfoStack[3].fastfile = "common_mp";
        XZoneInfoStack[3].loadpriority = 4;
        XZoneInfoStack[3].notknown = 0;
        XZoneInfoStack[2].fastfile = "ui_mp";
        XZoneInfoStack[2].loadpriority = 8;
        XZoneInfoStack[2].notknown = 0;
        XZoneInfoStack[1].fastfile = "localized_code_post_gfx_mp";
        XZoneInfoStack[1].loadpriority = 0;
        XZoneInfoStack[1].notknown = 0;
        XZoneInfoStack[0].fastfile = "code_post_gfx_mp";
        XZoneInfoStack[0].loadpriority = 2;
        XZoneInfoStack[0].notknown = 0;

        if(DB_ModFileExists()){
            XAssetscount = 6;
            XZoneInfoStack[5].fastfile = "mod";
            XZoneInfoStack[5].loadpriority = 16;
            XZoneInfoStack[5].notknown = 0;
        }else{
            XAssetscount = 5;
        }
        DB_LoadXAssets(&XZoneInfoStack[0],XAssetscount,0);

        Cmd_AddCommand("XAssetUsage", XAssetUsage_f);
//        Cmd_AddCommand("XModelList", XModelList_f);
}

#define NUM_ASSETTYPES 33
#define g_poolsize_ADDR 0x82401a0
#define g_assetNames_ADDR 0x8274940
#define DB_XAssetPool_ADDR 0x8240100
#define DB_FreeXAssetHeaderHandler_ADDR 0x8240060
#define DB_DynamicCloneXAssetHandler_ADDR 0x823fe80

#define MAX_XMODELS 2000
#define MAX_GFXIMAGE 3800
#define MAX_WEAPON 196
#define MAX_FX 680

typedef enum{
        XModelPieces,
        PhysPreset,
        XAnimParts,
        XModel,	
        Material,
        TechinqueSet,		// techset
        GfxImage,		// image
        snd_alias_list_t,	// sound
        SndCurve,
        LoadedSound,
        Col_Map_sp,
        Col_Map_mp,
        Com_Map,
        Game_Map_sp,
        Game_Map_mp,
        MapEnts,
        GfxMaps,
        GfxLightDef,
        UIMaps,
        Font_s,
        MenuList,		//menufile
        menuDef_t,		//menu
        LocalizeEntry,
        WeaponDef,
        SNDDriversGlobals,
        FxEffectDef,		//fx
        FxImpactTable,		//impactfx
        AIType,
        MPType,
        Character,
        XModelAlias,
        RawFile,
        StringTable,
        NumXAssets
}XAssetType_t;


typedef void* XAssetHeader_t;

typedef union
{
  XAssetType_t type;
  void* nextHeader;
}XAssetHeaderType_t;


typedef struct
{
  XAssetHeaderType_t this;
  XAssetHeader_t header;
  unsigned short field_8;
  unsigned short nextListIndex;
  unsigned short field_C;
  unsigned short field_E;
}XAsset_t;


unsigned short *db_hashTable = (unsigned short *)0x14121680;
XAsset_t* g_assetEntryPool = (XAsset_t*)0x14131680;
void* *DB_XAssetPool = (void*)DB_XAssetPool_ADDR;
int *DB_XAssetPoolSize = (int*)g_poolsize_ADDR;
char* *DB_GetXAssetTypeName = (char**)g_assetNames_ADDR;


//Are that headers ? I'm not sure
typedef struct XModelAssetsHeader_s{
        struct XModelAssetsHeader_s*	next;
        XModel_t xmodel;
}XModelAssetsHeader_t;

typedef struct WeaponDefHeader_s{
        struct WeaponDefHeader_s*	next;
        char			data[2164];
}WeaponDefHeader_t;

typedef struct XAssetsHeaderCommon_s{
        struct XAssetsHeaderCommon_s*	next;
}XAssetsHeaderCommon_t;


int XAssetStdCount[NumXAssets];
int XAssetRequestedCount[NumXAssets];


int DB_GetXAssetStdCount(XAssetType_t type)
{
	return XAssetStdCount[type];
}


#ifndef COD4X17A

qboolean DB_XAssetNoAlloc(XAssetType_t i)
{
	if(i == Col_Map_sp)
		return qtrue;
	if(i == Col_Map_mp)
		return qtrue;
	if(i == Com_Map)
		return qtrue;
	if(i ==	Game_Map_sp)
		return qtrue;
	if(i == Game_Map_mp)
		return qtrue;
	if(i == MapEnts)
		return qtrue;
	if(i == GfxMaps)
		return qtrue;
	if(i == UIMaps)
		return qtrue;
	if(i == SNDDriversGlobals)
		return qtrue;
	if(i == AIType)
		return qtrue;
	if(i == MPType)
		return qtrue;
	if(i == Character)
		return qtrue;
	if(i == LocalizeEntry)
		return qtrue;
	if(i == XModelAlias)
		return qtrue;
	if(i == RawFile)
		return qtrue;
	if(i == menuDef_t)
		return qtrue;
	if(i == WeaponDef)
		return qtrue;
	if(i == StringTable)
		return qtrue;
		
	return qfalse;
}



void XAssetsInitStdCount()
{
	XAssetStdCount[XModelPieces] = 64;
	XAssetStdCount[PhysPreset] = 64;
	XAssetStdCount[XAnimParts] = 4096;
	XAssetStdCount[XModel] = 1000;
	XAssetStdCount[Material] = 2048;
	XAssetStdCount[TechinqueSet] = 1024;
	XAssetStdCount[GfxImage] = 2400;
	XAssetStdCount[snd_alias_list_t] = 16000;
	XAssetStdCount[SndCurve] = 64;
	XAssetStdCount[LoadedSound] = 1200;
	XAssetStdCount[Col_Map_sp] = 1;
	XAssetStdCount[Col_Map_mp] = 1;
	XAssetStdCount[Com_Map] = 1;
	XAssetStdCount[Game_Map_sp] = 1;
	XAssetStdCount[Game_Map_mp] = 1;
	XAssetStdCount[MapEnts] = 2;
	XAssetStdCount[GfxMaps] = 1;
	XAssetStdCount[GfxLightDef] = 32;
	XAssetStdCount[UIMaps] = 0;
	XAssetStdCount[Font_s] = 16;
	XAssetStdCount[MenuList] = 128;
	XAssetStdCount[menuDef_t] = 1280;
	XAssetStdCount[LocalizeEntry] = 6144;
	XAssetStdCount[WeaponDef] = 720;
	XAssetStdCount[SNDDriversGlobals] = 1;
	XAssetStdCount[FxEffectDef] = 400;
	XAssetStdCount[FxImpactTable] = 4;
	XAssetStdCount[AIType] = 0;
	XAssetStdCount[MPType] = 0;
	XAssetStdCount[Character] = 0;
	XAssetStdCount[XModelAlias] = 0;
	XAssetStdCount[RawFile] = 1024;
	XAssetStdCount[StringTable] = 100;
}

cvar_t* r_xassetnum;

void DB_ParseRequestedXAssetNum()
{

	char toparse[1024];
	const char* typename;
	char* scanpos;
	char scanstring[64];
	int i, count;

	r_xassetnum = Cvar_RegisterString("r_xassetnum", "", CVAR_INIT, "The list of xassets with their count in the key=value key=value... format");

	Com_Memcpy(XAssetRequestedCount, XAssetStdCount, sizeof(XAssetRequestedCount));
	Com_sprintf(toparse, sizeof(toparse), " %s", r_xassetnum->string);
	
	for(i = 0;  i < NumXAssets; ++i)
	{		

		if(DB_XAssetNoAlloc(i) || i == menuDef_t || i == WeaponDef || i == StringTable)
		{
			continue;
		}
		
		typename = DB_GetXAssetTypeName[ i ];
		
		Com_sprintf(scanstring, sizeof(scanstring), " %s=", typename);
		
		scanpos = strstr(toparse, scanstring);
		if(scanpos == NULL)
		{
			continue;
		}
		
		scanpos += strlen(scanstring);
		
		count = atoi(scanpos);
		if(count < 1 || count > 65535)
		{
			continue;
		}
		
		if(count <= DB_GetXAssetStdCount(i))
		{
			continue;
		}
		XAssetRequestedCount[i] = count;
	}
}


void DB_CustomAllocOnce(XAssetType_t type)
{
	int count = DB_GetXAssetStdCount(type);
	int typesize = DB_GetXAssetTypeSize(type);
	void *alloc;

	alloc = Z_Malloc(count * typesize);
	if(alloc)
	{
		DB_XAssetPool[type] = alloc;
		DB_XAssetPoolSize[type] = count;
	}
}

void DB_RelocateXAssetMem()
{
	int i, typesize, count;
	
	void* newmem;
	
	for(i = 0;  i < NumXAssets; ++i)
	{
		if(DB_XAssetNoAlloc(i))
		{
			continue;
		}

		if(XAssetRequestedCount[i] <= DB_XAssetPoolSize[i])
		{
			//Only allocate if we need more than what is already allocated
			continue;
		}
		
		count = XAssetRequestedCount[i];
		typesize = DB_GetXAssetTypeSize(i);
		
		newmem = Z_Malloc(count * typesize);
		
		if(newmem == NULL)
		{
			continue;
		}

		Com_Printf("^2Reallocate %d XAssets on request of type: %s\n", count, DB_GetXAssetTypeName[i]);
		DB_XAssetPool[i] = newmem;
		DB_XAssetPoolSize[i] = count;
	}
}


void XAssets_PatchLimits(){

        void* ptrpool, *ptrsize;

        int size = NUM_ASSETTYPES * sizeof(void*);

        ptrpool = &DB_XAssetPool[0];
	ptrsize = &DB_XAssetPoolSize[0];

	if(!Sys_MemoryProtectWrite(ptrpool, size) || !Sys_MemoryProtectWrite(ptrsize, size))
	{
		Com_Error(ERR_FATAL,"XAssets_PatchLimits: Failed to change memory to writeable\n");
	}

	XAssetsInitStdCount();
	DB_ParseRequestedXAssetNum();

	DB_CustomAllocOnce( menuDef_t );
	DB_CustomAllocOnce( WeaponDef );
	DB_CustomAllocOnce( StringTable );

	DB_RelocateXAssetMem();

	if(!Sys_MemoryProtectReadonly(ptrpool, size) || !Sys_MemoryProtectReadonly(ptrsize, size))
	{
		Com_Error(ERR_FATAL,"XAssets_PatchLimits: Failed to change memory to read only\n");
	}
}
#endif

typedef struct
{
  char field_0[54];
}IDirect3DVertexBuffer9;

typedef struct
{
  char field_0[54];
}IDirect3DIndexBuffer9;


typedef struct
{
  char field_0[72];
  int field_48;
  int field_4C;
  IDirect3DVertexBuffer9 *vertexbuffer;
  IDirect3DIndexBuffer9 *indexbuffer;
}XZoneMemory;


typedef struct
{
  char fastfilename[64];
  int index;
  int mem_index;
  XZoneMemory zonememory;
  int field_A0;
  int field_A4;
}g_zones_struct;


typedef struct
{
  char fastfilename[64];
  int index;
}g_zoneinfo_struct;

typedef struct
{
  struct unk_xasset_struct1 *field_0;
  int field_4;
  short field_8;
  short field_A;
  int field_C;
}unk_xasset_struct1;

void R_UnlockVertexBuffer(IDirect3DVertexBuffer9* vertexbuf){}
void R_FreeStaticVertexBuffer(IDirect3DVertexBuffer9* vertexbuf){}
void R_UnlockIndexBuffer(IDirect3DIndexBuffer9* indexbuf){}
void R_FreeStaticIndexBuffer(IDirect3DIndexBuffer9* indexbuf){}
void Material_DirtyTechniqueSetOverrides(){};

#define g_zoneCount (*(int*)(0x14121654))
#define g_archiveBuf (*(byte*)(0x141b1694))
#define g_unknownVar_1 (*(int*)(0x14121650))

g_zones_struct *g_zones = (g_zones_struct*)0x141200a0;
byte *g_zoneHandles = (byte*)0x14121660;

void DB_FreeXZoneMemory(XZoneMemory* zonemem)
{
	if(zonemem->vertexbuffer)
	{
		R_UnlockVertexBuffer(zonemem->vertexbuffer);
		R_FreeStaticVertexBuffer(zonemem->vertexbuffer);
	}

	if(zonemem->indexbuffer)
	{
		R_UnlockIndexBuffer(zonemem->indexbuffer);
		R_FreeStaticIndexBuffer(zonemem->indexbuffer);
	}
	Com_Memset(zonemem->field_0, 0, sizeof(zonemem->field_0));
}

void DB_UnloadFastFile2(g_zones_struct* zone)
{
        DB_FreeXZoneMemory(&zone->zonememory);
        Com_Printf("Unloaded fastfile %s\n", zone->fastfilename);
        PMem_Free(zone->fastfilename, zone->mem_index);
        zone->fastfilename[0] = 0;
}


void DB_UnloadFastFile(byte loadmask)
{
	byte bitmask;
	int i;
	
	for(bitmask = 64; bitmask > 0 ; bitmask >>= 1)
	{
		if(bitmask == 2)
			bitmask >>= 1;
		
		if(!(loadmask & bitmask))
			continue;

		for ( i = g_zoneCount - 1; i >= 0; --i )
		{
			if ( g_zones[g_zoneHandles[i]].index & bitmask )
			{
				DB_UnloadFastFile2(&g_zones[g_zoneHandles[i]]);
				g_zoneCount--;
				if (i < g_zoneCount)
				{
					memcpy(&g_zoneHandles[i], &g_zoneHandles[i + 1], g_zoneCount - i);
				}
			}
		}
	}
}

/* No multithreading here :P */
void _InterlockedExchangeAdd(int* lockedvar, int num)
{
    *lockedvar += num;
}

void DB_LoadXAssets_Hook(XZoneInfo *zoneinfo, unsigned int assetscount)
{

	int i;

	DB_FreeUnusedResources();
	for(i = 0; i < assetscount; i++)
	{
		DB_UnloadFastFile( zoneinfo[i].notknown );
	}
	_InterlockedExchangeAdd(&g_unknownVar_1, -1);
	g_archiveBuf = 0;
	DB_LoadSounds();
	DB_LoadDObjs();
	Material_DirtyTechniqueSetOverrides();
	BG_FillInAllWeaponItems();

}



/*

void __cdecl DB_ReferencedFastFiles(char* g_zoneSumList, char* g_zoneNameList, int maxsize)
{
	int i;
	g_zones_struct *zone;
	char fileName[MAX_OSPATH];
	int v4;
	int filesize;
	char ospath[MAX_OSPATH];
	char checkSum[64];

	g_zoneSumList[0] = 0;
	g_zoneNameList[0] = 0;

	for(i = 0, zone = g_zones; i < 32; i++, ++zone;)
	{
		if ( zone->fastfilename[0] == '\0' || !Q_strncmp(zone->fastfilename, "localized_", 10) )
		{
			continue;
		}
		if ( g_zoneSumList[0] )
		{
			Q_strncat(g_zoneSumList, maxsize, " ");
		}
		if ( g_zoneNameList[0] )
		{
			Q_strncat(g_zoneNameList, maxsize, " ");
		}
		itoa(zone->checkSum, checkSum, 10);
		Q_strncat(g_zoneSumList, maxsize, checkSum);
		
		if ( zone->ff_dir == 1 )
		{
          Q_strncat(g_zoneNameList, maxsize, fs_gameDirVar->string);
          Q_strncat(g_zoneNameList, maxsize, "/");
          Q_strncat(g_zoneNameList, maxsize, zone->fastfilename);  
		  continue;
		}		
		if ( zone->ff_dir != 2 )
		{
			Q_strncat(g_zoneNameList, maxsize, zone->fastfilename);  
			continue;
		}
			
		if ( !(com_dedicated->integer) )
		{
			continue;
		}
        
		Com_sprintf(fileName, sizeof(fileName), "%s_load", zone->fastfilename);
        DB_BuildOSPath(fileName, 2, sizeof(ospath), ospath);
        v4 = CreateFileA(ospath);
        
		if ( v4 == -1 )
        {
			continue;
		}
		
        filesize = GetFileSize(v4, 0);
        CloseHandle(v4);
        if ( !filesize )
        {
			continue;
		}
		Q_strncat(g_zoneSumList, maxsize, " ");
        itoa(filesize, checkSum, 10);
        Q_strncat(g_zoneSumList, maxsize, checkSum);

	    Q_strncat(g_zoneNameList, maxsize, " ");
		Q_strncat(g_zoneNameList, maxsize, "usermaps");
		Q_strncat(g_zoneNameList, maxsize, "/");
		Q_strncat(g_zoneNameList, maxsize, zone->fastfilename);
		Q_strncat(g_zoneNameList, maxsize, "_load");

	}
}

char zoneSumList[MAX_STRING_CHARS];
char zoneFileList[MAX_STRING_CHARS];

const char* DB_ReferencedFFChecksums()
{
	DB_ReferencedFastFiles(zoneSumList, zoneFileList, sizeof(zoneFileList));
	return zoneSumList;
}

const char* DB_ReferencedFFNameList()
{
	DB_ReferencedFastFiles(zoneSumList, zoneFileList, sizeof(zoneFileList));
	return zoneFileList;
}
*/





void DB_EnumXAssets(XAssetType_t type, void (__cdecl *callback)(XAssetHeader_t *, void *), void* cbargs, qboolean a4)
{
  int index, sindex, i;
  XAsset_t *listselector, *slistselect;
/*
  InterlockedIncrement(&db_hashCritSect);
  while ( xasset_interlock )
  {
    Sys_Sleep(0);
  }
*/
  for(i = 0; i < 32768; ++i)
  {
    for(index = db_hashTable[i]; index; index = listselector->nextListIndex)
    {
	listselector = &g_assetEntryPool[index];

	if ( listselector->this.type != type )
		continue;
		
	callback(listselector->header, cbargs);
	if ( !a4 )
		continue;
	
	for(sindex = listselector->field_C; sindex; sindex = slistselect->field_C)
	{
            slistselect = &g_assetEntryPool[sindex];
            callback(slistselect->header, cbargs);
	}
    }
  }
/*
  InterlockedDecrement();
*/
}

void DB_PrintXAsset(XAssetHeader_t *header, void *none)
{
    XModel_t* xmodelhead = (XModel_t*)header;

    Com_Printf("%s\n", xmodelhead->name);
}

void XModelList_f()
{

    Com_Printf("XModel list:\n");
    Com_Printf("Name                          \n");
    Com_Printf("------------------------------\n");

    DB_EnumXAssets(XModel, DB_PrintXAsset, NULL, qtrue);

    Com_Printf("\n");
}


void DB_CountXAssets(int *count, int len ,qboolean a4)
{
  int index, sindex, i;
  XAsset_t *listselector, *slistselect;
  XAssetType_t type;
/*
  InterlockedIncrement(&db_hashCritSect);
  while ( xasset_interlock )
  {
    Sys_Sleep(0);
  }
*/
  if((len / sizeof(int)) < NumXAssets)
  {
    return;
  }

  Com_Memset(count, 0, len);

  for(i = 0; i < 32768; ++i)
  {
    for(index = db_hashTable[i]; index; index = listselector->nextListIndex)
    {
	listselector = &g_assetEntryPool[index];

	type = listselector->this.type;

	if(type < 0 || type >= NumXAssets)
	{
		continue;
	}

	++count[type];

	if ( !a4 )
		continue;
	
	for(sindex = listselector->field_C; sindex; sindex = slistselect->field_C)
	{
            slistselect = &g_assetEntryPool[sindex];
	    ++count[type];
	}
    }
  }
/*
  InterlockedDecrement();
*/
}

#ifndef COD4X17A

void DB_BuildOverallocatedXAssetList(char* configstring, int len)
{
    int i;
    char cstring[64];
    int countlist[NumXAssets];

    DB_CountXAssets(countlist, sizeof(countlist), qtrue);

    configstring[0] = '\0';

    for(i = 0; i < NumXAssets; ++i)
    {
        if(DB_XAssetNoAlloc(i))
        {
            continue;
        }

        if(DB_GetXAssetStdCount(i) >= countlist[i])
        {
            continue;
        }

        if(countlist[i] < 1)
        {
            continue;
        }

        Com_sprintf(cstring, sizeof(cstring), "%s=%d ", DB_GetXAssetTypeName[i], countlist[i]);
        Q_strcat(configstring, len, cstring);
    }

}


#endif


void XAssetUsage_f()
{
    int assettype, j, l;
    int countlist[NumXAssets];


    Com_Printf("XAsset usage:\n");
    Com_Printf("Name                 Used  Free \n");
    Com_Printf("-------------------- ----- -----\n");

    DB_CountXAssets(countlist, sizeof(countlist), qtrue);

    for(assettype = 0; assettype < NumXAssets; assettype++)
    {
	Com_Printf("%s", DB_GetXAssetTypeName[assettype]);

	l = 20 - strlen(DB_GetXAssetTypeName[assettype]);
	j = 0;

	do
	{
		Com_Printf (" ");
		j++;
	} while(j < l);

	Com_Printf(" %5d %5d\n", countlist[assettype], DB_XAssetPoolSize[assettype] - countlist[assettype]);

    }
    Com_Printf("\n");

}


/*


void XAssetUsage_f()
{
    int i, assettype, j, l;

    XAssetsHeaderCommon_t *header;

    Com_Printf("XAsset usage:\n");
    Com_Printf("Name                 Used  Free \n");
    Com_Printf("-------------------- ----- -----\n");

    for(assettype = 0; assettype < NumXAssets; assettype++)
    {

	header = DB_XAssetPool[assettype];

	for(i = 0; i < DB_XAssetPoolSize[assettype]; i++)
	{
	    if(header == NULL)
	        break;

	    else
	        header = header->next;
	}

	Com_Printf("%s", DB_GetXAssetTypeName[assettype]);

	l = 20 - strlen(DB_GetXAssetTypeName[assettype]);
	j = 0;

	do
	{
		Com_Printf (" ");
		j++;
	} while(j < l);


	Com_Printf(" %5d %5d\n", DB_XAssetPoolSize[assettype] - i, i);


    }
    Com_Printf("\n");
}

*/


