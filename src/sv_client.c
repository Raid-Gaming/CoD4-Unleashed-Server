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


#include "q_shared.h"
#include "qcommon_io.h"
#include "qcommon_mem.h"
#include "qcommon.h"
#include "huffman.h"
#include "msg.h"
#include "server.h"
#include "net_game_conf.h"
#include "misc.h"
#include "g_sv_shared.h"
#include "plugin_handler.h"
#include "q_platform.h"
#include "sys_main.h"
#include "punkbuster.h"
#include "scr_vm.h"
#include "cmd.h"
#include "sys_thread.h"
#include "hl2rcon.h"
#include "sv_auth.h"


#include <stdint.h>
#include <stdarg.h>
#include <string.h>

//AntiDoS
/*
int SV_ChallengeCookies(netadr_t from){
    char string[128];
    int var_01 = svs.time & 0xff000000;
    Com_sprintf(string, sizeof(string), "");
    return var_01;

}
*/
/*
=================
SV_GetChallenge

A "getchallenge" OOB command has been received
Returns a challenge number that can be used
in a subsequent connectResponse command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.

If we are authorizing, a challenge request will cause a packet
to be sent to the authorize server.

When an authorizeip is returned, a challenge response will be
sent to that ip.

ioquake3: we added a possibility for clients to add a challenge
to their packets, to make it more difficult for malicious servers
to hi-jack client connections.
Also, the auth stuff is completely disabled for com_standalone games
as well as IPv6 connections, since there is no way to use the
v4-only auth server for these new types of connections.
=================
*/
#ifdef COD4X17A
__optimize3 __regparm1 void SV_GetChallenge(netadr_t *from)
{
	int		i;
	int		oldest;
	int		oldestTime;
	int		oldestClientTime;
	int		clientChallenge;
	challenge_t	*challenge;

	oldest = 0;
	oldestClientTime = oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	challenge = &svse.challenges[0];
	clientChallenge = atoi(SV_Cmd_Argv(1));

	for(i = 0 ; i < MAX_CHALLENGES ; i++, challenge++)
	{
		if(NET_CompareAdr(from, &challenge->adr))
		{
			if(challenge->connected){
				Com_Memset(challenge, 0 ,sizeof(challenge_t));
				continue;
			}

			if(challenge->time < oldestClientTime)
				oldestClientTime = challenge->time;
			break;
		}

		if(challenge->time < oldestTime)
		{
			oldestTime = challenge->time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// this is the first time this client has asked for a challenge
		challenge = &svse.challenges[oldest];
		challenge->clientChallenge = clientChallenge;
		challenge->adr = *from;
		challenge->firstTime = svs.time;
		challenge->connected = qfalse;
		challenge->pbguid[31] = 0;
		Q_strncpyz(challenge->pbguid, SV_Cmd_Argv(2),33);
		challenge->ipAuthorize = 0;
		challenge->challenge = ( (rand() << 16) ^ rand() ) ^ svs.time;
	}

	challenge->time = svs.time;


	// Drop the authorize stuff if this client is coming in via IPv6 as the auth server does not support ipv6.
	// Drop also for addresses coming in on local LAN and for stand-alone games independent from id's assets.
	if(challenge->adr.type == NA_IP && svse.authorizeAddress.type != NA_DOWN && !Sys_IsLANAddress(from) && sv_authorizemode->integer != -1)
	{

		// look up the authorize server's IP
		if(svse.authorizeAddress.type == NA_BAD)
		{
			Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
			if (NET_StringToAdr(AUTHORIZE_SERVER_NAME, &svse.authorizeAddress, NA_IP))
			{
				svse.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
				Com_Printf( "%s resolved to %s\n", AUTHORIZE_SERVER_NAME, NET_AdrToString(&svse.authorizeAddress));
			}
		}

		// we couldn't contact the auth server, let them in.
		if(svse.authorizeAddress.type == NA_BAD){
			Com_Printf("Couldn't resolve auth server address\n");
			svse.authorizeAddress.type = NA_DOWN;
			return;
                }


		if(svse.authorizeAddress.type == NA_IP && challenge->adr.type == NA_IP && NET_CompareBaseAdr(from, &svse.authorizeAddress)){
			//Reset the default socket so that this is forwarded to all sockets
			if(NET_GetDefaultCommunicationSocket() == NULL){
				NET_RegisterDefaultCommunicationSocket(from);
				svse.authorizeAddress.sock = from->sock;
			}
			from->port = BigShort(PORT_AUTHORIZE);
			NET_OutOfBandPrint( NS_SERVER, from, "getIpAuthorize %i %s \"\" 0", challenge->challenge, NET_AdrToStringShort(from));
			return;
		}

		// if they have been challenging for a long time and we
		// haven't heard anything from the authorize server, go ahead and
		// let them in, assuming the id server is down
		else if(svs.time - challenge->firstTime > AUTHORIZE_TIMEOUT)
		{
			if(challenge->ipAuthorize == 0)
			{
				Com_PrintWarning( "Activisions authorize server timed out - Accept client without validating him\n" );
			}
		}else{

			if(!challenge->pbguid[31]){
				return;
			}

			// otherwise send their ip to the authorize server
			Com_DPrintf( "sending getIpAuthorize for %s\n", NET_AdrToString( from ));

			// the 0 is for backwards compatibility with obsolete sv_allowanonymous flags
			// getIpAuthorize <challenge> <IP> <game> 0 <auth-flag>
			NET_OutOfBandPrint( NS_SERVER, from, "needcdkey");

			NET_OutOfBandPrint( NS_SERVER, &svse.authorizeAddress,
				"getIpAuthorize %i %s \"\" 0 PB \"%s\"",challenge->challenge, NET_AdrToStringShort(from), challenge->pbguid );

			return;
		}
	}

	if(!challenge->pbguid[31]){
		return;
	}

	challenge->pingTime = com_frameTime;

	NET_OutOfBandPrint(NS_SERVER, &challenge->adr, "challengeResponse %d %d",
			   challenge->challenge, clientChallenge);
}


/*
====================
SV_AuthorizeIpPacket

A packet has been returned from the authorize server.
If we have a challenge adr for that ip, send the
challengeResponse to it
====================
*/

__optimize3 __regparm1 void SV_AuthorizeIpPacket( netadr_t *from ) {
	int	challenge;
	int	i;
	char	*s;
	char	*r;
	char	*p;

	if ( !NET_CompareBaseAdr( from, &svse.authorizeAddress )) {
		Com_Printf( "SV_AuthorizeIpPacket: not from authorize server\n" );
		return;
	}

	//This binds the serveraddress to a specific socket - No longer will this server be contacted over all sockets
	if(NET_GetDefaultCommunicationSocket() == NULL){
		NET_RegisterDefaultCommunicationSocket(from);
	}

	challenge = atoi( SV_Cmd_Argv( 1 ) );

	for (i = 0 ; i < MAX_CHALLENGES ; i++) {
		if ( svse.challenges[i].challenge == challenge ) {
			break;
		}
	}
	if ( i == MAX_CHALLENGES ) {
		Com_Printf( "SV_AuthorizeIpPacket: challenge not found\n" );
		return;
	}
	if(svse.challenges[i].connected){
	    return;
	}
	// send a packet back to the original client
	s = SV_Cmd_Argv( 2 );
	r = SV_Cmd_Argv( 3 );	// reason
	p = SV_Cmd_Argv( 5 );	//pbguid

	if ( !Q_stricmp( s, "demo" ) ) {
		// they are a demo client trying to connect to a real server
		NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "error\nServer is not a demo server\n" );
		// clear the challenge record so it won't timeout and let them through
		Com_Memset( &svse.challenges[i], 0, sizeof( svse.challenges[i] ) );
		return;
	}
	if ( !Q_stricmp( s, "accept" )) {
		if(Q_stricmp( p, svse.challenges[i].pbguid)){
			NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "error\nEXE_ERR_BAD_CDKEY");
			Com_Memset( &svse.challenges[i], 0, sizeof( svse.challenges[i] ));
			return;
		}else{
		        NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "challengeResponse %i", svse.challenges[i].challenge);
		        svse.challenges[i].pingTime = com_frameTime;
		        NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "print\n^2Success...");
			svse.challenges[i].ipAuthorize = 1; //CD-KEY was valid
		        return;
		}
	}

	if (!Q_stricmp( s, "deny" )) {

		svse.challenges[i].ipAuthorize = -1;
		if(!Q_stricmp(r, "CLIENT_UNKNOWN_TO_AUTH")){

			NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "print\nUnknown how to auth client\n");

			if(svs.time - svse.challenges[i].firstTime > AUTHORIZE_TIMEOUT)
			{

				NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "challengeResponse %i", svse.challenges[i].challenge);
			}
			return;
		}



		NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "print\n^1Auth Failed.");
		NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "challengeResponse %i", svse.challenges[i].challenge);
		svse.challenges[i].pingTime = com_frameTime;
		svse.challenges[i].ipAuthorize = -1; //CD-KEY was invalid
		return;


		Com_Memset( &svse.challenges[i], 0, sizeof( svse.challenges[i] ) );
		return;
	}

	NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "print\n^1Auth Failed");
	NET_OutOfBandPrint( NS_SERVER, &svse.challenges[i].adr, "challengeResponse %i", svse.challenges[i].challenge);
	svse.challenges[i].pingTime = com_frameTime;
	svse.challenges[i].ipAuthorize = -1; //CD-KEY was invalid
}


#else

__optimize3 __regparm1 void SV_GetChallenge(netadr_t *from)
{
	int	clientChallenge;
	int	challenge;

	if(from->type == NA_IP && svse.authorizeAddress.type != NA_DOWN)
	{
		/* This part is required to keep the server registered on the masterserver */
		// look up the authorize server's IP
		if(svse.authorizeAddress.type == NA_BAD)
		{
			Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
			if (NET_StringToAdr(AUTHORIZE_SERVER_NAME, &svse.authorizeAddress, NA_IP))
			{
				svse.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
				Com_Printf( "%s resolved to %s\n", AUTHORIZE_SERVER_NAME, NET_AdrToString(&svse.authorizeAddress));
			}
		}
		if(svse.authorizeAddress.type == NA_IP && from->type == NA_IP && NET_CompareBaseAdr(from, &svse.authorizeAddress))
		{
			//Reset the default socket so that this is forwarded to all sockets
			if(NET_GetDefaultCommunicationSocket() == NULL){
				NET_RegisterDefaultCommunicationSocket(from);
				svse.authorizeAddress.sock = from->sock;
			}
			from->port = BigShort(PORT_AUTHORIZE);
			challenge = NET_CookieHash(from);
			NET_OutOfBandPrint( NS_SERVER, from, "getIpAuthorize %i %s \"\" 0", challenge, NET_AdrToStringShort(from));
			return;
		}
	}
	challenge = NET_CookieHash(from);
	clientChallenge = atoi(SV_Cmd_Argv(1));
	NET_OutOfBandPrint(NS_SERVER, from, "challengeResponse %d %d 0 xproto", challenge, clientChallenge);

}



