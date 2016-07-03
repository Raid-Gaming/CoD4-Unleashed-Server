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
#include "cmd.h"
#include "qcommon_io.h"
#include "cvar.h"
#include "sys_net.h"
#include "server.h"
#include "g_shared.h"
#include "punkbuster.h"
#include "sys_main.h"
#include "q_platform.h"
#include "sec_crypto.h"
#include "sys_patch.h"

#include <string.h>
#include <stdlib.h>

#ifdef PUNKBUSTER


qboolean PbInitFailure;

#pragma pack(push, 1)

typedef struct pbsv_object_s
{
  int field_0;
  int field_4;
  void* hLibModule;
  void* hLib2Module;
  void* hLib3Module;
  char consoleprefix[32];
  char pbPath[256];
  int field_134;
  int wantdisable;
  int (__cdecl *PbSvGameCommand)(const char *cmd, char *arg);
  char* (__cdecl *PbSvGameQuery)(int type, char* string);
  int (__cdecl *PbSvGameMsg)(const char *msg);
  int (__cdecl *PbSvSendToClient)(int msgsize, char* message, int clientnum);
  int (__cdecl *sb)(struct pbsv_object_s *this, int event, int clnum, int maxstringlen, char* string, int);
  void (__cdecl *sa)(struct pbsv_object_s *this, int);
  int (__cdecl *PbSvSendToAddrPort)(char* netdest, unsigned short port, int msgsize, char* message);
  int (__cdecl *ConnectStringTrap)(struct pbsv_object_s *this, const char* netadr, const char* connectString);
  const char *(__cdecl *AuthClient)(struct pbsv_object_s *this, const char *, int, const char *);
  void (__cdecl *CaptureConsole)(struct pbsv_object_s *this, const char *msg, int buflen);
  int dword_D8B2ECC;
}pbsv_object_t;


#pragma pack(pop)

#define pbsv (*((pbsv_object_t*)(0x8879820)))

#ifndef MAX_PACKETLEN
#define MAX_PACKETLEN 1400
#endif

/*********************************************************************************************/
//		Mandatory functions for PunkBuster operation
/*********************************************************************************************/

char* g_ConsoleCaptureBuf;
int g_ConsoleCaptureBufLen;


__cdecl int PbSvSendToAddrPort(char* netdest, unsigned short port, int msgsize, char* message){

    netadr_t netadr;
    mvabuf;


    NET_StringToAdr(va("%s:%i", netdest, port), &netadr, NA_UNSPEC);

    netadr.sock = 0;

    netadr_t *sockadr;

    if((sockadr = NET_GetDefaultCommunicationSocket()) != NULL)
        netadr.sock = sockadr->sock;

    NET_SendPacket(NS_SERVER, msgsize, message, &netadr);
    return 0;
}

__cdecl int PbSvSendToClient(int msgsize, char* message, int clientnum){
    client_t *cl;
    cl = &svs.clients[clientnum];

    if(cl->state >= CS_CONNECTED){

        byte string[MAX_PACKETLEN];
        int i;

        // set the OutOfBand header
        string[0] = 0xff;
        string[1] = 0xff;
        string[2] = 0xff;
        string[3] = 0xff;

        if(msgsize + 4 > MAX_PACKETLEN){
            Com_PrintWarning("Buffer Overflow in NET_OutOfBandData %i bytes\n", msgsize);
            return 0;
        }

        for ( i = 0; i < msgsize ; i++ ) {
            string[i+4] = message[i];
        }

        NET_SendPacket( NS_SERVER, i+4, string, &cl->netchan.remoteAddress );

    }
    return 0;
}


__cdecl char* PbSvGameQuery(int para_01, char* string){

    int maxclients;
    client_t *cl;
    gclient_t *gclient;
    string[255] = 0;
    int		var_01;
    if(!string) return NULL;
    switch(para_01){
	case 0x65:
            maxclients = sv_maxclients->integer;
	    if(!maxclients) return 0;
	    *string = 0x30;
	    Com_sprintf(string, 255, "%i", maxclients);
	    return 0;

	case 0x66:
	    maxclients = sv_maxclients->integer;
	    var_01 = atoi(string);
	    Com_Memset(string, 0, 0x68);
	    if(var_01 < 0 || var_01 > maxclients) return "PB_Error: Query Failed";
	    cl = &svs.clients[var_01];

	    if(cl->state < CS_ACTIVE || cl->noPb == qtrue) return "PB_Error: Query Failed";
	    Q_strncpyz(string, cl->name, 254);
	    Q_strncpyz(&string[33], cl->pbguid, 221);
	    Q_strncpyz(&string[66], NET_AdrToString(&cl->netchan.remoteAddress), 188);
	    return NULL;

	case 0x67:
	    Q_strncpyz(string, Cvar_GetVariantString(string),255);
	    return NULL;

	case 0x72:

	    maxclients = sv_maxclients->integer;
	    *string = 0;
	    var_01 = atoi(string);

	    if(var_01 < 0 || var_01 > maxclients) return "PB_Error: Query Failed";;

	    cl = &svs.clients[var_01];
	    gclient = &level.clients[para_01];

	    if(cl->state < CS_ACTIVE || cl->noPb) return "PB_Error: Query Failed";;

	    Com_sprintf(string,255,"ping=%d score=%d", cl->ping, gclient->pers.scoreboard.score);
	    return NULL;

	default:
	    return NULL;
    }
}

