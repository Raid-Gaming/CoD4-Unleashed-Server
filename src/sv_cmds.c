/*
===========================================================================
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team
    Copyright (C) 1999-2005 Id Software, Inc.

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



/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/

#include "q_shared.h"
#include "qcommon_io.h"
#include "qcommon.h"
#include "cmd.h"
#include "server.h"
#include "g_shared.h"
#include "g_sv_shared.h"
#include "nvconfig.h"
#include "httpftp.h"
#include "qcommon_mem.h"
#include "sys_thread.h"
#include "sys_main.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    SAY_CHAT,
    SAY_CONSOLE,
    SAY_SCREEN
} consaytype_t;

/*
==================
SV_ConSay
==================
*/
static void SV_ConSay(client_t *cl, consaytype_t contype) {
	char	*p;
	char	text[1024];
	char	cmd_argbuf[MAX_STRING_CHARS];

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc () < 2 ) {
		Com_Printf( "Usage: command text... \n" );
		return;
	}
	*text = 0;
	if(cl){
	    if(contype == SAY_CHAT){
		strcpy (text, sv_contellname->string);
	    }else{
		strcpy (text, "^5PM: ^7");
	    }
	    p = Cmd_Argsv(2, cmd_argbuf, sizeof(cmd_argbuf));
	}else{
	    if(contype == SAY_CHAT){
		strcpy (text, sv_consayname->string);
	    }
	    p = Cmd_Args(cmd_argbuf, sizeof(cmd_argbuf));
	}
	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	switch(contype){

	case SAY_CHAT:
		SV_SendServerCommand(cl, "h \"%s\"", text);
	break;
	case SAY_CONSOLE:
		SV_SendServerCommand(cl, "e \"%s\"", text);
	break;
	case SAY_SCREEN:
		SV_SendServerCommand(cl, "c \"%s\"", text);
	break;
	}
}



typedef struct{
	int uid;
	client_t *cl;
}clanduid_t;



static void SV_GetPlayerByHandleInternal( const char* s, clanduid_t* cl) {
	client_t	*lastfound;
	int		i, playermatches;
	char		cleanName[64];

	cl->uid = 0;
	cl->cl = NULL;

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		return;
	}


        if((s[0] == '@' || s[0] == 'u') && isNumeric(&s[1], 0)){

		cl->uid = atoi(&s[1]);
		if(cl->uid < 1){
			cl->uid = 0;
			Com_Printf("Invalid UID specified.\n");
			return;
		}

        }else{

        //    cl = SV_Cmd_GetPlayerByHandle();

		cl->cl = NULL;

		// Check whether this is a numeric player handle
		for(i = 0; s[i] >= '0' && s[i] <= '9'; i++);

		if(!s[i])
		{
			int plid = atoi(s);

			// Check for numeric playerid match
			if(plid >= 0 && plid < sv_maxclients->integer)
			{
				cl->cl = &svs.clients[plid];

				if(!cl->cl->state)
					cl->cl = NULL;
			}
		}
		if(!cl->cl){

			if(strlen(s) < 3){ //Don't process too short names
				Com_Printf( "Player %s is not on the server\n", s );
				cl->cl = NULL;
				return;
			}

			playermatches = 0; //This must be one
			lastfound = NULL;

			// check for a exact name match
			for ( i=0, cl->cl=svs.clients ; i < sv_maxclients->integer ; i++, cl->cl++ ) {
				if ( !cl->cl->state ) {
					continue;
				}
				if ( !Q_stricmp( cl->cl->name, s ) ) {
					lastfound = cl->cl;
					playermatches++;
					continue;
				}

				Q_strncpyz( cleanName, cl->cl->name, sizeof(cleanName) );
				Q_CleanStr( cleanName );
				if ( !Q_stricmp( cleanName, s ) ) {
					lastfound = cl->cl;
					playermatches++;
					continue;
				}
			}

			if(!lastfound){ //No exact playermatch found - Now search for partial name matches
				for ( i=0, cl->cl=svs.clients ; i < sv_maxclients->integer ; i++, cl->cl++ ) {
					if ( !cl->cl->state ) {
						continue;
					}
					if ( Q_stristr( cl->cl->name, s ) ) {
						lastfound = cl->cl;
						playermatches++;
						continue;
					}

					Q_strncpyz( cleanName, cl->cl->name, sizeof(cleanName) );
					Q_CleanStr( cleanName );
					if ( Q_stristr( cleanName, s ) ) {
						lastfound = cl->cl;
						playermatches++;
						continue;
					}
				}
			}

			if(!lastfound){ //Still nothing found!
				Com_Printf( "Player %s is not on the server\n", s );
				cl->cl = NULL;
				return;
			}else if(playermatches != 1){
				Com_Printf( "Dup player matches!\n", s );
				cl->cl = NULL;
				return;
			}else{
				cl->cl = lastfound;
			}
		}
                cl->uid = cl->cl->uid;
        }

        if(!cl->cl && cl->uid > 0){ //See whether this player is currently onto server
            for(i = 0, cl->cl=svs.clients; i < sv_maxclients->integer; i++, cl->cl++){
                if(cl->cl->state && cl->uid == cl->cl->uid){
                    break;
                }
            }
            if(i == sv_maxclients->integer)
                cl->cl = NULL;
        }
	
}


/*
 ==================
 SV_Cmd_GetPlayerByHandle
 
 Returns the player with player id or name from Cmd_Argv(1)
 ==================
 */


static clanduid_t SV_Cmd_GetPlayerByHandle( void ) {
	clanduid_t	cl;
	char		*s;
	
	cl.uid = 0;
	cl.cl = NULL;
	
	// make sure server is running
	if ( !com_sv_running->boolean ) {
		Com_Printf("Server is not running\n");
		return cl;
	}
	
	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return cl;
	}
	
	s = Cmd_Argv(1);
	
	SV_GetPlayerByHandleInternal(s, &cl);
	return cl;
}

/*
 ==================
 SV_Cmd_GetPlayerByHandle
 
 Returns the player with player id or name from Cmd_Argv(1)
 ==================
 */


int SV_GetPlayerUIDByHandle( const char* handle)
{
	clanduid_t	cl;

	SV_GetPlayerByHandleInternal(handle, &cl);
	return cl.uid;
}

client_t* SV_GetPlayerClByHandle( const char* handle)
{
	clanduid_t	cl;
	
	SV_GetPlayerByHandleInternal(handle, &cl);
	return cl.cl;
}

const char* SV_GetPlayerNameByHandle( const char* handle)
{
	clanduid_t	cl;
	
	SV_GetPlayerByHandleInternal(handle, &cl);
	if(cl.cl)
	{
		return cl.cl->name;
	}
	return "";
}

/*
==================
SV_GetPlayerByNum

Returns the player with idnum from Cmd_Argv(1)
==================
*/
static client_t *SV_GetPlayerByNum( void ) {
	client_t	*cl;
	int			i;
	int			idnum;
	char		*s;

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		return NULL;
	}

	if ( Cmd_Argc() < 2 ) {
		Com_Printf( "No player specified.\n" );
		return NULL;
	}

	s = Cmd_Argv(1);

	for (i = 0; s[i]; i++) {
		if (s[i] < '0' || s[i] > '9') {
			Com_Printf( "Bad slot number: %s\n", s);
			return NULL;
		}
	}
	idnum = atoi( s );
	if ( idnum < 0 || idnum >= sv_maxclients->integer ) {
		Com_Printf( "Bad client slot: %i\n", idnum );
		return NULL;
	}

	cl = &svs.clients[idnum];
	if ( !cl->state ) {
		Com_Printf( "Client %i is not active\n", idnum );
		return NULL;
	}
	return cl;
}