#endif

/*
==================
SV_DirectConnect

A "connect" OOB command has been received
==================
*/

__optimize3 __regparm1 void SV_DirectConnect( netadr_t *from ) {
	char		userinfo[MAX_INFO_STRING];
	int			reconnectTime;

	int			j;
	int			i;
	client_t		*cl, *newcl;
	int			count;
	char			nick[33];
	char			ip_str[128];
	char			pbguid[33];
	int			clientNum;
	int			version;
	unsigned int		qport;
	int			challenge;
	char			*password;
	char			denied[MAX_STRING_CHARS];
	const char		*denied2;
	qboolean		canreserved;

	Q_strncpyz( userinfo, SV_Cmd_Argv(1), sizeof(userinfo) );
	challenge = atoi( Info_ValueForKey( userinfo, "challenge" ) );
	qport = atoi( Info_ValueForKey( userinfo, "qport" ) );

#ifdef COD4X17A

	qboolean		pluginreject;
	int			c;

	// see if the challenge is valid
	int		ping;
	for (c=0 ; c < MAX_CHALLENGES ; c++) {
		if (NET_CompareAdr(from, &svse.challenges[c].adr)) {
			if ( challenge == svse.challenges[c].challenge ) {
				break;		// good
			}
		}
	}

	if (c == MAX_CHALLENGES || challenge == 0) {
		NET_OutOfBandPrint( NS_SERVER, from, "error\nNo or bad challenge for address.\n" );
		return;
	}

#else

	if(challenge != NET_CookieHash(from))
	{
		NET_OutOfBandPrint( NS_SERVER, from, "error\nNo or bad challenge for address.\n" );
		return;
	}

#endif

	newcl = NULL;

	// quick reject
	for (j=0, cl=svs.clients ; j < sv_maxclients->integer ; j++, cl++) {

		if ( NET_CompareBaseAdr( from, &cl->netchan.remoteAddress ) && (cl->netchan.qport == qport || from->port == cl->netchan.remoteAddress.port )){

			reconnectTime = (svs.time - cl->lastConnectTime);
			if (reconnectTime < (sv_reconnectlimit->integer * 1000)) {
				Com_Printf("%s -> reconnect rejected : too soon\n", NET_AdrToString (from));
				NET_OutOfBandPrint( NS_SERVER, from, "print\nReconnect limit in effect... (%i)\n",
							(sv_reconnectlimit->integer - (reconnectTime / 1000)));
				return;
			}else{
				if(cl->state > CS_ZOMBIE){	//Free up used CGame-Resources first
					SV_FreeClient( cl );
				}
				newcl = cl;
				Com_Printf("reconnected: %s\n", NET_AdrToString(from));
				cl->lastConnectTime = svs.time;
				break;
			}
		}else if ( NET_CompareBaseAdr( from, &cl->netchan.remoteAddress ) && cl->state == CS_CONNECTED){
			NET_OutOfBandPrint( NS_SERVER, from,
				"error\nConnection refused:\nAn uncompleted connection from %s has been detected\nPlease try again later\n",
				NET_AdrToString(&cl->netchan.remoteAddress));
			Com_Printf("Rejected connection from %s. This is a Fake-Player-DoS protection\n", NET_AdrToString(&cl->netchan.remoteAddress));
#ifdef COD4X17A
			Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
			return;
		}
	}

	// force the IP key/value pair so the game can filter based on ip
	Com_sprintf(ip_str, sizeof(ip_str), "%s", NET_AdrToConnectionString( from ));
	Info_SetValueForKey( userinfo, "ip", ip_str );

#ifdef COD4X17A

	if(!newcl && svse.challenges[c].pingTime){
	        ping = com_frameTime - svse.challenges[c].pingTime;
    		Com_Printf( "Client %i connecting with %i challenge ping\n", c, ping );

        	svse.challenges[c].pingTime = 0;

		if ( sv_minPing->integer && ping < sv_minPing->integer ) {
			// don't let them keep trying until they get a big delay
			NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is for high ping players only minping: %i ms but your ping was: %i ms\n",sv_minPing->integer, ping);
			Com_Printf("Client %i rejected on a too low ping\n", c);
			// reset the address otherwise their ping will keep increasing
			// with each connect message and they'd eventually be able to connect
			svse.challenges[c].adr.port = 0;
			Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
			return;
		}
		if ( sv_maxPing->integer && ping > sv_maxPing->integer ) {
			NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is for low ping players only maxping: %i ms but your ping was: %i ms\n",sv_maxPing->integer, ping);
			Com_Printf("Client %i rejected on a too high ping\n", c);
			Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
			return;
		}
	}

#endif

	Q_strncpyz(nick, Info_ValueForKey( userinfo, "name" ), 33);

	denied[0] = '\0';

	SV_PlayerBannedByip(from, denied, sizeof(denied));
	if(denied[0]){
            NET_OutOfBandPrint( NS_SERVER, from, "error\n%s\n", denied);
		Com_Printf("Rejecting a connection from a banned network address: %s\n", NET_AdrToString(from));
#ifdef COD4X17A
	    Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
	    return;
	}

#ifdef COD4X17A
	Q_strncpyz(pbguid, svse.challenges[c].pbguid, sizeof(pbguid));
#else
	Q_strncpyz(pbguid, Info_ValueForKey( userinfo, "pbguid" ), sizeof(pbguid));
#endif


#ifdef COD4X17A
	version = atoi( Info_ValueForKey( userinfo, "protocol" ));
	if ( version != sv_protocol->integer ) {
		if( sv_protocol->integer == 6 && version < 6)
		{
			NET_OutOfBandPrint( NS_SERVER, from, "error\nServer uses a different protocol version: %i\n You have to install the update to Call of Duty 4  v1.7", sv_protocol->integer );
		}else{
			NET_OutOfBandPrint( NS_SERVER, from, "error\nServer uses a different protocol version: %i\n You use protocol version: %i", sv_protocol->integer, version );
		}
		
		Com_Printf("rejected connect from version %i\n", version);
		Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
		return;
	}

	if(sv_authorizemode->integer == 1 && svse.challenges[c].ipAuthorize == -1)
	{
		pluginreject = qtrue;

		PHandler_Event(PLUGINS_ONPLAYERCONNECTAUTHFAIL, from, &svse.challenges[c].pbguid, userinfo, &svse.challenges[c].ipAuthorize, &pluginreject);
		if(pluginreject)
		{
			NET_OutOfBandPrint( NS_SERVER, from, "error\n^1Someone is using this CD Key");
			if(svse.challenges[c].firstTime + 1000*30 < svs.time)
			{
				Com_Memset(&svse.challenges[c], 0, sizeof(challenge_t));
			}
			return;
		}
	}
#else
	version = atoi( Info_ValueForKey( userinfo, "protocol" ));
	if ( version != sv_protocol->integer ) {

#ifdef COD4X18UPDATE
		if(version < 7)
		{
			Com_Printf("Have to fix up old client which reports version %d\n", version);
		}else{
#endif
			NET_OutOfBandPrint( NS_SERVER, from, "error\nThis server requires protocol version: %d\nPlease restart CoD4 and see on the main-menu if a new update is available\n", sv_protocol->integer);
			Com_Printf("rejected connect from version %i\n", version);
			return;
#ifdef COD4X18UPDATE
		}
#endif
	}
#endif

#ifdef COD4X18UPDATE
	if(version < 7)
	{
		
	}else
#endif
	if(strlen(pbguid) < 10)
	{
		NET_OutOfBandPrint( NS_SERVER, from, "error\nConnection rejected: No or invalid GUID found/provided.\n" );
		Com_Printf("Rejected a connection: No or invalid GUID found/provided. Length: %i\n",
		strlen(pbguid));
#ifdef COD4X17A
		Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
		return;
	}



	// find a client slot:
	// if "sv_privateClients" is set > 0, then that number
	// of client slots will be reserved for connections that
	// have "password" set to the value of "sv_privatePassword"
	// Info requests will report the maxclients as if the private
	// slots didn't exist, to prevent people from trying to connect
	// to a full server.
	// This is to allow us to reserve a couple slots here on our
	// servers so we can play without having to kick people.

	//Get new slot for client
	// check for privateClient password
	password = Info_ValueForKey( userinfo, "password" );
	if(!newcl)
	{
		if ( !strcmp( password, sv_privatePassword->string ))
		{ 
			canreserved = qtrue;
		}else{
			canreserved = qfalse;
		}
		
#ifdef COD4X17A
		PHandler_Event(PLUGINS_ONPLAYERWANTRESERVEDSLOT, from, pbguid, userinfo, svse.challenges[c].ipAuthorize, &canreserved);
#else
		PHandler_Event(PLUGINS_ONPLAYERWANTRESERVEDSLOT, from, pbguid, userinfo, 0, &canreserved);
#endif
		if ( canreserved == qtrue) 
		{
			for ( j = 0; j < sv_privateClients->integer ; j++) {
				cl = &svs.clients[j];
				if (cl->state == CS_FREE) {
					newcl = cl;
					break;
				}
			}
		}
	}
	if(*sv_password->string && Q_strncmp(sv_password->string, password, 32)){
		NET_OutOfBandPrint( NS_SERVER, from, "error\nThis server has set a join-password\n^1Invalid Password\n");
		Com_Printf("Connection rejected from %s - Invalid Password\n", NET_AdrToString(from));
#ifdef COD4X17A
		Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
		return;
	}
	//Process queue
	for(i = 0 ; i < 10 ; i++){//Purge all older players from queue
	    if(svse.connectqueue[i].lasttime+21 < Com_GetRealtime()){
		svse.connectqueue[i].lasttime = 0;
		svse.connectqueue[i].firsttime = 0;
		svse.connectqueue[i].challengeslot = 0;
		svse.connectqueue[i].attempts = 0;
	    }
	}
	for(i = 0 ; i < 10 ; i++){//Move waiting players up in queue if there is a purged slot
	    if(svse.connectqueue[i].lasttime != 0){
		if(svse.connectqueue[i+1].challengeslot == svse.connectqueue[i].challengeslot){
		    svse.connectqueue[i+1].lasttime = 0;
		    svse.connectqueue[i+1].firsttime = 0;
		    svse.connectqueue[i+1].challengeslot = 0;
		    svse.connectqueue[i+1].attempts = 0;
		}
	    }else{
		Com_Memcpy(&svse.connectqueue[i],&svse.connectqueue[i+1],(9-i)*sizeof(connectqueue_t));
	    }
	}
	for(i = 0 ; i < 10 ; i++){//Find highest slot or the one which is already assigned to this player
#ifdef COD4X17A
	    if(svse.connectqueue[i].firsttime == 0 || svse.connectqueue[i].challengeslot == c){
#else
	    if(svse.connectqueue[i].firsttime == 0 || svse.connectqueue[i].challengeslot == challenge){
#endif
		break;
	    }
	}

	if(i == 0 && !newcl){
		for ( j = sv_privateClients->integer; j < sv_maxclients->integer ; j++) {
			cl = &svs.clients[j];
			if (cl->state == CS_FREE) {
				newcl = cl;
				svse.connectqueue[0].lasttime = 0;
				svse.connectqueue[0].firsttime = 0;
				svse.connectqueue[0].challengeslot = 0;
				svse.connectqueue[0].attempts = 0;
				break;
			}
		}
	}

	if ( !newcl ) {
		if(i == 10){
		    NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is full. More than 10 players are in queue.\n" );
		    return;
		}else{
		    NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is full. %i players wait before you.\n",i);
		}
		if(svse.connectqueue[i].attempts > 30){
		    NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is full. %i players wait before you.\n",i);
		    svse.connectqueue[i].attempts = 0;
		}else if(svse.connectqueue[i].attempts > 15 && i > 1){
		    NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is full. %i players wait before you.\n",i);
		    svse.connectqueue[i].attempts = 0;
		}else if(svse.connectqueue[i].attempts > 5 && i > 3){
		    NET_OutOfBandPrint( NS_SERVER, from, "error\nServer is full. %i players wait before you.\n",i);
		    svse.connectqueue[i].attempts = 0;
		}
		if(svse.connectqueue[i].firsttime == 0){
		    svse.connectqueue[i].firsttime = Com_GetRealtime();
		}
		svse.connectqueue[i].attempts++;
		svse.connectqueue[i].lasttime = Com_GetRealtime();
#ifdef COD4X17A
		svse.connectqueue[i].challengeslot = c;
#else
		svse.connectqueue[i].challengeslot = challenge;
#endif
		return;
	}

#ifdef COD4X17A
	//gotnewcl:
	Com_Memset(newcl, 0x00, sizeof(client_t));
#else
    #ifdef COD4X18UPDATE
	if(version == sv_protocol->integer || newcl->challenge != challenge || newcl->state != CS_CONNECTED)
	{
		Com_Memset(newcl, 0x00, sizeof(client_t));
	}
    #else
	Com_Memset(newcl, 0x00, sizeof(client_t));
    #endif
#endif

#ifdef COD4X17A
	newcl->authentication = svse.challenges[c].ipAuthorize;
#else
	newcl->authentication = 0;
#endif
	newcl->power = 0; //Sets the default power for the client
        newcl->challenge = challenge; // save the challenge
	// (build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized)


        clientNum = newcl - svs.clients;

        Q_strncpyz(cl->originguid, pbguid, 33);
        Q_strncpyz(cl->pbguid, pbguid, 33);	// save the pbguid
#ifdef COD4X17A
        if(newcl->authentication != 1 && sv_authorizemode->integer != -1){
            Com_Memset(newcl->pbguid, '0', 8);
        }
#endif
        //    char ret[33];
        //    Com_sprintf(ret,sizeof(ret),"NoGUID*%.2x%.2x%.2x%.2x%.4x",from->ip[0],from->ip[1],from->ip[2],from->ip[3],from->port);
        //    Q_strncpyz(newcl->pbguid, ret, sizeof(newcl->pbguid));	// save the pbguid

        denied[0] = '\0';

        // save the userinfo
        Q_strncpyz(newcl->userinfo, userinfo, sizeof(newcl->userinfo) );

        PHandler_Event(PLUGINS_ONPLAYERCONNECT, clientNum, from, newcl->originguid, userinfo, newcl->authentication, denied, sizeof(denied));

        SV_PlayerIsBanned(newcl->uid, newcl->pbguid, from, denied, sizeof(denied));

        if(denied[0]){
                NET_OutOfBandPrint( NS_SERVER, from, "error\n%s", denied);
		Com_Printf("Rejecting a connection from a banned GUID/UID\n");
#ifdef COD4X17A
		Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
                svse.connectqueue[i].lasttime = 0;
                svse.connectqueue[i].firsttime = 0;
                svse.connectqueue[i].challengeslot = 0;
		svse.connectqueue[i].attempts = 0;
		return;
        }

#ifdef COD4X17A
#ifdef PUNKBUSTER

	const char		*PunkBusterStatus;

	if(strstr(Cvar_GetVariantString("noPbGuids"), newcl->originguid) && 32 == strlen(newcl->originguid) && newcl->authentication == 1){
		newcl->noPb = qtrue;
	}
	if(newcl->noPb == qfalse){
		PunkBusterStatus = PbAuthClient(NET_AdrToString(from), atoi(Info_ValueForKey( userinfo, "cl_punkbuster" )), newcl->pbguid);
		if(PunkBusterStatus){
		    NET_OutOfBandPrint( NS_SERVER, from, "%s", PunkBusterStatus);
		    Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
		    return;
		}
	}
#endif
#endif
	newcl->unsentVoiceData = 0;
	newcl->hasVoip = 1;
	newcl->gentity = SV_GentityNum(clientNum);
	newcl->clscriptid = Scr_AllocArray();
	newcl->protocol = version;
#ifndef COD4X17A
#ifdef COD4X18UPDATE
	if(newcl->protocol != sv_protocol->integer)
	{
		newcl->needupdate = qtrue;
	}else{
		newcl->needupdate = qfalse;
	}
#endif
#endif

	// get the game a chance to reject this connection or modify the userinfo
	denied2 = ClientConnect(clientNum, newcl->clscriptid);

	if ( denied2 ) {
		NET_OutOfBandPrint( NS_SERVER, from, "error\n%s\n", denied2 );
		Com_Printf("Game rejected a connection: %s\n", denied2);
		SV_FreeClientScriptId(newcl);
#ifdef COD4X17A
		Com_Memset( &svse.challenges[c], 0, sizeof( svse.challenges[c] ));
#endif
		return;
	}

	// save the address
	// init the netchan queue
	Netchan_Setup( NS_SERVER, &newcl->netchan, *from, qport,
			 newcl->unsentBuffer, sizeof(newcl->unsentBuffer),
			 newcl->fragmentBuffer, sizeof(newcl->fragmentBuffer));

#ifdef COD4X17A
	svse.challenges[c].connected = qtrue;
#else
	ReliableMessageSetup(&newcl->relmsg, qport, NS_SERVER, from);
#endif
	Com_Printf( "Going from CS_FREE to CS_CONNECTED for %s num %i guid %s from: %s\n", nick, clientNum, newcl->pbguid, NET_AdrToConnectionString(from));
	
	Q_strncpyz(newcl->xversion, Info_ValueForKey( userinfo, "xver"), sizeof(newcl->xversion));

	newcl->state = CS_CONNECTED;
	newcl->nextSnapshotTime = svs.time;
	newcl->lastPacketTime = svs.time;
	newcl->lastConnectTime = svs.time;

#ifndef COD4X17A
#ifdef COD4X18UPDATE
	if(newcl->needupdate)
	{
		newcl->nextSnapshotTime = 0x7fffffff;
	}
	SV_UserinfoChanged(newcl);

	if(!newcl->updateconnOK && newcl->needupdate)
	{
		SV_ConnectWithUpdateProxy(newcl);
		return;
	}
#endif
#endif

	SV_UserinfoChanged(newcl);

	// send the connect packet to the client
	if(sv_modStats->boolean){
	    NET_OutOfBandPrint( NS_SERVER, from, "connectResponse %s", fs_gameDirVar->string);
	}else{
	    NET_OutOfBandPrint( NS_SERVER, from, "connectResponse");
	}
	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	newcl->gamestateMessageNum = -1; //newcl->gamestateMessageNum = -1;


	// if this was the first client on the server, or the last client
	// the server can hold, send a heartbeat to the master.
	for (j=0, cl=svs.clients, count=0; j < sv_maxclients->integer; j++, cl++) {
		if (cl->state >= CS_CONNECTED ) {
			count++;
		}
	}
	if ( count == 1 || count == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}

#ifdef COD4X17A
__optimize3 __regparm2 void SV_ReceiveStats(netadr_t *from, msg_t* msg){

	unsigned short qport;
	client_t *cl;
	byte curstatspacket;
	byte var_02;
	int buffersize;

	qport = MSG_ReadShort( msg );
	
	// find which client the message is from
	cl = SV_ReadPackets(from, qport);
	if(cl == NULL)
	{
		Com_DPrintf("SV_ReceiveStats: Received statspacket from disconnected remote client: %s qport: %d\n", NET_AdrToString(from), qport);
		return;
	}
	
	curstatspacket = MSG_ReadByte(msg);
	if(curstatspacket > 6){
		Com_Printf("Invalid stat packet %i of stats data\n", curstatspacket);
		return;
	}
	Com_Printf("Received packet %i of stats data\n", curstatspacket);
	if((curstatspacket+1)*1240 >= sizeof(cl->stats)){
		buffersize = sizeof(cl->stats)-(curstatspacket*1240);
	}else{
		buffersize = 1240;
	}
	MSG_ReadData(msg, &cl->stats[1240*curstatspacket], buffersize);

	cl->receivedstats |= (1 << curstatspacket);
	var_02 = cl->receivedstats;
	var_02 = ~var_02;
	var_02 = var_02 & 127;
	cl->lastPacketTime = svs.time;

	NET_OutOfBandPrint( NS_SERVER, from, "statResponse %i", var_02 );
}

#else
#ifdef COD4X18UPDATE


__optimize3 __regparm2 void SV_ReceiveStats(netadr_t *from, msg_t* msg){

	unsigned short qport;
	client_t *cl;
	byte var_02;

	qport = MSG_ReadShort( msg );
	// find which client the message is from
	cl = SV_ReadPackets(from, qport);
	if(cl == NULL)
	{
		Com_DPrintf("SV_ReceiveStats: Received statspacket from disconnected remote client: %s qport: %d\n", NET_AdrToString(from), qport);
		return;
	}

	cl->receivedstats = 0x7F;
	var_02 = cl->receivedstats;
	var_02 = ~var_02;
	var_02 = var_02 & 127;
	cl->lastPacketTime = svs.time;

	NET_OutOfBandPrint( NS_SERVER, from, "statResponse %i", var_02 );
}



#endif
#endif


/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C friendly form.
=================
*/
void SV_UserinfoChanged( client_t *cl ) {
	char	*val;
	char	ip[128];
	int	i;
	int	len;

	// name for C code
	Q_strncpyz( cl->name, Info_ValueForKey (cl->userinfo, "name"), sizeof(cl->name) );

	if(!Q_isprintstring(cl->name) || strstr(cl->name,"ID_") || strstr(cl->name,"&&") || strstr(cl->name,"///") || Q_PrintStrlen(cl->name) < 3){
		if(cl->state == CS_ACTIVE){
			if(!Q_isprintstring(cl->name)) SV_SendServerCommand(cl, "c \"^5Playernames can not contain advanced ASCII-characters\"");
			if(strlen(cl->name) < 3) SV_SendServerCommand(cl, "c \"^5Playernames can not be shorter than 3 characters\"");
		}
		if(cl->uid <= 0){
		    Com_sprintf(cl->name, 16, "CID_%i", cl - svs.clients);
		    cl->usernamechanged = UN_NEEDUID;
		} else {
		    Com_sprintf(cl->name, 16, "ID_%i:%i", cl->uid / 100000000, cl->uid % 100000000);
		    cl->usernamechanged = UN_OK;
		}
		Info_SetValueForKey( cl->userinfo, "name", cl->name);
	}else{
	    cl->usernamechanged = UN_VERIFYNAME;
	}
	Q_strncpyz(cl->shortname, cl->name, sizeof(cl->shortname));
	// rate command
	// if the client is on the same subnet as the server and we aren't running an
	// internet public server, assume they don't need a rate choke
	if ( Sys_IsLANAddress( &cl->netchan.remoteAddress )) {
		cl->rate = 1048576;	// lans should not rate limit
	} else {
		val = Info_ValueForKey (cl->userinfo, "rate");
		if (strlen(val)) {
			i = atoi(val);
			cl->rate = i;
			if (cl->rate < 2500) {
				cl->rate = 2500;
			} else if (cl->rate >= 25000) {
				cl->rate = sv_maxRate->integer;
			}
		} else {
			cl->rate = 2500;
		}
	}
	// snaps command
	val = Info_ValueForKey (cl->userinfo, "snaps");

	if(strlen(val))
	{
		i = atoi(val);

		if(i < 10)
			i = 10;
		else if(i > sv_fps->integer)
			i = sv_fps->integer;
		else if(i == 30)
			i = sv_fps->integer;

		cl->snapshotMsec = 1000 / i;
	}
	else
		cl->snapshotMsec = 50;

	val = Info_ValueForKey(cl->userinfo, "cl_voice");
	cl->hasVoip = atoi(val);

	// TTimo
	// maintain the IP information
	// the banning code relies on this being consistently present
	if( NET_IsLocalAddress(cl->netchan.remoteAddress) )
		Q_strncpyz(ip,  "localhost", sizeof(ip));
	else
		Com_sprintf(ip, sizeof(ip), "%s", NET_AdrToConnectionString( &cl->netchan.remoteAddress ));

	val = Info_ValueForKey( cl->userinfo, "ip" );

	if( val[0] )
		len = strlen( ip ) - strlen( val ) + strlen( cl->userinfo );
	else
		len = strlen( ip ) + 4 + strlen( cl->userinfo );

	if( len >= MAX_INFO_STRING )
		SV_DropClient( cl, "userinfo string length exceeded" );
	else
		Info_SetValueForKey( cl->userinfo, "ip", ip );

	cl->wwwDownload = qfalse;
	if(Info_ValueForKey(cl->userinfo, "cl_wwwDownload"))
		cl->wwwDownload = qtrue;
		
	PHandler_Event(PLUGINS_ONCLIENTUSERINFOCHANGED, cl);

}


/*
==================
SV_UpdateUserinfo_f
==================
*//*
static void SV_UpdateUserinfo_f( client_t *cl ) {
	Q_strncpyz( cl->userinfo, Cmd_Argv(1), sizeof(cl->userinfo) );

	SV_UserinfoChanged( cl );
	// call prog code to allow overrides
	VM_Call( gvm, GAME_CLIENT_USERINFO_CHANGED, cl - &svs.clients );
}*/


/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalMessage() will handle that
=====================
*/
__cdecl void SV_DropClient( client_t *drop, const char *reason ) {
	int i;
	int clientnum;
	char var_01[2];
	const char *dropreason;
	char clientName[64];


	if ( drop->state <= CS_ZOMBIE ) {
		return;     // already dropped
	}

	if(drop->demorecording)
	{
		SV_StopRecord(drop);
	}
	Q_strncpyz(clientName, drop->name, sizeof(clientName));

	clientnum = drop - svs.clients;

	if(drop->uid > 0 && strlen(drop->pbguid) == 32)
		SV_EnterLeaveLog("^4Client %s %s ^4left this server from slot %d with uid %d guid %s", drop->name, NET_AdrToString(&drop->netchan.remoteAddress), clientnum, drop->uid, drop->pbguid);
	else if(drop->uid > 0)
		SV_EnterLeaveLog("^4Client %s %s ^4left this server from slot %d with uid %d", drop->name, NET_AdrToString(&drop->netchan.remoteAddress), clientnum, drop->uid);
	else if(strlen(drop->pbguid) == 32)
		SV_EnterLeaveLog("^4Client %s %s ^4left this server from slot %d with guid %s", drop->name, NET_AdrToString(&drop->netchan.remoteAddress), clientnum, drop->pbguid);

	SV_FreeClient(drop);

	G_DestroyAdsForPlayer(drop);

#ifdef COD4X17A
	challenge_t *challenge;

	if ( !drop->gentity ) {
		// see if we already (maybe still ??) have a challenge for this ip
		challenge = &svse.challenges[0];

		for ( i = 0 ; i < MAX_CHALLENGES ; i++, challenge++ ) {
			if ( NET_CompareAdr( &drop->netchan.remoteAddress, &challenge->adr ) ) {
				Com_Memset(challenge, 0, sizeof(challenge_t));
				break;
			}
		}
	}
#else
	ReliableMessageDisconnect(&drop->relmsg);
#endif

	clientnum = drop - svs.clients;

	if(!reason)
		reason = "Unknown reason for dropping";

	if(!Q_stricmp(reason, "silent")){
		//Just disconnect him and don't tell anyone about it
		Com_Printf("Player %s^7, %i dropped: %s\n", clientName, clientnum, reason);
		drop->state = CS_ZOMBIE;        // become free in a few seconds

		HL2Rcon_EventClientLeave(clientnum);
		PHandler_Event(PLUGINS_ONPLAYERDC, drop, reason);	// Plugin event
		return;
	}


	if(SEH_StringEd_GetString( reason )){
		var_01[0] = 0x14;
		var_01[1] = 0;
	}else{
		var_01[0] = 0;
	}

	if(!Q_stricmp(reason, "EXE_DISCONNECTED")){
		dropreason = "EXE_LEFTGAME";
	} else {
		dropreason = reason;
	}

	// tell everyone why they got dropped
	SV_SendServerCommand(NULL, "%c \"\x15%s^7 %s%s\"\0", 0x65, clientName, var_01, dropreason);

	Com_Printf("Player %s^7, %i dropped: %s\n", clientName, clientnum, dropreason);

	SV_SendServerCommand_IW(NULL, 1, "%c %d", 0x4b, clientnum);

	// add the disconnect command

	drop->reliableSent = drop->reliableSequence = drop->reliableAcknowledge;	//Reset unsentBuffer and Sequence to ommit the outstanding junk from beeing transmitted
	drop->netchan.unsentFragments = qfalse;
	SV_SendServerCommand_IW( drop, 1, "%c \"%s\" PB\0", 0x77, dropreason);

	if(drop->netchan.remoteAddress.type == NA_BOT){
		drop->state = CS_FREE;  // become free now
		drop->netchan.remoteAddress.type = 0; //Reset the botflag
		Com_DPrintf( "Going to CS_FREE for Bot %s\n", clientName );
	}else{

		drop->state = CS_ZOMBIE;        // become free in a few seconds
		Com_DPrintf( "Going to CS_ZOMBIE for %s\n", clientName );
	}

	HL2Rcon_EventClientLeave(clientnum);

	PHandler_Event(PLUGINS_ONPLAYERDC,(void*)drop);	// Plugin event


	// if this was the last client on the server, send a heartbeat
	// to the master so it is known the server is empty
	// send a heartbeat now so the master will get up to date info
	// if there is already a slot for this ip, reuse it
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			break;
		}
	}
	if ( i == sv_maxclients->integer ) {
		SV_Heartbeat_f();
	}
}