void PbCaptureConsoleOutput_wrapper(const char *msg, int msglen)
{
    PbCaptureConsoleOutput(msg, 4096);

}


void PB_DropClient(unsigned int clientnum, const char* dropmsg)
{
    char dropmsgbuf[MAX_STRING_CHARS];

    if(clientnum > MAX_CLIENTS)
        return;

    Q_strncpyz(dropmsgbuf, dropmsg, sizeof(dropmsgbuf));

    Q_strchrrepl(dropmsgbuf, '"', '\'');

    SV_DropClient(&svs.clients[clientnum], dropmsgbuf);

}

void PBCvar_Set(const char* name, const char* value)
{
    if(Cvar_FindVar( name ))
    {
	Cvar_SetLatched(name, value);
    }
}

void PbSvUnload()
{
    pbsv.field_4 = 0;
    pbsv.sa = 0;
    pbsv.sb = 0;
    pbsv.ConnectStringTrap = 0;
    pbsv.AuthClient = 0;
    pbsv.CaptureConsole = 0;
    if ( pbsv.hLibModule )
    {
        Sys_CloseLibrary(pbsv.hLibModule);
    }
    pbsv.hLibModule = 0;
}

void PbSvBuildHomePath(char* ospath)
{
	ospath[0] = '\0';
	char pathsep[2];

	pathsep[0] = PATH_SEP;
	pathsep[1] = '\0';

	if ( pbsv.PbSvGameQuery == NULL )
	{
		return;
	}
	
	Q_strncpyz(ospath, "fs_homepath", MAX_OSPATH);
	pbsv.PbSvGameQuery(103, ospath);

	if(ospath[0] == '\0')
	{
		Q_strncpyz(ospath, Sys_DefaultInstallPath( ), MAX_OSPATH - 5 );
	}
	
	if ( ospath[0] )
    {
		if ( ospath[strlen(ospath) -1] != PATH_SEP )
		{
			Q_strcat(ospath, MAX_OSPATH, pathsep);
		}
    }
	Q_strcat(ospath, MAX_OSPATH, "pb");
	Q_strcat(ospath, MAX_OSPATH, pathsep);
}


void PbSvBuildBasePath(char* ospath)
{
	ospath[0] = '\0';
	char pathsep[2];

	pathsep[0] = PATH_SEP;
	pathsep[1] = '\0';

	if ( pbsv.PbSvGameQuery == NULL )
	{
		return;
	}
	Q_strncpyz(ospath, "fs_basepath", MAX_OSPATH);
	pbsv.PbSvGameQuery(103, ospath);
    
	if ( ospath[0] )
    {
		if ( ospath[strlen(ospath) -1] != PATH_SEP )
		{
			Q_strcat(ospath, MAX_OSPATH, pathsep);
		}
    }
	Q_strcat(ospath, MAX_OSPATH, "pb");
	Q_strcat(ospath, MAX_OSPATH, pathsep);

}



void PbSvMakeHomepathCopy(const char *modulename, char *basepath)
{
	char dstpath[512];
	char srcpath[512];
	
	Q_strncpyz(dstpath, pbsv.pbPath, sizeof( dstpath ) );
	Q_strcat( dstpath, sizeof(dstpath), modulename );

	Q_strncpyz(srcpath, basepath, sizeof( srcpath ) );
	Q_strcat( srcpath, sizeof(srcpath), modulename );	

	/* File exists ? */
	if ( FS_FileExistsOSPath( dstpath ) == qtrue )
	{
		return;
	}
	/* Does not exist! Copy it from basepath */

	FS_CopyFile( srcpath, dstpath );
}