/*
==================
SV_ConSayChat_f
==================
*/

static void SV_ConSayChat_f(void) {
    SV_ConSay(NULL,SAY_CHAT);
}

/*
==================
SV_ConSayConsole_f
==================
*/

static void SV_ConSayConsole_f(void) {
    SV_ConSay(NULL,SAY_CONSOLE);
}

/*
==================
SV_ConSayScreen_f
==================
*/

static void SV_ConSayScreen_f(void) {
    SV_ConSay(NULL,SAY_SCREEN);
}

/*
==================
SV_ConTell
==================
*/

static void SV_ConTell( consaytype_t contype) {

    client_t *cl;

    if ( Cmd_Argc() < 3 ) {
	Com_Printf( "1. Usage: tellcommand clientnumber text... \n2. Usage: tellcommand \"client by name\" text...\n" );
	return;
    }
    cl = SV_Cmd_GetPlayerByHandle().cl;

    if(cl != NULL){
        SV_ConSay(cl,contype);
    }
}

/*
==================
SV_ConTellScreen_f
==================
*/
static void SV_ConTellScreen_f(void) {
    SV_ConTell(SAY_SCREEN);
}

/*
==================
SV_ConTellConsole_f
==================
*/
static void SV_ConTellConsole_f(void) {
    SV_ConTell(SAY_CONSOLE);
}

/*
==================
SV_ConTellChat_f
==================
*/
static void SV_ConTellChat_f(void) {
    SV_ConTell(SAY_CHAT);
}


/*
===========
SV_DumpUser_f

Examine all a users info strings FIXME: move to game
===========
*/

#define MAX_NICKNAMES 6

static void SV_DumpUser_f( void ) {
	clanduid_t	cl;
	char		*guid;

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Com_Printf ("Usage: dumpuser <userid>\n");
		return;
	}

	guid = SV_IsGUID(Cmd_Argv(1));
	if(guid){
		cl.cl = NULL;
		cl.uid = 0;
	}else{
		cl = SV_Cmd_GetPlayerByHandle();
		if ( !cl.uid && !cl.cl ) {
			return;
		}
	}

	Com_Printf( "\nuserinfo\n" );
	Com_Printf( "----------------------------------------------------\n" );

	if(cl.cl){

		Info_Print( cl.cl->userinfo );

		Com_Printf ("pbguid               %s\n", cl.cl->pbguid );

		switch(cl.cl->authentication){
		    case 1:
			Com_Printf ("authentication       %s\n", AUTHORIZE_SERVER_NAME);
		    break;
		    case 0:
			Com_Printf ("authentication       %s timed out\n", AUTHORIZE_SERVER_NAME);
		    break;
		    case -1:
			Com_Printf ("authentication       Plugin or N/A\n");
		    break;
		    default:
			Com_Printf ("authentication       unknown\n");
		}
		if(cl.cl->OS == 'M'){
			Com_Printf ("OperatingSystem      Mac OS X\n");
		}else if(cl.cl->OS == 'W'){
			Com_Printf ("OperatingSystem      Windows\n");
		}else{
			Com_Printf ("OperatingSystem      Unknown\n");
		}
		if(cl.cl->uid > 0){
			Com_Printf ("PlayerUID            %i\n",cl.cl->uid);
		}else{
			Com_Printf ("PlayerUID            N/A\n");
		}

		if(cl.cl->uid <= 0)
			return;
        } else {
		Com_Printf("Player is not on server.\n");
        }

//	InsertPluginEvent
/*
	if(psvs.useuids != 1)
		return;


	Com_Printf( "\nExtended info\n" );
	Com_Printf( "----------------------------------------------------\n" );

	if(cl.uid || guid){

		*infostring = 0;
		if(cl.uid)
			Info_SetValueForKey( infostring, "UID", va("%i",cl.uid));

		else
			Info_SetValueForKey( infostring, "GUID", guid);

		if(svse.cmdInvoker.currentCmdInvoker)
			CL_AddReliableCommand(&svse.authserver, va("getUserinfo Ticket %i \"%s\"", svs.clients[SV_RemoteCmdGetInvokerClnum()].challenge+QT_CMDQUERY, infostring));
		else
			CL_AddReliableCommand(&svse.authserver, va("getUserinfo Ticket %i \"%s\"", -1, infostring));

		Com_DPrintf("Query sent:\n");

	}else{
		Com_Printf("N/A\n");
	}*/
}

/*
================
SV_Status_f
================
*/
static void SV_Status_f( void ) {
	int			i, j, l;
	client_t	*cl;
	gclient_t	*gclient;
	const char		*s;
	int			ping;

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Com_Printf ("map: %s\n", sv_mapname->string );

	Com_Printf ("num score ping guid                             name            lastmsg address                                              qport rate\n");
	Com_Printf ("--- ----- ---- -------------------------------- --------------- ------- ---------------------------------------------------- ----- -----\n");

	for (i=0,cl=svs.clients, gclient = level.clients; i < sv_maxclients->integer ; i++, cl++, gclient++)
	{
		if (!cl->state)
			continue;
		Com_Printf ("%3i ", i);
		Com_Printf ("%5i ", gclient->pers.scoreboard.score);
		if (cl->state == CS_CONNECTED)
			Com_Printf ("CNCT ");
		else if (cl->state == CS_ZOMBIE)
			Com_Printf ("ZMBI ");
		else if (cl->state == CS_PRIMED)
			Com_Printf ("PRIM ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf ("%4i ", ping);
		}

		Com_Printf ("%s", cl->pbguid );

		l = 33 - strlen(cl->pbguid);
		j = 0;

		do
		{
			Com_Printf (" ");
			j++;
		} while(j < l);

		Com_Printf ("%s", cl->shortname);

		// TTimo adding a ^7 to reset the color
		// NOTE: colored names in status breaks the padding (WONTFIX)
		Com_Printf ("^7");
		l = 16 - Q_PrintStrlen(cl->shortname);
		j = 0;

		do
		{
			Com_Printf (" ");
			j++;
		} while(j < l);

		Com_Printf ("%7i ", svs.time - cl->lastPacketTime );
		/* Length must be: [0000:1111:2222:3333:4444:5555:6666:7777:8888]:65535 = 52 */
		s = NET_AdrToString( &cl->netchan.remoteAddress );
		Com_Printf ("%s", s);
		l = 52 - strlen(s);
		j = 0;

		do
		{
			Com_Printf(" ");
			j++;
		} while(j < l);

		Com_Printf (" %5i", cl->netchan.qport);

		Com_Printf (" %5i", cl->rate);

		Com_Printf ("\n");
	}
	Com_Printf ("\n");
}

/*
============
Cmd_UnbanPlayer_f
============
*/

static void Cmd_UnbanPlayer_f() {

    char* guid;
    int uid;
    qboolean unbanstatus;

    if ( Cmd_Argc() < 2) {
        if(psvs.useuids)
            Com_Printf( "Usage: unban <@uid>\n" );
        else
            Com_Printf( "Usage: unban <guid>\n" );
        return;
    }

    guid = SV_IsGUID(Cmd_Argv(1));
    if(guid){
        uid = 0;
        unbanstatus = SV_RemoveBan(0, guid, NULL);
    }else{
        uid = SV_Cmd_GetPlayerByHandle().uid;
        if(uid < 1){
            Com_Printf("Error: This player can not be unbanned, no such player\n");
            return;
        }
        unbanstatus = SV_RemoveBan(uid, NULL, NULL);
    }

    //InsertPluginEvent


    if(!unbanstatus){
        Com_Printf("Error: Tried to unban a player who is not actually banned\n");
        return;
    }else{

        if(uid)
            SV_PrintAdministrativeLog( "Removed ban for player with uid: %i", uid);
        else
            SV_PrintAdministrativeLog( "Removed ban for player with guid: %s", guid);
    }

}