/*
==================
SV_UserMove

The message usually contains all the movement commands
that were in the last three packets, so that the information
in dropped packets can be recovered.

On very fast clients, there may be multiple usercmd packed into
each of the backup packets.
==================
*/
__optimize3 __regparm3 void SV_UserMove( client_t *cl, msg_t *msg, qboolean delta ) {
	int i, key, clientNum;
	unsigned int *ackTime;
	unsigned int sysTime;
	int cmdCount;
	usercmd_t nullcmd;
	usercmd_t cmds[MAX_PACKET_USERCMDS];
	usercmd_t   *cmd, *oldcmd;
	playerState_t *ps;
//	extclient_t *extcl;


	if ( delta ) {
		cl->deltaMessage = cl->messageAcknowledge;
	} else {
		cl->deltaMessage = -1;
	}

	if(cl->reliableSequence - cl->reliableAcknowledge >= MAX_RELIABLE_COMMANDS){
		return;
	}

	cmdCount = MSG_ReadByte( msg );

	if ( cmdCount < 1 ) {
		Com_Printf( "cmdCount < 1\n" );
		return;
	}

	if ( cmdCount > MAX_PACKET_USERCMDS ) {
		Com_Printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	// save time for ping calculation
	ackTime = &cl->frames[ cl->messageAcknowledge & PACKET_MASK ].messageAcked;
	sysTime = Sys_Milliseconds();
	if(*ackTime == 0xFFFFFFFF)
		*ackTime = sysTime;


	clientNum = cl - svs.clients;
//	extcl = &svse.extclients[clientNum];

	// use the checksum feed in the key
	key = sv.checksumFeed;
	// also use the message acknowledge
	key ^= cl->messageAcknowledge;
	// also use the last acknowledged server command in the key
//	key ^= Com_HashKey( extcl->reliableCommands[ cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ].command, 32 );
	key ^= Com_HashKey( cl->reliableCommands[ cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ].command, 32 );

	ps = SV_GameClientNum( clientNum );

	oldcmd = &nullcmd;

	MSG_SetDefaultUserCmd(ps, oldcmd);


	for ( i = 0 ; i < cmdCount ; i++ ) {
		cmd = &cmds[i];
		MSG_ReadDeltaUsercmdKey( msg, key, oldcmd, cmd );
		if( ! BG_IsWeaponValid(ps, cmd->weapon) ){
			cmd->weapon = ps->weapon;
		}
		if( ! BG_IsWeaponValid(ps, cmd->offHandIndex) ){
			cmd->offHandIndex = ps->offHandIndex;
		}

		oldcmd = cmd;
	}

	cl->unknownUsercmd1 = MSG_ReadLong(msg);
	cl->unknownUsercmd2 = MSG_ReadLong(msg);
	cl->unknownUsercmd3 = MSG_ReadLong(msg);
	cl->unknownUsercmd4 = MSG_ReadLong(msg);


	// TTimo
	// catch the no-cp-yet situation before SV_ClientEnterWorld
	// if CS_ACTIVE, then it's time to trigger a new gamestate emission
	// if not, then we are getting remaining parasite usermove commands, which we should ignore

	if ( sv_pure->integer != 0 && cl->pureAuthentic == 0 /*&& !cl->gotCP*/ ) {
		if ( cl->state == CS_ACTIVE ) {
			// we didn't get a cp yet, don't assume anything and just send the gamestate all over again
			Com_DPrintf( "%s: didn't get cp command, resending gamestate\n", cl->name);
			SV_SendClientGameState( cl );
		}
		return;
	}

	// if this is the first usercmd we have received
	// this gamestate, put the client into the world
	if ( cl->state == CS_PRIMED ) {
		SV_ClientEnterWorld( cl, &cmds[0] );
		return;
		// the moves can be processed normaly
	}

	if ( cl->state != CS_ACTIVE ) {
		cl->deltaMessage = -1;
		return;
	}

	// usually, the first couple commands will be duplicates
	// of ones we have previously received, but the servertimes
	// in the commands will cause them to be immediately discarded
	for ( i =  0 ; i < cmdCount ; i++ ) {
		// if this is a cmd from before a map_restart ignore it
		if ( cmds[i].serverTime > cmds[cmdCount - 1].serverTime ) {
			continue;
		}
		// extremely lagged or cmd from before a map_restart
		//if ( cmds[i].serverTime > svs.time + 3000 ) {
		//	continue;
		//}
		// don't execute if this is an old cmd which is already executed
		// these old cmds are included when cl_packetdup > 0
		if ( cmds[i].serverTime <= cl->lastUsercmd.serverTime ) {   // Q3_MISSIONPACK
	//		if ( cmds[i].serverTime > cmds[cmdCount-1].serverTime ) {
			continue;   // from just before a map_restart
		}
		if(sysTime >= cl->clFrameCalcTime){
			cl->clFrameCalcTime = sysTime + 1000;
			cl->clFPS = cl->clFrames;
			cl->clFrames = 0;
		}
		cl->clFrames++;

		SV_ClientThink( cl, &cmds[ i ] );
	
		PHandler_Event(PLUGINS_ONCLIENTMOVECOMMAND, cl, &cmds[ i ]);

		if(cl->demorecording && !cl->demowaiting)
			SV_WriteDemoArchive(cl);
	}
}

/*
==================
SV_ClientEnterWorld
==================
*/
void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd ) {
	int clientNum;
	sharedEntity_t *ent;
	mvabuf;

/*
	if(client->netchan.remoteAddress.type != NA_BOT && ((sv_pure->integer != 0 && client->pureAuthentic == 0) || !psvs.serverAuth))
		return;
*/
	Com_DPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", client->name );
	client->state = CS_ACTIVE;

	// set up the entity for the client
	clientNum = client - svs.clients;
	ent = SV_GentityNum( clientNum );
	ent->s.number = clientNum;
	client->gentity = ent;

	client->deltaMessage = -1;
	client->nextSnapshotTime = svs.time;    // generate a snapshot immediately

	if(cmd)
		client->lastUsercmd = *cmd;

	// call the game begin function
	ClientBegin( clientNum );

	// a bad cp command was sent, drop the client
	if ( client->netchan.remoteAddress.type != NA_BOT && sv_pure->integer != 0 && client->pureAuthentic == 2) {
		SV_DropClient( client, "EXE_UNPURECLIENTDETECTED" );
		return;
	}
	client->enteredWorldTime = svs.time;
	if(client->connectedTime == 0)
	{
		client->connectedTime = svs.time;
	}
	//Set gravity, speed... to system default
	Pmove_ExtendedInitForClient(client);

	if(sv_autodemorecord->boolean && !client->demorecording && (client->netchan.remoteAddress.type == NA_IP || client->netchan.remoteAddress.type == NA_IP6))
	{
		if(psvs.useuids){

			if(client->uid > 0)
				SV_RecordClient(client, va("demo_%i_", client->uid));
		}
		else
		{
			SV_RecordClient(client, va("demo_%s_", client->pbguid));
		}
	}

	if(!client->enteredWorldForFirstTime && (client->netchan.remoteAddress.type == NA_IP || client->netchan.remoteAddress.type == NA_IP6)){
		if(client->uid > 0)
		{
			SV_EnterLeaveLog("^5Client %s %s ^5entered this server in slot %d with uid %d", client->name, NET_AdrToString(&client->netchan.remoteAddress), clientNum, client->uid);
			client->power = Auth_GetClPowerByUID(client->uid);
		}else{
			SV_EnterLeaveLog("^5Client %s %s ^5entered this server in slot %d with guid %s", client->name, NET_AdrToString(&client->netchan.remoteAddress), clientNum, client->pbguid);
		}
	}

	client->enteredWorldForFirstTime = qtrue;
	client->pureAuthentic = 1;

	HL2Rcon_EventClientEnterWorld( clientNum );
	PHandler_Event(PLUGINS_ONCLIENTENTERWORLD, client);

}