const char* PbSv6makefn(char *outmodname, char *modulename)
{
	char basepath[MAX_OSPATH];

	if ( !pbsv.pbPath[0] )
	{
		PbSvBuildHomePath( pbsv.pbPath );
		PbSvBuildBasePath( basepath );

		if ( Q_stricmp(basepath, pbsv.pbPath) && basepath[0] && pbsv.pbPath[0] )
		{
		  Sys_Mkdir( pbsv.pbPath );
		  Sys_Mkdir( basepath );
		  PbSvMakeHomepathCopy("pbsv" DLL_EXT, basepath);
		  PbSvMakeHomepathCopy("pbcl" DLL_EXT, basepath);
		  PbSvMakeHomepathCopy("pbag" DLL_EXT, basepath);
		}
	}
	Q_strncpyz(outmodname, pbsv.pbPath, MAX_OSPATH);
	Q_strcat(outmodname, MAX_OSPATH, modulename);
	return outmodname;
}

void PB_SystemDummy(const char* cmdline)
{
	Com_PrintWarning("system( %s ) of PunkBuster is blocked while running in securemode\n", cmdline);
}

#define PBSV_HASH "4f16daaf68bafdf97a27330317c59a5472821e201dcd0f0d"
#define PBAG_HASH "c020bcabbcb8aff43f054b5864e2f640e80a6d0b1ea006ee"
#define PBCL_HASH "1684c2da4b734b7aa489accd83f5da8a78f0b88e3723506b"

int PbSVLoadModule()
{
	char hash[128];
	long unsigned sizeofhash;

	char modname[256];
	char modname_dest[256];
	const char* dlname;
	const char* dlname2;
	byte* imagebase;

	if ( pbsv.hLibModule )
	{
		return 0;
	}
	
	PbSvUnload();

	if ( FS_FileExistsOSPath( PbSv6makefn(modname, "pbsvnew" DLL_EXT) ) == qtrue )
	{
		Sys_Chmod( PbSv6makefn(modname, "pbsvold" DLL_EXT), 0777);
		FS_RemoveOSPath( PbSv6makefn(modname, "pbsvold" DLL_EXT) );
		rename( PbSv6makefn(modname, "pbsv" DLL_EXT),  PbSv6makefn(modname_dest, "pbsvold" DLL_EXT) );
		Sys_Chmod( PbSv6makefn(modname, "pbsv" DLL_EXT), 0777);
		FS_RemoveOSPath( PbSv6makefn(modname, "pbsv" DLL_EXT) );
		rename( PbSv6makefn(modname, "pbsvnew" DLL_EXT), PbSv6makefn(modname_dest, "pbsv" DLL_EXT) );
	}

	if(com_securemode)
	{

		dlname = PbSv6makefn(modname, "pbag" DLL_EXT);
		dlname2 = PbSv6makefn(modname_dest, "pbags" DLL_EXT);

		if ( FS_FileExistsOSPath( dlname ) == qtrue )
		{
			FS_RemoveOSPath( dlname2 );
			rename( dlname ,dlname2 );
			FS_RemoveOSPath( dlname );
		}

		hash[0] = '\0';
		sizeofhash = sizeof(hash);

		Sec_HashFile(SEC_HASH_TIGER, dlname2, hash, &sizeofhash, qfalse);

		if(Q_stricmp(hash ,PBAG_HASH))
		{
			Com_Printf("Tiger = %s\n", hash);
			Com_PrintError("%s checksum missmatch! PunkBuster will not startup in securemode when the checksum is invalid.\n", dlname2);
			return 1;
		}


		dlname = PbSv6makefn(modname, "pbcl" DLL_EXT);
		dlname2 = PbSv6makefn(modname_dest, "pbcls" DLL_EXT);

		if ( FS_FileExistsOSPath( dlname ) == qtrue )
		{
			FS_RemoveOSPath( dlname2 );
			rename( dlname ,dlname2 );
			FS_RemoveOSPath( dlname );
		}

		hash[0] = '\0';
		sizeofhash = sizeof(hash);

		Sec_HashFile(SEC_HASH_TIGER, dlname2, hash, &sizeofhash, qfalse);

		if(Q_stricmp(hash ,PBCL_HASH))
		{
			Com_Printf("Tiger = %s\n", hash);
			Com_PrintError("%s checksum missmatch! PunkBuster will not startup in securemode when the checksum is invalid.\n", dlname2);
			return 1;
		}

		dlname2 = PbSv6makefn(modname, "pbsv" DLL_EXT);

		hash[0] = '\0';
		sizeofhash = sizeof(hash);

		Sec_HashFile(SEC_HASH_TIGER, dlname2, hash, &sizeofhash, qfalse);

		if(Q_stricmp(hash ,PBSV_HASH))
		{
			Com_Printf("Tiger = %s\n", hash);
			Com_PrintError("%s checksum missmatch! PunkBuster will not startup in securemode when the checksum is invalid.\n", dlname2);
			return 1;
		}

	}

	dlname = PbSv6makefn(modname, "pbsv" DLL_EXT);
	pbsv.hLibModule = Sys_LoadLibrary( dlname );

	if ( pbsv.hLibModule == NULL )
	{
		return 1;
	}

	if(com_securemode)
	{

		/* Remove system() */
		imagebase = LIBRARY_ADDRESS_BY_HANDLE( pbsv.hLibModule );

		if(Sys_MemoryProtectWrite( (void*)(imagebase + 0x4c6e0), 0xdfdf7 ) == qfalse)
		{
			return 1;
		}

		SetCall((DWORD)(imagebase + 0x114346), PB_SystemDummy);
		SetCall((DWORD)(imagebase + 0xc23cb), PB_SystemDummy);

		if(Sys_MemoryProtectExec( (void*)(imagebase + 0x4c6e0), 0xdfdf7 ) == qfalse)
		{
			return 1;
		}
	}


	pbsv.sa = Sys_GetProcedure( "sa" );
    pbsv.sb = Sys_GetProcedure( "sb" );
	
    if ( pbsv.sa && pbsv.sb ){
    
		pbsv.wantdisable = 0;
		return 0;
    }else{
	
        PbSvUnload();
	return 1;
	}

}