/*
============
Cmd_BanPlayer_f
============
*/

static void Cmd_BanPlayer_f() {

    int i;
    char* guid = NULL;
    clanduid_t cl = { 0 };
    char banreason[256];
    char dropmsg[MAX_STRING_CHARS];

    if(!Q_stricmp(Cmd_Argv(0), "banUser") || !Q_stricmp(Cmd_Argv(0), "banClient"))
	{
        if(Cmd_Argc() < 2){
            Com_Printf( "Usage: banUser <user>\n" );
			Com_Printf( "Where user is one of the following: online-playername | online-playerslot | guid | uid\n" );
			Com_Printf( "online-playername can be a fraction of the playername. uid is a number > 0 and gets written with a leading \"@\" character\n" );
			Com_Printf( "guid is an hex decimal string with length of 8 characters\n" );
            return;
        }
	
	}else{
	    if ( Cmd_Argc() < 3) {
            Com_Printf( "Usage: permban <user> <Reason for this ban (max 126 chars)>\n" );
			Com_Printf( "Where user is one of the following: online-playername | online-playerslot | guid | uid\n" );
			Com_Printf( "online-playername can be a fraction of the playername. uid is a number > 0 and gets written with a leading \"@\" character\n" );
			Com_Printf( "guid is an hex decimal string with length of 8 characters\n" );
            return;
        }
    }

    guid = SV_IsGUID(Cmd_Argv(1));
	if(guid){
		goto gothandle;
	}
	
	cl = SV_Cmd_GetPlayerByHandle();	
	if(cl.cl){
		goto gothandle;
	}
	
	if( cl.uid ){
		goto gothandle;
	}
	
	Com_Printf("Error: This player can not be banned, no such player\n");
	return;
		
gothandle:
		
	if(cl.cl && strlen(cl.cl->pbguid) == 32)
	{
		guid = &cl.cl->pbguid[24];
	}
	else if(cl.cl && cl.uid < 1)
	{
		Com_Printf("Error: This player has no valid ID and got banned by IP only\n");
		SV_DropClient(cl.cl, "Invalid ID\n");
		SV_PlayerAddBanByip(&cl.cl->netchan.remoteAddress, "INVALID USER", 0, "INVALID", 0, -1);
		return;
	}
	
	
    banreason[0] = 0;
    if ( Cmd_Argc() > 2) {
        for(i = 2; Cmd_Argc() > i ;i++){
            Q_strcat(banreason,256,Cmd_Argv(i));
            Q_strcat(banreason,256," ");
        }
    }else{
        Q_strncpyz(banreason, "The admin has given no reason", 256);
    }

    if(strlen(banreason) > 126){
        Com_Printf("Error: You have exceeded the maximum allowed length of 126 characters for the reason\n");
        return;
    }

	if(cl.cl){
		
		if(cl.cl->power > Cmd_GetInvokerPower() && Cmd_GetInvokerPower() > 1){
			Com_Printf("Error: You cannot ban an admin with higher power!\n");
			return;
		}
		
		//Banning
		SV_AddBan(cl.uid, Cmd_GetInvokerUID(), guid, cl.cl->name, (time_t)-1, banreason);
		//Messages and kick
		if(cl.uid > 0){
			Com_Printf( "Banrecord added for player: %s uid: %i\n", cl.cl->name, cl.uid);
			SV_PrintAdministrativeLog( "banned player: %s uid: %i IP: %s with the following reason: %s", cl.cl->name, cl.uid, NET_AdrToString ( &cl.cl->netchan.remoteAddress ), banreason);
			Com_sprintf(dropmsg, sizeof(dropmsg), "You have been permanently banned on this server\nYour ban will %s expire\nYour UID is: %i    Banning admin UID is: %i\nReason for this ban:\n%s",
				"never", cl.uid, Cmd_GetInvokerUID(), banreason);
		}else{
			Com_Printf( "Banrecord added for player: %s guid: %s\n", cl.cl->name, cl.cl->pbguid);
			SV_PrintAdministrativeLog( "banned player: %s guid: %s IP: %s with the following reason: %s", cl.cl->name, cl.cl->pbguid, NET_AdrToString ( &cl.cl->netchan.remoteAddress ), banreason);
			Com_sprintf(dropmsg, sizeof(dropmsg), "You have been permanently banned on this server\nYour GUID is: %s    Banning admin UID is: %i\nReason for this ban:\n%s",
				cl.cl->pbguid, Cmd_GetInvokerUID(), banreason);			
			
			if(cl.cl->authentication < 1)
				SV_PlayerAddBanByip(&cl.cl->netchan.remoteAddress, banreason, 0, cl.cl->pbguid, 0, -1);
		}
		SV_DropClient(cl.cl, dropmsg);


	}else{
		//Banning
		SV_AddBan(cl.uid, Cmd_GetInvokerUID(), guid, "N/A", (time_t)-1, banreason);
		//Messages
		if(cl.uid > 0){
			Com_Printf( "Banrecord added for uid: %i\n", cl.uid);
			SV_PrintAdministrativeLog( "banned player uid: %i with the following reason: %s", cl.uid, banreason);
		}else{
			Com_Printf( "Banrecord added for guid: %s\n", guid);
			SV_PrintAdministrativeLog( "banned player guid: %s with the following reason: %s", guid, banreason);
		}
	}
	//InsertPluginEvent
}


/*
============
Cmd_TempBanPlayer_f
============
*/