sharedEntity_t* SV_AddBotClient(){

    int i, p, cntnames, read;
    unsigned short qport;
    client_t *cl = NULL;
    const char* denied;
    char name[16];
    char botnames[128][16];
    char userinfo[MAX_INFO_STRING];
    netadr_t botnet;
    usercmd_t ucmd;
    fileHandle_t file;
    mvabuf;

	//Find a free serverslot for our bot

	for ( i = sv_privateClients->integer; i < sv_maxclients->integer; i++) {
		cl = &svs.clients[i];
		if (cl->state == CS_FREE) {
			break;
		}
	}
	if( i == sv_maxclients->integer )
		return NULL;		//No free slot

        //Getting a new name for our bot
	FS_SV_FOpenFileRead("botnames.txt", &file);

	if(!file){
		cntnames = 0;
	}else{
		for(cntnames = 0; cntnames < 128; cntnames++){
			read = FS_ReadLine(botnames[cntnames], 16, file);
			if(read <= 0)
				break;
			if(strlen(botnames[cntnames]) < 2)
				break;
		}
		FS_FCloseFile(file);
	}
	if(!cntnames){
		Q_strncpyz(name,va("bot%d", i),sizeof(name));
	}else{
		Q_strncpyz(name,botnames[rand() % cntnames],sizeof(name));
		for(p = 0; p < sizeof(name); p++){
			if(name[p] == '\n' || name[p] == '\r')
				name[p] = 0;
		}
	}


//Connect our bot

	Com_RandomBytes((byte*) &qport, sizeof(short));

	*userinfo = 0;

	Info_SetValueForKey( userinfo, "cg_predictItems", "1");
	Info_SetValueForKey( userinfo, "color", "4");
	Info_SetValueForKey( userinfo, "head", "default");
	Info_SetValueForKey( userinfo, "model", "multi");
	Info_SetValueForKey( userinfo, "snaps", "20");
	Info_SetValueForKey( userinfo, "rate", "99999");
	Info_SetValueForKey( userinfo, "name", name);
	Info_SetValueForKey( userinfo, "protocol", va("%i", sv_protocol->integer));
	Info_SetValueForKey( userinfo, "qport", va("%hi", qport));

	Com_Memset(&botnet,0,sizeof(botnet));
	botnet.type = NA_BOT;
	Info_SetValueForKey( userinfo, "ip", NET_AdrToString( &botnet ) );

	//gotnewcl:
	Com_Memset(cl, 0x00, sizeof(client_t));

	cl->authentication = 1;
	cl->power = 0; //Sets the default power for the client
	// (build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized)
	Q_strncpyz(cl->pbguid,"BOT-Client",33);	// save the pbguid

	// save the userinfo
	Q_strncpyz(cl->userinfo, userinfo, 1024 );

	cl->unsentVoiceData = 0;
	cl->hasVoip = 0;
        cl->gentity = SV_GentityNum(i);
        cl->clscriptid = Scr_AllocArray();

	denied = ClientConnect(i, cl->clscriptid);
	if ( denied ) {
		Com_Printf("Bot couldn't connect: %s\n", denied);
		SV_FreeClientScriptId(cl);
		return NULL;
	}
	Com_Printf( "Going from CS_FREE to CS_CONNECTED for %s num %i\n", name, i);

	// save the addressgamestateMessageNum
	// init the netchan queue
	Netchan_Setup( NS_SERVER, &cl->netchan, botnet, qport,
			 cl->unsentBuffer, sizeof(cl->unsentBuffer),
			 cl->fragmentBuffer, sizeof(cl->fragmentBuffer));

/*	for(index = 0; index < MAX_RELIABLE_COMMANDS; index++ ){
		if(index < MAX_RELIABLE_COMMANDS / 2){
			cl->reliableCommands[index] = &cl->lowReliableCommands[index & (MAX_RELIABLE_COMMANDS - 1)];
		} else {
			cl->reliableCommands[index] = &svse.extclients[i].highReliableCommands[index & (MAX_RELIABLE_COMMANDS - 1)];
		}
	}
*/
	cl->state = CS_CONNECTED;
	cl->nextSnapshotTime = svs.time;
	cl->lastPacketTime = svs.time;
	cl->lastConnectTime = svs.time;

	SV_UserinfoChanged(cl);
	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	cl->gamestateMessageNum = -1; //newcl->gamestateMessageNum = -1;

	cl->canNotReliable = 1;
        //Let enter our new bot the game

//	SV_SendClientGameState(cl);

	Com_Memset(&ucmd, 0, sizeof(ucmd));

	SV_ClientEnterWorld(cl, &ucmd);

	return SV_GentityNum(i);
}