void PbSv10AddPbEvent(pbsv_object_t *this, int eventType, int clnum, int maxstringsize, char *string, int a6)
{	
	if ( !pbsv.PbSvGameCommand || PbInitFailure )
		return;
	
	if ( !pbsv.wantdisable )
	{
	  
		if ( pbsv.hLibModule )
		{
			pbsv.sb(this, eventType, clnum, maxstringsize, string, a6);
			return;
		}
		
	}else{
	
		if ( pbsv.hLibModule )
		{
			PbSvUnload();
			return;
		}
	}
		
	if ( PbSVLoadModule() )
		return;
		
	pbsv.sb(this, eventType, clnum, maxstringsize, string, a6);
}


void PbServerProcessEvents(int type)
{
	if ( pbsv.PbSvGameCommand == NULL )
	{
		return;
	}
	
    if ( pbsv.hLibModule )
    {
		if ( pbsv.wantdisable )
		{
			PbSvUnload();
			return;
		}
		
		pbsv.sa(&pbsv, type);
		return;
    }
    
    if ( pbsv.wantdisable )
	{
		PbSv10AddPbEvent(&pbsv, 16, -1, 0, "", 0);
	}
	
}

void PbServerForceProcess()
{
    PbServerProcessEvents( -1 );
}



int __cdecl PbSvGameCommand(const char *cmd, char *arg)
{
  char *strptr;
  char *split;
  char oldchar;

  if ( !Q_stricmp(cmd, "set_sv_punkbuster") )
  {
    if(arg[0] == '0')
	{
		Cvar_SetBoolByName("sv_punkbuster", 0);
	}else{
		Cvar_SetBoolByName("sv_punkbuster", 1);		
	}
    return 0;
  }

  if ( !Q_stricmp(cmd, "ConCapBufLen") )
  {
    g_ConsoleCaptureBufLen = (int)arg;
    return 0;
  }
  
  if ( !Q_stricmp(cmd, "ConCapBuf") )
  {
    g_ConsoleCaptureBuf = (char*)arg;
    return 0;
  }
  
  if ( !Q_stricmp(cmd, "Cmd_Exec") )
  {
    Cmd_ExecuteSingleCommand(0, 0, arg);
    if ( !Q_stricmpn(arg, "pb_", 3u) )
    {
      PbServerProcessEvents( -1 );
    }
	return 0;
  }
  
  strptr = arg;
  
  while( strptr[0] == ' ')
		++strptr;

  while( strptr[0] != '\0' && strptr[0] != ' ')
		++strptr;

  split = strptr;

  while( strptr[0] == ' ')
	++strptr;
  
  if ( !Q_stricmp(cmd, "DropClient") )
  {
      PB_DropClient(atoi(arg), strptr);
      return 0;
  }
  
  if ( !Q_stricmp(cmd, "Cvar_Set") || !Q_stricmp(cmd, "Dvar_Set") )
  {
      oldchar = *split;
      *split = 0;
      PBCvar_Set(arg, strptr);
      *split = oldchar;
  }
  return 0;
}