static void Cmd_TempBanPlayer_f() {

    int i;
    clanduid_t cl = { 0 };
    char banreason[256];
    int duration = 0;
    char endtime[32];
    char* temp;
    time_t aclock;
    time(&aclock);
    int length;
    char buff[8];
    char *guid = NULL;
    char dropmsg[MAX_STRING_CHARS];

    if ( Cmd_Argc() < 4) {
		Com_Printf( "Usage: tempban <user> <time> <Reason for this ban (max. 126 chars)>\n" );
		Com_Printf( "Where user is one of the following: online-playername | online-playerslot | guid | @uid\n" );
		Com_Printf( "Where time is one of the following: ^2XX^7m | ^2XX^7h | ^2XX^7d\n" );
		Com_Printf( "Where reason for this ban is contains a description without the letters: \" ; %% / \\ \n" );		
		Com_Printf( "online-playername can be a fraction of the playername. uid is a number > 0 and gets written with a leading \"@\" character\n" );
		Com_Printf( "guid is a hexadecimal string with length of 8 characters\n" );
		Com_Printf( "XX is an integer representing the time of ban in minutes, hours or days\n" );		
		return;
    }
	/* Get the handle for this player */
    guid = SV_IsGUID(Cmd_Argv(1));
	if(guid){
		goto gothandle;
	}
	
	cl = SV_Cmd_GetPlayerByHandle();	
	if(cl.cl){
		goto gothandle;
	}
	
	if( cl.uid ){
		goto gothandle;
	}
	
	Com_Printf("Error: This player can not be banned, no such player\n");
	return;
	
gothandle:
	
	/* Get the time this ban should last */
    length = strlen(Cmd_Argv(2));
    if(length > 7){
        Com_Printf("Error: Did not got a valid ban time\n");
        return;
    }

    if(Cmd_Argv(2)[length-1] == 'm'){
        if(isNumeric(Cmd_Argv(2),length-1)){
            Q_strncpyz(buff,Cmd_Argv(2),length);
            duration = atoi(buff);
        }

    }else if(Cmd_Argv(2)[length-1] == 'h'){
        if(isNumeric(Cmd_Argv(2),length-1)){
            Q_strncpyz(buff,Cmd_Argv(2),length);
            duration = (atoi(buff) * 60);
        }
    }else if(Cmd_Argv(2)[length-1] == 'd'){
        if(isNumeric(Cmd_Argv(2),length-1)){
            Q_strncpyz(buff,Cmd_Argv(2),length);
            duration = (atoi(buff) * 24 * 60);
        }
    }
    if(duration < 1){
        Com_Printf("Error: Did not got a valid ban time\n");
        return;
    }
    if(duration > 60*24*30){
        Com_Printf("Error: Can not issue a temporary ban that lasts longer than 30 days\n");
        return;
    }

	time_t expire = (aclock+(time_t)(duration*60));
	temp = ctime(&expire);
	temp[strlen(temp)-1] = 0;
	Q_strncpyz(endtime, temp, sizeof(endtime));
	
	/* Verify the players ID and drop/ipban if needed */
	
	if(cl.cl && strlen(cl.cl->pbguid) == 32)
	{
		guid = &cl.cl->pbguid[24];
	}
	else if(cl.cl && cl.uid < 1)
	{
		Com_Printf("Error: This player has no valid ID and got banned by IP only\n");
		SV_DropClient(cl.cl, "Invalid ID\n");
		SV_PlayerAddBanByip(&cl.cl->netchan.remoteAddress, "INVALID USER", 0, "INVALID", 0, expire);
		return;
	}
	
	/* Get a valid banreason */
    banreason[0] = 0;
    for(i = 3; Cmd_Argc() > i ;i++){
        Q_strcat(banreason,256,Cmd_Argv(i));
        Q_strcat(banreason,256," ");
    }
    if(strlen(banreason) > 126){
        Com_Printf("Error: You have exceeded the maximum allowed length of 126 for the reason\n");
        return;
    }
	

	if(cl.cl){
		
		if(cl.cl->power > Cmd_GetInvokerPower() && Cmd_GetInvokerPower() > 1){
			Com_Printf("Error: You cannot tempban an admin with higher power!\n");
			return;
		}
		
		SV_AddBan(cl.uid, Cmd_GetInvokerUID(), guid, cl.cl->name, expire, banreason);

		if( cl.uid > 0 ){

			Com_Printf( "Banrecord added for player: %s uid: %i\n", cl.cl->name, cl.uid);
			SV_PrintAdministrativeLog( "temporarily banned player: %s uid: %i IP: %s until %s with the following reason: %s", cl.cl->name, cl.uid, NET_AdrToString ( &cl.cl->netchan.remoteAddress ), endtime, banreason);
			Com_sprintf(dropmsg, sizeof(dropmsg), "You have been temporarily banned from this server\nYour ban will expire on: %s UTC\nYour UID is: %i    Banning admin UID is: %i\nReason for this ban:\n%s",
				endtime, cl.uid, Cmd_GetInvokerUID(), banreason);
			SV_DropClient(cl.cl, dropmsg);

		}else{
			Com_Printf( "Banrecord added for player: %s guid: %s\n", cl.cl->name, cl.cl->pbguid);
			SV_PrintAdministrativeLog( "temporarily banned player: %s guid: %s IP: %s until %s with the following reason: %s", cl.cl->name, cl.cl->pbguid,NET_AdrToString ( &cl.cl->netchan.remoteAddress ), endtime, banreason);
			Com_sprintf(dropmsg, sizeof(dropmsg), "You have been temporarily banned from this server\nYour ban will expire on: %s UTC\nYour GUID is: %s    Banning admin UID is: %i\nReason for this ban:\n%s",
				endtime, cl.cl->pbguid, Cmd_GetInvokerUID(), banreason);

			if(cl.cl->authentication < 1)
				SV_PlayerAddBanByip(&cl.cl->netchan.remoteAddress, banreason, 0, cl.cl->pbguid, 0, expire);

		}
		SV_DropClient(cl.cl, dropmsg);
	}else{

		SV_AddBan(cl.uid, Cmd_GetInvokerUID(), guid, "N/A", expire, banreason);

		if(cl.uid > 0){
			Com_Printf( "Banrecord added for uid: %i\n", cl.uid);
			SV_PrintAdministrativeLog( "temporarily banned player uid: %i until %s with the following reason: %s", cl.uid, endtime, banreason);

		}else{
			Com_Printf( "Banrecord added for guid: %s\n", guid);
			SV_PrintAdministrativeLog( "temporarily banned player guid: %s until %s with the following reason: %s", guid, endtime, banreason);
		}
	}
	//InsertPluginEvent
}






/*
============
Cmd_KickPlayer_f
============
*/

static void Cmd_KickPlayer_f() {

    int i;
    clanduid_t cl;
    char kickreason[256];
    char dropmsg[MAX_STRING_CHARS];

    if ( Cmd_Argc() < 2) {
		
		Com_Printf( "Usage: kick <user> <Reason for this ban (max. 126 chars)>\n" );
		Com_Printf( "Where user is one of the following: online-playername | online-playerslot\n" );
		Com_Printf( "Where reason for this ban is contains a description without the letters: \" ; %% / \\ \n" );		
		Com_Printf( "online-playername can be a fraction of the playername.\n" );		
		return;
    }

    cl = SV_Cmd_GetPlayerByHandle();
	
    if(!cl.cl)
	{
            Com_Printf("Error: This player is not online and can not be kicked\n");
            return;
    }

    kickreason[0] = 0;
    if ( Cmd_Argc() > 2) {
        for(i = 2; Cmd_Argc() > i ;i++){
            Q_strcat(kickreason,256,Cmd_Argv(i));
            Q_strcat(kickreason,256," ");
        }
    }else{
        Q_strncpyz(kickreason, "The admin has no reason given", 256);
    }
    if(strlen(kickreason) >= 256 ){
        Com_Printf("Error: You have exceeded the maximum allowed length of 126 for the reason\n");
        return;
    }
	
	if(cl.cl->power > Cmd_GetInvokerPower() && Cmd_GetInvokerPower() > 1){
		Com_Printf("Error: You cannot kick an admin with higher power!\n");
		return;
	}
	
	if(cl.uid > 0){
		Com_Printf( "Player kicked: %s ^7uid: %i\nReason: %s\n", cl.cl->name, cl.uid, kickreason);
		SV_PrintAdministrativeLog( "kicked player: %s^7 uid: %i with the following reason: %s", cl.cl->name, cl.uid, kickreason);
	}else{
        Com_Printf( "Player kicked: %s ^7guid: %s\nReason: %s\n", cl.cl->name, cl.cl->pbguid, kickreason);
        SV_PrintAdministrativeLog( "kicked player: %s^7 guid: %s with the following reason: %s", cl.cl->name, cl.cl->pbguid, kickreason);
	}
	
	if(Cmd_GetInvokerUID() > 0){
		Com_sprintf(dropmsg, sizeof(dropmsg), "Player kicked:\nAdmin UID is: %i\nReason for this kick:\n%s", Cmd_GetInvokerUID(), kickreason);
	}else{
		Com_sprintf(dropmsg, sizeof(dropmsg), "Player kicked:\nReason for this kick:\n%s", kickreason);
	}
	SV_DropClient(cl.cl, dropmsg);
}