/*
============
SV_RemoveAllBots
============
*/


void SV_RemoveAllBots(){

	int i;
	client_t *cl;

	for(i=0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++){
		if(cl->netchan.remoteAddress.type == NA_BOT && cl->state > CS_FREE){
			SV_DropClient(cl, "EXE_DISCONNECTED");
		}
	}
}


/*
============
SV_RemoveBot
============
*/

sharedEntity_t* SV_RemoveBot() {

	int i;
	client_t *cl;

	for(i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++){
		if(cl->netchan.remoteAddress.type == NA_BOT && cl->state > CS_FREE){
			SV_DropClient(cl, "EXE_DISCONNECTED");
			return SV_GentityNum(i);
		}
	}
	return NULL;
}


char* SV_IsGUID(char* GUID){

	int j, k;

	if(strlen(GUID) == 8){
		k = 8;
	}else if(strlen(GUID) == 32){
		k = 32;
	}else{
		return NULL;
	}



        j = 0;
        while(j < k){
            if(GUID[j] < 0x30 || GUID[j] > 0x66 || (GUID[j] < 0x41 && GUID[j] > 0x39) || (GUID[j] < 0x61 && GUID[j] > 0x46)){
                return NULL;
            }
            j++;
        }
        Q_strlwr(GUID);
        if(k == 8)
            return GUID;
        else
            return &GUID[24];

}