void PbServerTrapConsole(const char *msg, int buflen)
{
  if ( pbsv.CaptureConsole )
  {
	pbsv.CaptureConsole(&pbsv, msg, buflen);
  }
}

void PbCaptureConsoleOutput(const char *msg, int buflen)
{
  unsigned int len;

  //PbClientTrapConsole(msg, buflen);
  PbServerTrapConsole(msg, buflen);
  
  if ( g_ConsoleCaptureBuf )
  {
    len = strlen(g_ConsoleCaptureBuf);
    if ( (signed int)(strlen(msg) + 1 + len - 1) < g_ConsoleCaptureBufLen )
	{
      strcpy(&g_ConsoleCaptureBuf[len], msg);
	}
  }
}


const char * PbAuthClient(const char *netadr, qboolean clhaspunkbuster, const char *guid)
{
  const char *errormsg;

  errormsg = NULL;

  if ( pbsv.AuthClient )
  {
    errormsg = pbsv.AuthClient(&pbsv, netadr, clhaspunkbuster, guid);
  }

  return errormsg;
}

void PbPassConnectString(const char *netadrStr, const char *connectString)
{
  if ( pbsv.ConnectStringTrap )
    pbsv.ConnectStringTrap(&pbsv, netadrStr, connectString);
}

void DisablePbSv()
{
  PbSv10AddPbEvent(&pbsv, 118, -1, 0, 0, 0);
}

void EnablePbSv()
{
  PbSv10AddPbEvent(&pbsv, 117, -1, 0, 0, 0);
  Cmd_ExecuteSingleCommand(0, 0, "pb_sv_guidrelax 7");
}

void PbServerCompleteCommand(char *string, int len)
{
  PbSv10AddPbEvent(&pbsv, 51, -1, len, string, 0);
}




void PbSvAddEvent(int eventType, int clnum, int lenstr, char * str)
{
	PbSv10AddPbEvent(&pbsv, eventType, clnum, lenstr, str, 0);
}

int __cdecl PbSvGameMsg(const char *msg)
{
/*  if ( Q_stricmpn(pbsv.consoleprefix, "[skipnotify]", 12) )
    PbMsgToScreen(pbsv.consoleprefix, msg);
  else
*/
    Com_Printf("%s: %s\n", pbsv.consoleprefix, msg);
    return 0;
}



qboolean PbServerInitialize()
{
	pbsv.PbSvGameCommand = PbSvGameCommand;
	pbsv.PbSvGameQuery = PbSvGameQuery;
	pbsv.PbSvGameMsg = PbSvGameMsg;
	pbsv.PbSvSendToClient = PbSvSendToClient;
	pbsv.PbSvSendToAddrPort = PbSvSendToAddrPort;

	PbSv10AddPbEvent(&pbsv, 16, -1, 0, "", 0);
	
	if ( !pbsv.sb )
	{
		Cvar_SetBoolByName("sv_punkbuster", 0);
		PbInitFailure = qtrue;
		return qfalse;
	}
	return qtrue;
}


void PbSvDestructor()
{
  pbsv.field_4 = 0;
  pbsv.sa = 0;
  pbsv.sb = 0;
  pbsv.ConnectStringTrap = 0;
  pbsv.AuthClient = 0;
  pbsv.CaptureConsole = 0;
  if ( pbsv.hLibModule )
    Sys_CloseLibrary(pbsv.hLibModule);
  pbsv.hLibModule = 0;
  if ( pbsv.hLib2Module )
    Sys_CloseLibrary(pbsv.hLib2Module);
  pbsv.hLib2Module = 0;
  pbsv.dword_D8B2ECC = 0;
  if ( pbsv.hLib3Module )
    Sys_CloseLibrary(pbsv.hLib3Module);
  pbsv.hLib3Module = 0;
}

void PbSvConstructor()
{
    pbsv.field_0 = 0x357AFE32u;
    strcpy(pbsv.consoleprefix, "^3PunkBuster Server");
    pbsv.hLibModule = 0;
    pbsv.wantdisable = 1;
    pbsv.PbSvGameCommand = 0;
    pbsv.PbSvGameQuery = 0;
    pbsv.PbSvGameMsg = 0;
    pbsv.PbSvSendToClient = 0;
    pbsv.field_4 = 0;
    pbsv.sb = 0;
    pbsv.sa = 0;
    pbsv.PbSvSendToAddrPort = 0;
    pbsv.ConnectStringTrap = 0;
    pbsv.AuthClient = 0;
    pbsv.CaptureConsole = 0;
}

#endif