/*
================
SV_DumpBanlist_f
================
*/

static void SV_DumpBanlist_f(){
    SV_DumpBanlist();
}



/*
================
SV_MiniStatus_f
================
*/
static void SV_MiniStatus_f( void ) {
	int			i, j, l;
	client_t	*cl;
	gclient_t	*gclient;
	const char	*s;
	int		ping;
	qboolean	odd = qfalse;

	// make sure server is running
	if ( !com_sv_running->boolean ) {
		Com_Printf( "Server is not running.\n" );
		return;
	}

	Com_Printf ("map: %s\n", sv_mapname->string );

	Com_Printf ("num score ping uid/guid   name                             address                                             FPS XVer\n");

	Com_Printf ("--- ----- ---- ---------- -------------------------------- --------------------------------------------------- --- ----\n");
	for (i=0,cl=svs.clients, gclient = level.clients ; i < sv_maxclients->integer ; i++, cl++, gclient++)
	{
		if (!cl->state)
			continue;

		if(odd)
			Com_Printf ("^8");
		else
			Com_Printf ("^7");

		Com_Printf ("%3i ", i);
		Com_Printf ("%5i ", gclient->pers.scoreboard.score);
		if (cl->state == CS_CONNECTED)
			Com_Printf ("CNCT ");
		else if (cl->state == CS_ZOMBIE)
			Com_Printf ("ZMBI ");
		else if (cl->state == CS_PRIMED)
			Com_Printf ("PRIM ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf ("%4i ", ping);
		}

		
		if(cl->uid > 0){
			Com_Printf ("@%9i ", cl->uid );

		}else{

			s = cl->pbguid + 24;
			Com_Printf ("%s", s);
			l = 11 - strlen(s);

			j = 0;

			do
			{
				Com_Printf(" ");
				j++;
			} while(j < l);
		}

		Com_Printf ("%s", cl->name);

		// TTimo adding a ^7 to reset the color
		// NOTE: colored names in status breaks the padding (WONTFIX)

		if(odd)
			Com_Printf ("^8");
		else
			Com_Printf ("^7");

		l = 33 - Q_PrintStrlen(cl->name);
		j = 0;

		do
		{
			Com_Printf (" ");
			j++;
		} while(j < l);

		s = NET_AdrToConnectionStringMask( &cl->netchan.remoteAddress );
		Com_Printf ("%s", s);
		l = 52 - strlen(s);
		j = 0;

		do
		{
			Com_Printf(" ");
			j++;
		} while(j < l);

		Com_Printf("%3i ", cl->clFPS);

		Com_Printf("%s ", cl->xversion);

		odd = ~odd;
		Com_Printf ("\n");
	}
}





/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f( void ) {
	svse.nextHeartbeatTime = 0;
}


/*
============
Cmd_ExecuteTranslatedCommand_f
============
*/

static void Cmd_ExecuteTranslatedCommand_f(){

    int i;
    char outstr[128];

    char *cmdname = Cmd_Argv(0);
    char *cmdstring = NULL;
    char *tmp;

    for(i=0; i < MAX_TRANSCMDS; i++){
        if(!Q_stricmp(cmdname, psvs.translatedCmd[i].cmdname)){
            cmdname = psvs.translatedCmd[i].cmdname;
            cmdstring = psvs.translatedCmd[i].cmdargument;
            break;
        }
    }
    if(!cmdstring) return;

    tmp = outstr;
    i = 1;
    while(*cmdstring){
        if(*cmdstring == '$'){
            if(!Q_strncmp(cmdstring, "$uid", 4)){
                Com_sprintf(tmp, sizeof(outstr) - (tmp - outstr), "%i", Cmd_GetInvokerUID());
                tmp += strlen(tmp);
                cmdstring += 4;
            }else if(!Q_strncmp(cmdstring, "$clnum", 6)){
                Com_sprintf(tmp, sizeof(outstr) - (tmp - outstr), "%i", Cmd_GetInvokerClnum());
                tmp += strlen(tmp);
                cmdstring += 6;
            }else if(!Q_strncmp(cmdstring, "$pow", 4)){
                Com_sprintf(tmp, sizeof(outstr) - (tmp - outstr), "%i", Cmd_GetInvokerPower());
                tmp += strlen(tmp);
                cmdstring += 4;
            }else if(!Q_strncmp(cmdstring, "$arg", 4)){
                if(!*Cmd_Argv(i)){
                    cmdstring += 4;
                    if(*cmdstring == ':' && *(cmdstring + 1) != ' '){ // Default argument in place!
                        ++cmdstring; // Just advance the pointer and read in the argument as any other part of the string
                    }else{
                        Com_Printf("Not enough arguments to this command\n");
                        return;
                    }
                } else{
                    cmdstring += 4;
                    if(*cmdstring == ':') // Skip default arg (if any)
                        while(*cmdstring != ' ' && *cmdstring != ';' && *cmdstring) ++cmdstring;
                	
                    if(strchr(Cmd_Argv(i), ';') || strchr(Cmd_Argv(i), '\n')){
                        return;
                    }
                    Com_sprintf(tmp, sizeof(outstr) - (tmp - outstr), "%s", Cmd_Argv(i));
                    tmp += strlen(tmp);
                   i++;
                }
            }
        }

        *tmp = *cmdstring;
        cmdstring++;
        tmp++;

    }

    *tmp = 0;
    Com_DPrintf("String to Execute: %s\n", outstr);
    Cbuf_AddText( outstr);
}



/*
============
Cmd_AddTranslatedCommand_f
============
*/

static void Cmd_AddTranslatedCommand_f() {

    char *cmdname;
    char *string;
    int free;
    int i;

    if ( Cmd_Argc() != 3) {
        Com_Printf( "Usage: addCommand <commandname> <\"string to execute\"> String can contain: $uid $clnum $pow $arg $arg:default\n" );
        return;
    }

    cmdname = Cmd_Argv(1);
    string = Cmd_Argv(2);

    for(i=0, free = -1; i < MAX_TRANSCMDS; i++){
        if(!Q_stricmp(cmdname, psvs.translatedCmd[i].cmdname)){
            Com_Printf("This command is already defined\n");
            return;
        }
        if(!*psvs.translatedCmd[i].cmdname){
            free = i;
        }

    }
    if(free == -1){
        Com_Printf("Exceeded limit of custom commands\n");
        return;
    }

    Q_strncpyz(psvs.translatedCmd[free].cmdname, cmdname, sizeof(psvs.translatedCmd[free].cmdname));
    Q_strncpyz(psvs.translatedCmd[free].cmdargument, string, sizeof(psvs.translatedCmd[free].cmdargument));

    Cmd_AddPCommand (psvs.translatedCmd[free].cmdname, Cmd_ExecuteTranslatedCommand_f, 100);
    Com_Printf("Added custom command: %s -> %s\n", psvs.translatedCmd[free].cmdname, psvs.translatedCmd[free].cmdargument);

}

/*
====================
SV_StopRecording_f

stop recording a demo
====================
*/
static void SV_StopRecord_f( void ) {

	clanduid_t cl;
	int i;

	if ( Cmd_Argc() != 2) {
		Com_Printf( "stoprecord <client>\n" );
		return;
	}

	if(!Q_stricmp(Cmd_Argv( 1 ), "all"))
	{
		for(i = 0, cl.cl = svs.clients; i < sv_maxclients->integer; i++, cl.cl++)
		{
			if(cl.cl->demorecording)
				SV_StopRecord(cl.cl);
		}
		return;
	}


	cl = SV_Cmd_GetPlayerByHandle();
	if(!cl.cl){
		Com_Printf("Error: This player is not online and can not be recorded\n");
		return;
	}
	SV_StopRecord(cl.cl);
}