int SV_GetUid(unsigned int clnum){

    if(clnum > 63)
        return -1;

    client_t *cl = &svs.clients[clnum];
    if(cl->uid < 1)
        return -1;

    return cl->uid;

}

void SV_SetUid(unsigned int clnum, unsigned int uid){

    if(clnum > 63)
        return;

    client_t *cl = &svs.clients[clnum];

    if(cl->state < CS_CONNECTED)
    {
        return;
    }
    cl->uid = uid;

}


/*
==================
SV_WWWRedirect

Send the client the full url of the http/ftp download server
==================
*/

void SV_WWWRedirect(client_t *cl, msg_t *msg){

    Com_sprintf(cl->wwwDownloadURL, sizeof(cl->wwwDownloadURL), "%s/%s", sv_wwwBaseURL->string, cl->downloadName);

    Com_Printf("Redirecting client '%s' to %s\n", cl->name, cl->wwwDownloadURL);

    cl->wwwDownloadStarted = qtrue;

    MSG_WriteByte(msg, svc_download);
    MSG_WriteLong(msg, -1);
    MSG_WriteString(msg, cl->wwwDownloadURL);
    MSG_WriteLong(msg, cl->downloadSize);
    MSG_WriteLong(msg, (int32_t)sv_wwwDlDisconnected->boolean);

    cl->wwwDl_var01 = qfalse;

    if(cl->download){
        FS_FCloseFile(cl->download);
    }

    cl->download = 0;
    *cl->downloadName = 0;
}


/*
==================
SV_WriteDownloadToClient

Check to see if the client wants a file, open it if needed and start pumping the client
Fill up msg with data
==================
*/

__cdecl void SV_WriteDownloadToClient( client_t *cl, msg_t *msg ) {
	int curindex;
	char errorMessage[1024];

	if ( !*cl->downloadName ) {
		return; // Nothing being downloaded
	}

	if ( cl->wwwDl_var02 ) {
		return;
	}

	if ( !cl->download ) {
		// We open the file here

		// DHM - Nerve
		// CVE-2006-2082
		// validate the download against the list of pak files
		if ( !FS_VerifyPak( cl->downloadName ) ) {
			// will drop the client and leave it hanging on the other side. good for him
			SV_DropClient( cl, "illegal download request" );
			return;
		}

		if ( !sv_allowDownload->integer || ( cl->downloadSize = FS_SV_FOpenFileRead( cl->downloadName, &cl->download ) ) <= 0 ) {
			// cannot auto-download file
			if ( !sv_allowDownload->integer ) {
				Com_Printf( "clientDownload: %d : \"%s\" download disabled", cl - svs.clients, cl->downloadName );

				if ( sv_pure->integer ) {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "EXE_AUTODL_SERVERDISABLED_PURE\x15%s", cl->downloadName );
				} else {
					Com_sprintf( errorMessage, sizeof( errorMessage ), "EXE_AUTODL_SERVERDISABLED\x15%s", cl->downloadName );
				}

			} else {
				Com_Printf( "clientDownload: %d : \"%s\" file not found on server\n", cl - svs.clients, cl->downloadName );
				Com_sprintf( errorMessage, sizeof( errorMessage ), "EXE_AUTODL_FILENOTONSERVER\x15%s", cl->downloadName );
			}
			MSG_WriteByte( msg, svc_download );
			MSG_WriteLong( msg, 0 ); // client is expecting block zero
			MSG_WriteLong( msg, -1 ); // illegal file size
			MSG_WriteString( msg, errorMessage );

			cl->wwwDl_var01 = 0;
			if(cl->download){
				FS_FCloseFile(cl->download);
			}
			cl->download = 0;
			*cl->downloadName = 0;
			return;
		}

		Com_Printf( "clientDownload: %d : begining \"%s\"\n", cl - svs.clients, cl->downloadName );

		if(sv_wwwDownload->boolean && cl->wwwDownload){
			if(cl->wwwDl_var03){
				cl->wwwDl_var03 = 0;
				return;
			}
			SV_WWWRedirect(cl, msg);
			return;
		}

		// Init
		cl->downloadCurrentBlock = cl->downloadClientBlock = cl->downloadXmitBlock = 0;
		cl->downloadCount = 0;
		cl->downloadEOF = qfalse;

		cl->wwwDownloadStarted = 0;
	}

	while ( cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW && cl->downloadSize != cl->downloadCount ) {

		curindex = ( cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW );

		// Perform any reads that we need to
		if ( !cl->downloadBlocks[curindex] ) {
			cl->downloadBlocks[curindex] = Z_Malloc( MAX_DOWNLOAD_BLKSIZE);
			if ( !cl->downloadBlocks[curindex]) {//Crash fix for download subsystem
				SV_DropClient(cl, "Failed to allocate a new chunk of memory for the serverdownloadsystem");
				return;
			}
		}

		cl->downloadBlockSize[curindex] = FS_Read( cl->downloadBlocks[curindex], MAX_DOWNLOAD_BLKSIZE, cl->download );

		if ( cl->downloadBlockSize[curindex] < 0 ) {
			// EOF right now
			cl->downloadCount = cl->downloadSize;
			break;
		}

		cl->downloadCount += cl->downloadBlockSize[curindex];

		// Load in next block
		cl->downloadCurrentBlock++;
	}

	// Check to see if we have eof condition and add the EOF block
	if (cl->downloadCount == cl->downloadSize && !cl->downloadEOF && cl->downloadCurrentBlock - cl->downloadClientBlock < MAX_DOWNLOAD_WINDOW ) {

		cl->downloadBlockSize[cl->downloadCurrentBlock % MAX_DOWNLOAD_WINDOW] = 0;
		cl->downloadCurrentBlock++;

		cl->downloadEOF = qtrue;  // We have added the EOF block
	}



	// Write out the next section of the file, if we have already reached our window,
	// automatically start retransmitting

	if ( cl->downloadClientBlock == cl->downloadCurrentBlock ) {
		return; // Nothing to transmit

	}
	if ( cl->downloadXmitBlock == cl->downloadCurrentBlock ) {
	// We have transmitted the complete window, should we start resending?

		//FIXME:  This uses a hardcoded one second timeout for lost blocks
		//the timeout should be based on client rate somehow
		if ( svs.time - cl->downloadSendTime > 1000 ) {
			cl->downloadXmitBlock = cl->downloadClientBlock;
		} else {
			return;
		}
	}

	// Send current block
	curindex = ( cl->downloadXmitBlock % MAX_DOWNLOAD_WINDOW );

	MSG_WriteByte( msg, svc_download );
	MSG_WriteLong( msg, cl->downloadXmitBlock );

	// block zero is special, contains file size
	if ( cl->downloadXmitBlock == 0 ) {
		MSG_WriteLong( msg, cl->downloadSize );
	}

	MSG_WriteShort( msg, cl->downloadBlockSize[curindex] );

	// Write the block
	if ( cl->downloadBlockSize[curindex] ) {
		if ( !cl->downloadBlocks[curindex]) {//Crash evaluation for download subsystem
			Com_PrintError("FATAL Server error in SV_WriteDownloadToClient.\nClient: %i, Name: %s, File: %s, CLDlBlock: %i, DlSize: %i, DlBlkSize: %i, DlSendTime: %i, ServerTime: %i, XmitBlock: %i, ClientState: %i, CurIndex: %i",
			cl - svs.clients, cl->name, cl->downloadName, cl->downloadClientBlock, cl->downloadSize, cl->downloadBlockSize[curindex], cl->downloadSendTime, svs.time, cl->downloadXmitBlock, cl->state, curindex);
			SV_DropClient(cl, "Fatal server error in Downloadsystem");
			return;
		}

		MSG_WriteData( msg, cl->downloadBlocks[curindex], cl->downloadBlockSize[curindex] );
	}

	Com_DPrintf( "clientDownload: %d : writing block %d\n", cl - svs.clients, cl->downloadXmitBlock );

	// Move on to the next block
	// It will get sent with next snap shot.  The rate will keep us in line.
	cl->downloadXmitBlock++;

	cl->downloadSendTime = svs.time;
}

/*
void SV_DlBurstFragments(){
    int i;
    client_t* cl;

//        int time;
//        int diff;
//        time = Sys_Milliseconds();


    for(cl = svs.clients, i = 0; i < sv_maxclients->integer; i++, cl++){

        if(*cl->downloadName && cl->download){
            while(SV_Netchan_TransmitNextFragment(cl));
        }
    }

//    diff = Sys_Milliseconds() - time;
//    Com_Printf("Took %i msec\n", diff);
}*/


/*
================
SV_SendClientGameState

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each new map load.

It will be resent if the client acknowledges a later message but has
the wrong gamestate.
================
*/
void SV_SendClientGameState( client_t *client ) {
	msg_t msg;
	byte msgBuffer[MAX_MSGLEN];

	while(client->state != CS_FREE && client->netchan.unsentFragments){
		SV_Netchan_TransmitNextFragment(client);
	}
#ifdef COD4X17A
	if(!client->canNotReliable){

		if(client->receivedstats != 127)
		{
			if(!client->receivedstats)
				NET_OutOfBandPrint(NS_SERVER, &client->netchan.remoteAddress, "requeststats\n");

			return;
		}

	}else{
		Com_Memset(client->stats, 0, sizeof(client->stats));
		client->receivedstats = 127;
	}
#endif


	SV_SetServerStaticHeader();

	if(client->state < CS_PRIMED)
	{
		serverStatus_Write();
	}

	Com_DPrintf( "SV_SendClientGameState() for %s\n", client->name );
	Com_DPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", client->name );
	client->state = CS_PRIMED;
	client->pureAuthentic = 0;

	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	client->gamestateMessageNum = client->netchan.outgoingSequence;

	MSG_Init( &msg, msgBuffer, sizeof( msgBuffer ) );

	MSG_ClearLastReferencedEntity(&msg);

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// send any server commands waiting to be sent first.
	// we have to do this cause we send the client->reliableSequence
	// with a gamestate and it sets the clc.serverCommandSequence at
	// the client side
	SV_UpdateServerCommandsToClient( client, &msg );

	// send the gamestate
	SV_WriteGameState(&msg, client);

	MSG_WriteLong( &msg, client - svs.clients );

	// write the checksum feed
	MSG_WriteLong( &msg, sv.checksumFeed );

	MSG_WriteByte(&msg, svc_EOF);

	// NERVE - SMF - debug info
	Com_DPrintf( "Sending %i bytes in gamestate to client: %i\n", msg.cursize, client - svs.clients );

	// deliver this to the client
	SV_SendMessageToClient( &msg, client );
	SV_GetServerStaticHeader();
}