/*
====================
SV_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
//static char demoName[MAX_QPATH];        // compiler bug workaround
static void SV_Record_f( void ) {

	char* s;
	char name[MAX_QPATH];
	clanduid_t cl;
	int i;

	if ( Cmd_Argc() > 3 || Cmd_Argc() < 2) {
		Com_Printf( "record <client> <demoname>\n" );
		return;
	}

	if ( Cmd_Argc() == 3 ) {
		s = Cmd_Argv( 2 );
	} else {
		s = NULL;
	}

	if(!Q_stricmp(Cmd_Argv( 1 ), "all"))
	{
		for(i = 0, cl.cl = svs.clients; i < sv_maxclients->integer; i++, cl.cl++)
		{
			if(cl.cl->state == CS_ACTIVE && !cl.cl->demorecording){

				if(cl.cl->uid > 0)
				{
					Com_sprintf(name, sizeof(name), "demo_%i_", cl.cl->uid);

				}else{
				
					Com_sprintf(name, sizeof(name), "demo_%s_", cl.cl->pbguid);
				}
				SV_RecordClient(cl.cl, name);
			}
		}
		return;
	}

	cl = SV_Cmd_GetPlayerByHandle();
	if(!cl.cl){
		Com_Printf("Error: This player is not online and can not be recorded\n");
		return;
	}

	if(s){
		
		SV_RecordClient(cl.cl, s);
	}else{
		
		if(cl.cl->uid > 0)
			Com_sprintf(name, sizeof(name), "demo_%i_", cl.cl->uid);
		else
			Com_sprintf(name, sizeof(name), "demo_%s_", cl.cl->pbguid);
	}
	SV_RecordClient(cl.cl, name);
}



static void SV_ShowRules_f(){

    unsigned int clnum;
    client_t* cl;
    int i;

    if(Cmd_GetInvokerPower() > 20){

	for(i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
	{
		if(cl->state == CS_ACTIVE){
	            cl->msgType = 1;
	            cl->currentAd = 0;
		}
	}

    }else{
        clnum = Cmd_GetInvokerClnum();
        if(clnum < 64){
            cl = &svs.clients[clnum];
            cl->msgType = 1;
            cl->currentAd = 0;
        }
    }
}

static void SV_AddRule_f(){

    if ( Cmd_Argc() != 2) {
        Com_Printf( "Usage: addRuleMsg <\"text here in quotes\">\n" );
        return;
    }

    G_AddRule( Cmd_Argv(1));
}

static void SV_AddAdvert_f(){

    if ( Cmd_Argc() != 2) {
        Com_Printf( "Usage: addAdvertMsg <\"text here in quotes\">\n" );
        return;
    }
    G_AddAdvert( Cmd_Argv(1));
}

static void SV_ClearAllMessages_f(){

    G_ClearAllMessages();

}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f( void ) {
	Com_Printf( "Server info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_SERVERINFO ) );
}


/*
===========
SV_Systeminfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Systeminfo_f( void ) {
	Com_Printf( "System info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_SYSTEMINFO ) );
}

//===========================================================


/*
=================
SV_KillServer
=================
*/
static void SV_KillServer_f( void ) {
	SV_Shutdown("killserver");
	Com_Restart(  );
}


static void SV_Map_f( void ) {

	const char* map;
	const char* cmd;

	map = Cmd_Argv( 1 );
	if ( !map ) {
		return;
	}

	SV_Map(map);

	// set the cheat value
	// if the level was started with "map <levelname>", then
	// cheats will not be allowed.  If started with "devmap <levelname>"
	// then cheats will be allowed
	cmd = Cmd_Argv( 0 );
	if ( !Q_stricmp( cmd, "devmap" ) ) {

		Cvar_SetBool( sv_cheats, qtrue );
	} else {

		Cvar_SetBool( sv_cheats, qfalse );
	}
}


/*
================
SV_MapRotate_f

================
*/
static void SV_MapRotate_f( void ) {

	char map[MAX_QPATH];
	char gametype[MAX_QPATH];
	char map_rotationbuf[8192];
	char* maplist;
	int len;

	// DHM - Nerve :: Check for invalid gametype
	Com_Printf("map_rotate...\n");
	Com_Printf("\"sv_mapRotation\" is: \"%s\"\n\n", sv_mapRotation->string);
	Com_Printf("\"sv_mapRotationCurrent\" is: \"%s\"\n\n", sv_mapRotationCurrent->string);

	if(sv_mapRotationCurrent->string[0] == '\0')
	{
		Cvar_SetString(sv_mapRotationCurrent, sv_mapRotation->string);
	}

	Q_strncpyz(map_rotationbuf, sv_mapRotationCurrent->string, sizeof(map_rotationbuf));
	Com_ParseReset();
	maplist = Com_ParseGetToken(map_rotationbuf);

	if(maplist == NULL){
		if(com_sv_running->boolean){
			Com_PrintError("\"sv_mapRotation\" is empty. Restarting current map\n");
			SV_MapRestart( qfalse );
		}else{
			Com_PrintError("\"sv_mapRotation\" is empty. Can not start the server\n");
		}
		return;
	}

	while(qtrue)
	{
		if(maplist == NULL)
			break;

		if(!Q_stricmpn(maplist, "gametype ", 9)){

			maplist = Com_ParseGetToken(maplist);
			if(maplist == NULL)
				break;

			len = Com_ParseTokenLength(maplist);
			if(len >= sizeof(gametype)){
				len = sizeof(gametype) -1;
				Com_PrintWarning("Oversize gametype name length at: %s\n", maplist);
			}
			Q_strncpyz(gametype, maplist, len+1);
			Cvar_SetString(sv_g_gametype, gametype);
			maplist = Com_ParseGetToken(maplist); //Pop off the gametype argument

		}else if(!Q_stricmpn(maplist, "map ", 4)){

			maplist = Com_ParseGetToken(maplist);
			if(maplist == NULL)
				break;

			len = Com_ParseTokenLength(maplist);
			if(len >= sizeof(map)){
				len = sizeof(map) -1;
				Com_PrintWarning("Oversize map name length at: %s\n", maplist);
			}
			Q_strncpyz(map, maplist, len+1);

			maplist = Com_ParseGetToken(maplist); //Pop off the last map-name

			if(maplist == NULL)
				maplist = "";

			Cvar_SetString(sv_mapRotationCurrent, maplist); //Set the cvar with one map less

			if(!SV_Map(map)){ //Load the level
				Com_PrintError("Invalid mapname at %s %s\nRestarting current map\n", map, maplist);
				SV_MapRestart( qfalse );
			}
			return;
		}else{
			Com_PrintError("Broken maprotation at: %s\n", maplist);
			maplist = Com_ParseGetToken(maplist); //Pop off the last argument
		}
	}


	if(com_sv_running->boolean){
		Com_PrintError("\"sv_mapRotation\" is broken at: %s.\nRestarting current map\n");
		SV_MapRestart( qfalse );
	}else{
		Com_PrintError("\"sv_mapRotation\" is empty. Can not start the server\n");
	}
}

static void SV_MapRestart_f(void){

	SV_MapRestart(qfalse);

}

static void SV_FastRestart_f(void){

	SV_MapRestart(qtrue);

}


static void SV_SetPerk_f( void ){

    clanduid_t cl;
    unsigned int perkIndex;
    unsigned int clnum;

    cl = SV_Cmd_GetPlayerByHandle();
    if(!cl.cl)
        return;

    if(Cmd_Argc() < 3){
        Com_DPrintf("Unknown Perk\n");
        return;
    }
    perkIndex = BG_GetPerkIndexForName(Cmd_Argv(2));
    if(perkIndex > 19){
        Com_DPrintf("Unknown Perk: %s\n", Cmd_Argv(2));
        return;
    }

    clnum = cl.cl - svs.clients;

    playerState_t *ps = SV_GameClientNum(clnum);

    ps->perks |= (1 << perkIndex);

    level.clients[clnum].sess.perkIndex |= (1 << perkIndex);

}



static void SV_TestTimeOverrun( void ){

	svs.time = 0x6ffeffff;

}



static void SV_GetCurrentServeTimer(){

	Com_Printf("Server Time is : %x\n", svs.time);
}


static void SV_SetGravity_f( void ){

    clanduid_t cl;
    int gravity;
    unsigned int clnum;

    cl = SV_Cmd_GetPlayerByHandle();
    if(!cl.cl)
        return;

    if(Cmd_Argc() < 3){
        Com_Printf("Bad args\n");
        return;
    }

    gravity = atoi(Cmd_Argv(2));

    clnum = cl.cl - svs.clients;

    playerState_t *ps = SV_GameClientNum(clnum);

    ps->gravity = gravity;

    Com_Printf("Gravity: %i\n", ps->gravity);

}
/*
static void SV_SetStance_f( void ){

    clanduid_t cl;
    int gravity;
    unsigned int clnum;

    cl = SV_Cmd_GetPlayerByHandle();
    if(!cl.cl)
        return;

    if(Cmd_Argc() < 3){
        Com_Printf("Bad args\n");
        return;
    }

    gravity = atoi(Cmd_Argv(2));

    clnum = cl.cl - svs.clients;

    playerState_t *ps = SV_GameClientNum(clnum);

    ps->stance = gravity;

    Com_Printf("Gravity: %i, Stance: %i\n", ps->gravity, ps->stance);

}
*/



static void SV_Find_f( void ){

    int i;

    for(i = 0; i < sv_maxclients->integer; i++){

        playerState_t *ps = SV_GameClientNum(i);

        if(svs.clients[i].state < CS_ACTIVE)
            continue;

        Com_Printf("Clientnum: %i \n", ps->clientNum);
    }
}


static void SV_ShowConfigstring_f()
{
    char buffer[8192];
    int index;

    buffer[0] = 0;

    if ( Cmd_Argc() != 2 ) {
	Com_Printf( "Usage: showconfigstring <index>\n" );
	return;
    }

    index = atoi(Cmd_Argv(1));
    SV_GetConfigstring(index, buffer, sizeof(buffer));
    Com_Printf("CS len is %d CS is: %s\n", strlen(buffer), buffer);
}




qboolean SV_RunDownload(const char* url, const char* filename)
{
	ftRequest_t* curfileobj;
	int len, transret;

	curfileobj = FileDownloadRequest(url);
	if(curfileobj == NULL)
	{
		Com_Printf("Couldn't connect to download-server for downloading. Failed to download %s\n", filename);
		return qfalse;	
	}
		
	do
	{
		transret = FileDownloadSendReceive( curfileobj );
		usleep(20000);
		
	} while (transret == 0);
		
	if(transret < 0)
	{
		Com_Printf("Downloading of file: \"%s\" has failed\n", filename );
		FileDownloadFreeRequest(curfileobj);
		return qfalse;
	}
	
	if(curfileobj->code != 200)
	{
		Com_Printf("Downloading of file: \"%s\" has failed with the following message: %d %s\n", filename, curfileobj->code, curfileobj->status);
		FileDownloadFreeRequest(curfileobj);
		return qfalse;
		
	}
	
	len = FS_SV_BaseWriteFile(filename, curfileobj->recvmsg.data + curfileobj->headerLength, curfileobj->contentLength);
	if(len != curfileobj->contentLength){
		
		len = FS_SV_HomeWriteFile(filename, curfileobj->recvmsg.data + curfileobj->headerLength, curfileobj->contentLength);
		if(len != curfileobj->contentLength)
		{
			Com_PrintError("Opening of \"%s\" for writing has failed! Download aborted.\n", filename);
			FileDownloadFreeRequest(curfileobj);
			return qfalse;
		}
	}
	FileDownloadFreeRequest(curfileobj);
	return qtrue;
}

void SV_DemoCompletedExec(const char* mapname)
{
	char cmdline[MAX_STRING_CHARS];

	if(!*sv_mapDownloadCompletedCmd->string)
		return;

	Com_sprintf(cmdline, sizeof(cmdline), "\"%s/%s\" \"%s/usermaps/%s\"", fs_homepath->string, sv_mapDownloadCompletedCmd->string, fs_homepath->string, mapname);

	Sys_DoStartProcess(cmdline);

	Sys_TermProcess();
}

void SV_DownloadMapThread(char *inurl)
{
	int len;
	
	char url[MAX_STRING_CHARS];
	char dlurl[MAX_STRING_CHARS];
	char *mapname;
	char filename[MAX_OSPATH];
	static qboolean downloadActive = 0;

	
	Q_strncpyz(url, inurl, sizeof(url));
	Z_Free(inurl);
	
	
	Sys_EnterCriticalSection(CRIT_MISC);

	if(downloadActive)
	{
		Com_Printf("There is already a map download running. Won't download this.\n");
		Sys_LeaveCriticalSection(CRIT_MISC);		
		return;
	}
	downloadActive = qtrue;
	
	Sys_LeaveCriticalSection(CRIT_MISC);

	
	len = strlen(url);
	
	if(url[len -1] == '/')
		url[len -1] = '\0';
	
	mapname = strrchr(url, '/');

	if(mapname == NULL)
	{
		Com_Printf("Invalid map download path\n");
		downloadActive = qfalse;
		return;
	}
	
	mapname++;
		
	
	Com_sprintf(filename, sizeof(filename), "usermaps/%s/%s%s", mapname ,mapname, ".ff" );
	Com_sprintf(dlurl, sizeof(dlurl), "%s/%s%s", url, mapname, ".ff");
	
	Com_Printf("Begin downloading of file: \"%s\"\n", filename );
	
	if(SV_RunDownload(dlurl, filename) == qfalse)
	{
		Com_Printf("Aborted map download\n");
		downloadActive = qfalse;
		return;
	}
	Com_Printf("Received file: %s\n", filename );
	
	
	
	
	Com_sprintf(filename, sizeof(filename), "usermaps/%s/%s%s", mapname ,mapname, "_load.ff" );
	Com_sprintf(dlurl, sizeof(dlurl), "%s/%s%s", url, mapname, "_load.ff");
	
	Com_Printf("Begin downloading of file: \"%s\"\n", filename );
	
	if(SV_RunDownload(dlurl, filename) == qfalse)
	{
		Com_Printf("Aborted map download\n");
		downloadActive = qfalse;
		return;
	}
	Com_Printf("Received file: %s\n", filename );
	
	
	
	Com_sprintf(filename, sizeof(filename), "usermaps/%s/%s%s", mapname ,mapname, ".iwd" );
	Com_sprintf(dlurl, sizeof(dlurl), "%s/%s%s", url, mapname, ".iwd");
	
	Com_Printf("Begin downloading of file: \"%s\"\n", filename );
	
	if(SV_RunDownload(dlurl, filename) == qfalse)
	{
		Com_Printf("File %s was not downloaded. This map has maybe no .iwd file\n", filename);
	}else{
		Com_Printf("Received file: %s\n", filename );
        }
	Com_Printf("Download of map \"%s\" has been completed\n", mapname);
	downloadActive = qfalse;

	Sys_SetupThreadCallback(SV_DemoCompletedExec, mapname);

}