/*
==================
SV_CloseDownload

clear/free any download vars
==================
*/
static void SV_CloseDownload( client_t *cl ) {
	int i;

	// EOF
	if ( cl->download ) {
		FS_FCloseFile( cl->download );
	}
	cl->download = 0;
	*cl->downloadName = 0;

	// Free the temporary buffer space
	for ( i = 0; i < MAX_DOWNLOAD_WINDOW; i++ ) {
		if ( cl->downloadBlocks[i] ) {
			Z_Free( cl->downloadBlocks[i] );
			cl->downloadBlocks[i] = NULL;
		}
	}
}



/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

/*
===================
SV_ExecuteClientMessage

Parse a client packet
===================
*/
__optimize3 __regparm2 void SV_ExecuteClientMessage( client_t *cl, msg_t *msg ) {
	int c, clnum;
	int serverId;
	static const char *clc_strings[256] = { "clc_move", "clc_moveNoDelta", "clc_clientCommand", "clc_EOF", "clc_nop"};


	msg_t decompressMsg;
	byte buffer[NETCHAN_FRAGMENTBUFFER_SIZE +1];

	MSG_Init(&decompressMsg, buffer, sizeof(buffer));

	decompressMsg.cursize = MSG_ReadBitsCompress(msg->data + msg->readcount, msg->cursize - msg->readcount, decompressMsg.data, decompressMsg.maxsize);
	if(decompressMsg.cursize == decompressMsg.maxsize)
	{
		SV_DropClient(cl, "SV_ExecuteClientMessage: Client sent oversize message");
		return;
	}
	
	clnum = cl - svs.clients;
	
	if ( sv_shownet->integer == clnum ) {
		Com_Printf( "------------------\n" );
	}
	
	serverId = cl->serverId;

	if ( serverId != sv_serverId && !cl->wwwDl_var01 && !cl->wwwDownloadStarted && !cl->wwwDl_var02 )
	{
		if((serverId & 0xf0) != (sv_serverId & 0xf0))
		{
			while(qtrue)
			{
				c = MSG_ReadBits(&decompressMsg, 3);
				
				if ( sv_shownet->integer == clnum )
				{
					if ( !clc_strings[c] ) {
						Com_Printf( "%3i:BAD CMD %i\n", decompressMsg.readcount - 1, c );
					} else {
						Com_Printf( "%3i:%s\n",  decompressMsg.readcount - 1, clc_strings[c] );
					}
				}
				
				if(c == clc_clientCommand)
				{
					if ( !SV_ClientCommand( cl, &decompressMsg, 1 ) || cl->state == CS_ZOMBIE)
						return; // we couldn't execute it because of the flood protection

				}else{

					break;
				}
			}

			if ( cl->messageAcknowledge > cl->gamestateMessageNum )
			{

				Com_DPrintf( "%s : dropped gamestate, resending\n", cl->name );
				SV_SendClientGameState( cl );
			}
			return;
		}else{
			if(cl->state == CS_PRIMED)
				SV_ClientEnterWorld(cl, 0);
		}
		return;
        }

	while(qtrue)
	{
		c = MSG_ReadBits(&decompressMsg, 3);
						
		if ( sv_shownet->integer == clnum )
		{
			if ( !clc_strings[c] ) {
					Com_Printf( "%3i:BAD CMD %i\n", decompressMsg.readcount - 1, c );
			} else {
				Com_Printf( "%3i:%s\n",  decompressMsg.readcount - 1, clc_strings[c] );
			}
		}
		if(c == clc_clientCommand){ //2
				if ( !SV_ClientCommand( cl, &decompressMsg, 0 ) || cl->state == CS_ZOMBIE ) {
					return; // we couldn't execute it because of the flood protection
				}

		}else{

			break;
		}
	}

	switch(c)
	{
		case clc_EOF:   //3
			return;

		case clc_move:  //0
			SV_UserMove( cl, &decompressMsg, qtrue );
			return;

		case clc_moveNoDelta:  //1
			SV_UserMove( cl, &decompressMsg, qfalse );
			return;

		default:
			Com_PrintWarning( "bad command byte %d for client %i\n", c, cl - svs.clients );
			return;
	}
}


/*
==================
SV_UpdateUserinfo_f
==================
*/
static void SV_UpdateUserinfo_f( client_t *cl ) {
	Q_strncpyz( cl->userinfo, SV_Cmd_Argv( 1 ), sizeof( cl->userinfo ) );

	SV_UserinfoChanged( cl );

	// call prog code to allow overrides
	ClientUserinfoChanged( cl - svs.clients );
}

/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately  FIXME: move to game?
=================
*/
static void SV_Disconnect_f( client_t *cl ) {
	SV_DropClient( cl, "EXE_DISCONNECTED" );
}


/*
=================
SV_VerifyPaks_f

If we are pure, disconnect the client if they do no meet the following conditions:

1. the first two checksums match our view of cgame and ui DLLs
   Wolf specific: the checksum is the checksum of the pk3 we found the DLL in
2. there are no any additional checksums that we do not have

This routine would be a bit simpler with a goto but i abstained

=================
*/
static void SV_VerifyPaks_f( client_t *cl ) {
/*	int nChkSum1, nClientPaks, nServerPaks, i, j, nCurArg;
	int nClientChkSum[1024];
	int nServerChkSum[1024];
*/
	// if we are pure, we "expect" the client to load certain things from
	// certain pk3 files, namely we want the client to have loaded the
	// ui and cgame that we think should be loaded based on the pure setting
	if ( sv_pure->boolean != 0 ) {
	
/*		nCurArg = 0;
		nClientPaks = SV_Cmd_Argc() - 3;

		if(nClientPaks < 0 || nClientPaks > 1024 || *SV_Cmd_Argv( 1 ) != '@')
		{
			cl->pureAuthentic = 2;
			return;
		}

		if(nClientPaks != 0)
		{
			for(i = 0; i <= nClientPaks; i++)
			{
				nClientChkSum[i] = atoi(SV_Cmd_Argv( i+2 ));
			}
			//Find duplicates
			for(i = 0; i < nClientPaks; i++)
			{
			    for(j = i+1; j < nClientPaks; j++)
			    {
				if(nClientChkSum[i] == nClientChkSum[j])
				{
					cl->pureAuthentic = 2;
					return;
				}
			    }
			}
		}else{
			cl->pureAuthentic = 0;
			return;
		}

		Cmd_TokenizeString(FS_LoadedIwdPureChecksums());

		nServerPaks = Cmd_Argc();
		nCurArg = -1;

		if(nServerPaks > 1024)
			nServerPaks = 1024;

		for(i = 0; i < nServerPaks; i++)
		{
			nServerChkSum[i] = atoi(Cmd_Argv( i ));
		}
		
		Cmd_EndTokenizedString();

		// check if the number of checksums was correct
		nChkSum1 = sv.checksumFeed;
		for ( i = 0; i < nClientPaks; i++ ) {
			nChkSum1 ^= nClientChkSum[i];
		}
		nChkSum1 ^= nClientPaks;
		if ( nChkSum1 == nClientChkSum[nClientPaks] ) {*/
			cl->pureAuthentic = 1;
/*		}else{
			cl->pureAuthentic = 2;
		}*/
	}
}

/*
=================
SV_ResetPureClient_f
=================
*/
static void SV_ResetPureClient_f( client_t *cl ) {
	cl->pureAuthentic = 0;
}


/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f( client_t *cl ) {

	// Kill any existing download
	SV_CloseDownload( cl );

	// cl->downloadName is non-zero now, SV_WriteDownloadToClient will see this and open
	// the file itself
	cl->wwwDl_var01 = 1;
	Q_strncpyz( cl->downloadName, SV_Cmd_Argv( 1 ), sizeof( cl->downloadName ) );
}


/*
==================
SV_StopDownload_f

Abort a download if in progress
==================
*/
void SV_StopDownload_f( client_t *cl ) {
	if ( *cl->downloadName ) {
		Com_DPrintf( "clientDownload: %d : file \"%s\" aborted\n", cl - svs.clients, cl->downloadName );
	}

	SV_CloseDownload( cl );
	cl->wwwDl_var01 = 0;
}

/*
==================
SV_DoneDownload_f

Downloads are finished
==================
*/
void SV_DoneDownload_f( client_t *cl ) {
	Com_DPrintf( "clientDownload: %s Done\n", cl->name );
	// resend the game state to update any clients that entered during the download
	SV_CloseDownload( cl );
	cl->wwwDl_var01 = 0;
	SV_SendClientGameState( cl );
}

/*
==================
SV_NextDownload_f

The argument will be the last acknowledged block from the client, it should be
the same as cl->downloadClientBlock
==================
*/
void SV_NextDownload_f( client_t *cl ) {

	int block = atoi( SV_Cmd_Argv( 1 ) );

	if ( block == cl->downloadClientBlock ) {
		Com_DPrintf( "clientDownload: %d : client acknowledge of block %d\n", cl - svs.clients, block );

		// Find out if we are done.  A zero-length block indicates EOF
		if ( cl->downloadBlockSize[cl->downloadClientBlock % MAX_DOWNLOAD_WINDOW] == 0 ) {
			Com_Printf( "clientDownload: %d : file \"%s\" completed\n", cl - svs.clients, cl->downloadName );
			SV_CloseDownload( cl );
			return;
		}

		cl->downloadSendTime = svs.time;
		cl->downloadClientBlock++;
		return;
	}
	// We aren't getting an acknowledge for the correct block, drop the client
	// FIXME: this is bad... the client will never parse the disconnect message
	//			because the cgame isn't loaded yet
	SV_DropClient( cl, "broken download" );
}


/*
==================
SV_RetransmitDownload_f

The argument will be the last acknowledged block from the client, it should be
the same as cl->downloadClientBlock
==================
*/
void SV_RetransmitDownload_f( client_t *cl ) {

	int block = atoi( SV_Cmd_Argv( 1 ) );

	if ( block == cl->downloadClientBlock ) {
		cl->downloadXmitBlock = block;
	}
}