void SV_DownloadMap_f()
{
	char *url;
	int len;
	char buf[128];
	
	if ( Cmd_Argc() != 2 )
	{
		Com_Printf( "Usage: downloadmap <\"url\">\nFor example: downloadmap \"http://somehost/cod4/usermaps/mapname\"\n" );
		return;
    }
	
	len = strlen(Cmd_Argv(1));
	
	Cmd_Args(buf, sizeof(buf));
	
	if(len < 3 || len > MAX_STRING_CHARS)
	{
		Com_Printf( "Usage: downloadmap <\"url\">\n" );
		return;
	}
	
	url = Z_Malloc(len +1);
	if(url == NULL)
	{
		Com_PrintError("SV_DownloadMap_f(): Out of memory\n");
		return;
	}
	
	Q_strncpyz(url, Cmd_Argv(1), len +1);

	if(Sys_CreateCallbackThread(SV_DownloadMapThread, url) == qfalse)
	{
		Com_PrintError("SV_DownloadMap_f(): Failed to start download thread\n");
		Z_Free(url);
	}
}

void SV_ChangeGametype_f()
{

    char gametypestring[32];
    char* find;

    if(Cmd_Argc() != 2)
    {
        Com_Printf( "Usage: gametype <gametypename>\n" );
        return;
    }

    Q_strncpyz(gametypestring, Cmd_Argv(1), sizeof(gametypestring));

    if(strchr(gametypestring, ';'))
    {
        return;
    }

    find = strchr(gametypestring, '\n');

    if(find)
    {
        *find = '\0';
    }

    Cvar_Set("g_gametype", gametypestring);
    Cbuf_AddText("map_restart\n");

}


void SV_AddOperatorCommands(){
	
	static qboolean	initialized;
	
	if ( initialized ) {
		return;
	}
	initialized = qtrue;

	Cmd_AddCommand ("killserver", SV_KillServer_f);
	Cmd_AddCommand ("setPerk", SV_SetPerk_f);
	Cmd_AddPCommand ("map_restart", SV_MapRestart_f, 50);
	Cmd_AddCommand ("fast_restart", SV_FastRestart_f);
	Cmd_AddPCommand ("rules", SV_ShowRules_f, 1);
	Cmd_AddCommand ("heartbeat", SV_Heartbeat_f);
	Cmd_AddPCommand ("kick", Cmd_KickPlayer_f, 35);
	Cmd_AddCommand ("clientkick", Cmd_KickPlayer_f);
	Cmd_AddCommand ("onlykick", Cmd_KickPlayer_f);
	Cmd_AddPCommand ("unban", Cmd_UnbanPlayer_f, 80);
	Cmd_AddCommand ("unbanUser", Cmd_UnbanPlayer_f);
	Cmd_AddPCommand ("permban", Cmd_BanPlayer_f, 80);
	Cmd_AddPCommand ("tempban", Cmd_TempBanPlayer_f, 50);
	Cmd_AddCommand ("bpermban", Cmd_BanPlayer_f);
	Cmd_AddCommand ("btempban", Cmd_TempBanPlayer_f);
	Cmd_AddCommand ("banUser", Cmd_BanPlayer_f);
	Cmd_AddCommand ("banClient", Cmd_BanPlayer_f);
	Cmd_AddCommand ("ministatus", SV_MiniStatus_f);
	Cmd_AddPCommand ("say", SV_ConSayChat_f, 70);
	Cmd_AddCommand ("consay", SV_ConSayConsole_f);
	Cmd_AddPCommand ("screensay", SV_ConSayScreen_f, 70);
	Cmd_AddPCommand ("tell", SV_ConTellChat_f, 70);
	Cmd_AddCommand ("contell", SV_ConTellConsole_f);
	Cmd_AddPCommand ("screentell", SV_ConTellScreen_f, 70);
	Cmd_AddCommand ("dumpuser", SV_DumpUser_f);
	Cmd_AddCommand ("stringUsage", SV_StringUsage_f);
	Cmd_AddCommand ("scriptUsage", SV_ScriptUsage_f);
	
	Cmd_AddPCommand("stoprecord", SV_StopRecord_f, 70);
	Cmd_AddPCommand("record", SV_Record_f, 50);
	
	if(Com_IsDeveloper()){
		Cmd_AddCommand ("showconfigstring", SV_ShowConfigstring_f);
		Cmd_AddCommand ("devmap", SV_Map_f);
		
	}
	
}

void SV_AddSafeCommands(){

	static qboolean	initialized;

	if ( initialized ) {
		return;
	}
	initialized = qtrue;

	Cmd_AddPCommand ("systeminfo", SV_Systeminfo_f, 1);
	Cmd_AddPCommand ("serverinfo", SV_Serverinfo_f, 1);
	Cmd_AddPCommand ("map", SV_Map_f, 60);
	Cmd_AddCommand ("map_rotate", SV_MapRotate_f);
	Cmd_AddCommand ("addAdvertMsg", SV_AddAdvert_f);
	Cmd_AddCommand ("addRuleMsg", SV_AddRule_f);
	Cmd_AddCommand ("clearAllMsg", SV_ClearAllMessages_f);
	Cmd_AddPCommand ("dumpbanlist", SV_DumpBanlist_f, 30);
	Cmd_AddCommand ("writenvcfg", NV_WriteConfig);
	Cmd_AddCommand ("status", SV_Status_f);
	Cmd_AddCommand ("addCommand", Cmd_AddTranslatedCommand_f);
	Cmd_AddCommand ("downloadmap", SV_DownloadMap_f);
	Cmd_AddPCommand ("gametype", SV_ChangeGametype_f, 80);

}


void SV_Cmd_Init( void ) {

	*(int*)0x8879a40 = -1;
	*(int*)0x887eb40 = 0;
	*(int*)0x887eb44 = 0;

}


/*
============
SV_Cmd_Argc	Returns count of commandline arguments
============
*/
int	SV_Cmd_Argc( void ) {

	int	cmd_argc;

	__asm__ (
	"mov	0x8879a40,%%eax			\n\t"
	"mov	0x8879a84(,%%eax,4), %%eax	\n\t"
	:"=a" (cmd_argc));

	return cmd_argc;
}


/*
============
SV_Cmd_Argv	Returns commandline argument by number
============
*/

char	*SV_Cmd_Argv( int arg ) {

	char* cmd_argv;

	__asm__ (
	"mov	0x8879a40,%%eax			\n\t"
	"mov    $0x822be98,%%edx		\n\t"
	"cmpl   %%ecx,0x8879a84(,%%eax,4)	\n\t"
	"jle	1f				\n\t"
	"mov    0x8879aa4(,%%eax,4),%%eax	\n\t"
	"lea	(%%eax,%%ecx,4),%%edx		\n\t"
	"mov    0x4(%%eax),%%edx		\n\t"
	"lea	(%%eax,%%ecx,4),%%edx		\n\t"
	"mov	(%%edx),%%edx			\n\t"
	"1:					\n\t"
	"					\n\t"
	:"=d" (cmd_argv)
	:"c" (arg)
	:"eax"					);
	return (cmd_argv);
}

/*
============
SV_Cmd_ArgvBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void	SV_Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, SV_Cmd_Argv(arg), bufferLength );
}