/*
==================
SV_WWWDownload_f

==================
*/
void SV_WWWDownload_f( client_t *cl ) {

	char* download = SV_Cmd_Argv( 1 );

	if(!cl->wwwDownloadStarted)
	{
		Com_PrintWarning("SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", download, cl->name);
		SV_DropClient(cl, "Unexpected www download message.");
		return;
	}
	if(!Q_stricmp(download, "ack"))
	{
	    if(cl->wwwDl_var02)
		Com_PrintWarning("Duplicated wwwdl ack from client: '%s'\n", cl->name);
	
	    cl->wwwDl_var02 = qtrue;
	
	}else if(!Q_stricmp(download, "bbl8r")){
	
		SV_DropClient(cl, "Client dropped to download files");
	
	}else if(!cl->wwwDl_var02){

		Com_PrintWarning("SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", download, cl->name);
		SV_DropClient(cl, "Unexpected www download message.");

	}else if(!Q_stricmp(download, "done")){
	
		cl->wwwDl_var01 = 0;
		if ( cl->download ) {
			FS_FCloseFile( cl->download );
		}
		cl->download = 0;
		*cl->downloadName = 0;
		cl->wwwDownloadStarted = 0;
		cl->wwwDl_var02 = 0;

	}else if(!Q_stricmp(download, "fail")){

		Com_PrintWarning("Client '%s' reported that the http download of '%s' failed, falling back to a server download\n", cl->name, cl->downloadName);

		cl->wwwDl_var01 = 0;
		if ( cl->download ) {
			FS_FCloseFile( cl->download );
		}
		cl->download = 0;
		*cl->downloadName = 0;
		cl->wwwDownloadStarted = 0;
		cl->wwwDl_var02 = 0;
		cl->wwwDl_var03 = 1;

		SV_SendClientGameState(cl);

	}else if(!Q_stricmp(download, "chkfail")){

		Com_PrintWarning("Client '%s' reports that the redirect download for '%s' had wrong checksum.\n        You should make sure that your files on your redirect are the same files you have on your server\n", cl->name, cl->downloadName);
		
		cl->wwwDl_var01 = 0;
		if ( cl->download ) {
			FS_FCloseFile( cl->download );
		}
		cl->download = 0;
		*cl->downloadName = 0;
		cl->wwwDownloadStarted = 0;
		cl->wwwDl_var02 = 0;
		cl->wwwDl_var03 = 1;

		SV_SendClientGameState(cl);

	}else{
		Com_PrintWarning("SV_WWWDownload: Unknown wwwdl subcommand '%s' for client '%s'\n", download, cl->name);
		SV_DropClient(cl, "Unexpected www download message.");
	}
}

void SV_MutePlayer_f(client_t* cl){

	int muteClient = atoi( SV_Cmd_Argv( 1 ) );

	if(muteClient > 63 || muteClient < 0)
		return;
	
	cl->mutedClients[muteClient] = 1;
}


void SV_UnmutePlayer_f(client_t* cl){

	int muteClient = atoi( SV_Cmd_Argv( 1 ) );

	if(muteClient > 63 || muteClient < 0)
		return;
	
	cl->mutedClients[muteClient] = 0;
}

#ifndef COD4X17A

#define NUM_STATS_PARTS 16
#define NUM_STATS_MASK (NUM_STATS_PARTS -1)
#define STATS_PART_SIZE (sizeof(cl->stats) / 16)

void SV_Stats_f(client_t* cl)
{
	msg_t msg;
	byte buf[MAX_STRING_CHARS];

	MSG_Init(&msg, buf, sizeof(buf));

	MSG_WriteString(&msg, SV_Cmd_Argv(1));

	Com_Printf("Received packet %i of stats data\n", cl->receivedstats & NUM_STATS_MASK);

	MSG_ReadBase64(&msg, &cl->stats[STATS_PART_SIZE * (cl->receivedstats & NUM_STATS_MASK)], STATS_PART_SIZE);

	++cl->receivedstats;
}
#endif

typedef struct {
	char    *name;
	void ( *func )( client_t *cl );
	qboolean indlcmd;
} ucmd_t;

static ucmd_t ucmds[] = {
	{"userinfo", SV_UpdateUserinfo_f, 0},
	{"disconnect", SV_Disconnect_f, 1},
	{"cp", SV_VerifyPaks_f, 0},
	{"vdr", SV_ResetPureClient_f, 0},
	{"download", SV_BeginDownload_f, 0},
	{"nextdl", SV_NextDownload_f, 0},
	{"stopdl", SV_StopDownload_f, 0},
	{"donedl", SV_DoneDownload_f, 0},
	{"retransdl", SV_RetransmitDownload_f, 0},
	{"wwwdl", SV_WWWDownload_f, 0},
	{"muteplayer", SV_MutePlayer_f, 0},
	{"unmuteplayer", SV_UnmutePlayer_f, 0},
#ifndef COD4X17A
	{"stats", SV_Stats_f, 0},
#endif
	{NULL, NULL, 0}
};

/*
==================
SV_ExecuteClientCommand

Also called by bot code
==================
*/
void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK, qboolean inDl ) {
	ucmd_t  *u;

	SV_Cmd_TokenizeString( s );

	if(SV_Cmd_Argc() < 1){
		SV_Cmd_EndTokenizedString( );
		return;
	}

	// see if it is a server level command
	for ( u = ucmds ; u->name ; u++ ) {
		if ( !strcmp( SV_Cmd_Argv( 0 ), u->name ) ) {

			if(!inDl || u->indlcmd){
				u->func( cl );
			}
			SV_Cmd_EndTokenizedString( );
			return;
		}
	}

	if ( clientOK ) {
		// pass unknown strings to the game
		if ( !u->name && sv.state == SS_GAME ) {
			ClientCommand( cl - svs.clients );
		}
	}

	SV_Cmd_EndTokenizedString( );
}


/*
===============
SV_ClientCommand
===============
*/
qboolean SV_ClientCommand( client_t *cl, msg_t *msg, qboolean inDl) {
	int seq;
	const char  *s;
	qboolean clientOk = qtrue;
	qboolean floodprotect = qtrue;
	char stringbuf[MAX_STRING_CHARS];

	seq = MSG_ReadLong( msg );
	s = MSG_ReadString( msg, stringbuf, sizeof(stringbuf));

	// see if we have already executed it
	if ( cl->lastClientCommand >= seq ) {
		return qtrue;
	}

	Com_DPrintf( "clientCommand: %s : %i : %s\n", cl->name, seq, s );

	// drop the connection if we have somehow lost commands
	if ( seq > cl->lastClientCommand + 1 ) {
		Com_Printf( "Client %s lost %i clientCommands\n", cl->name, seq - cl->lastClientCommand + 1 );
		SV_DropClient( cl, "EXE_LOSTRELIABLECOMMANDS" );
		return qfalse;
	}

	if(!sv_floodProtect->boolean || cl->state == CS_PRIMED || cl->netchan.remoteAddress.type == NA_LOOPBACK
		// NERVE - SMF - some server game-only commands we cannot have flood protect
		|| !Q_strncmp( "team", s, 4 ) || !Q_strncmp( "mr", s, 2 ) || !Q_strncmp( "score", s, 5 )){
		//Com_DPrintf( "Skipping flood protection for: %s\n", s );
		floodprotect = qfalse;
	}

	// malicious users may try using too many string commands
	// to lag other players.  If we decide that we want to stall
	// the command, we will stop processing the rest of the packet,
	// including the usercmd.  This causes flooders to lag themselves
	// but not other people
	// We don't do this when the client hasn't been active yet since its
	// normal to spam a lot of commands when downloading

	if(inDl){
		Com_DPrintf( "client text ignored for %s: %s\n", cl->name, SV_Cmd_Argv( 0 ) );
		clientOk = qfalse;
	}


	if(floodprotect){

		if(!inDl && !cl->floodprotect ){

			if(svs.time < cl->nextReliableTime){
				Com_DPrintf( "client text ignored for %s: %s\n", cl->name, SV_Cmd_Argv( 0 ) );
				clientOk = qfalse;
			}
		}

		if(svs.time > cl->nextReliableTime){

			cl->floodprotect = sv_floodProtect->integer - 1;
		}else{
			cl->floodprotect = cl->floodprotect - 1;

			if(cl->floodprotect < 0)
				cl->floodprotect = 0;
		}

		cl->nextReliableTime = svs.time + 800;
	}

	SV_ExecuteClientCommand( cl, s, clientOk, inDl );

	cl->lastClientCommand = seq;
	Q_strncpyz( cl->lastClientCommandString, s, sizeof( cl->lastClientCommandString ) );

	return qtrue;       // continue procesing
}

const char* SV_GetGuid( unsigned int clnum)
{
	if(clnum > sv_maxclients->integer)
		return "";

	return svs.clients[clnum].pbguid;
}

void SV_DelayDropClient(client_t *client, const char *dropmsg)
{
	if ( client->state != CS_ZOMBIE && client->delayDropMsg == NULL)
	{
		client->delayDropMsg = dropmsg;
	}
}


void SV_WriteClientVoiceData(msg_t *msg, client_t *client)
{	
	int i;
	
	MSG_WriteByte(msg, client->unsentVoiceData);
	
	for(i = 0; i < client->unsentVoiceData; i++)
	{
		MSG_WriteByte( msg, client->voicedata[i].num );
		MSG_WriteByte( msg, client->voicedata[i].dataLen );
		MSG_WriteData( msg, client->voicedata[i].data, client->voicedata[i].dataLen );
	}
	
}

void SV_SendClientVoiceData(client_t *client)
{
	
	msg_t msg;
	byte buff[0x20000];
	
	if ( client->state < CS_ACTIVE || client->unsentVoiceData == 0)
	{
		return;
	}
    MSG_Init(&msg, buff, sizeof(buff));
    MSG_WriteString(&msg, "v");
    SV_WriteClientVoiceData(&msg, client);
    if ( msg.overflowed )
    {
		Com_PrintWarning( "WARNING: voice msg overflowed for %s\n", client->shortname);
		return;
    }
    NET_OutOfBandData(NS_SERVER, &client->netchan.remoteAddress, msg.data, msg.cursize);
    client->unsentVoiceData = 0;
}

void SV_GetVoicePacket(netadr_t *from, msg_t *msg)
{
	unsigned short qport;
	client_t *cl;
	
	qport = (unsigned short)MSG_ReadShort(msg);

	cl = SV_ReadPackets(from, qport);
	
	if ( cl && cl->state >= CS_CONNECTED)
	{
		cl->lastPacketTime = svs.time;
		if ( cl->state >= CS_ACTIVE )
			SV_UserVoice(cl, msg);
		else
			SV_PreGameUserVoice(cl, msg);

	}
}

client_t* SV_ReadPackets(netadr_t *from, unsigned short qport)
{
	int i;
	client_t *cl;

	// find which client the message is from
	for (i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++)
	{
		
		if ( cl->state == CS_FREE ) {
			continue;
		}
		if ( !NET_CompareBaseAdr( from, &cl->netchan.remoteAddress ) ) {
			continue;
		}
		
		// it is possible to have multiple clients from a single IP
		// address, so they are differentiated by the qport variable
		if ( cl->netchan.qport != qport ) {
			continue;
		}
		
		// the IP port can't be used to differentiate them, because
		// some address translating routers periodically change UDP
		// port assignments
		if ( cl->netchan.remoteAddress.port != from->port ) {
			Com_Printf( "SV_ReceiveStats: fixing up a translated port\n" );
			cl->netchan.remoteAddress.port = from->port;
		}
		return cl;
	
	}
	return NULL;
}
