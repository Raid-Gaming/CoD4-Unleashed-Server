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
#include "netchan.h"
#include "server.h"

#include <string.h>
/*
fileHandle_t commanddump;


void SV_DumpCommands( client_t *client, byte *cmd, int size, qboolean cl_to_servercmd) {

	if(!com_developer || com_developer->integer < 2)
		return;

	msg_t msg;
	byte buffer[20400];

	MSG_Init(&msg, buffer, sizeof(buffer));

	if(cl_to_servercmd){
		Com_sprintf((char*)buffer, sizeof(buffer), "\nDir: Cl -> Sv, Cl: %i, T: %i, NA: %i, Len %i, Msg: ", 
			client - svs.clients, svs.time ,client->reliableSequence - client->reliableAcknowledge, size);
        }else{
		Com_sprintf((char*)buffer, sizeof(buffer), "\nDir: Sv -> Cl, Cl: %i, T: %i, NA: %i, Len %i, Msg: ", 
			client - svs.clients, svs.time ,client->reliableSequence - client->reliableAcknowledge, size);
        }

	int lenbeginstr = strlen((char*)buffer);

	MSG_ReadBitsCompress(cmd, &buffer[lenbeginstr], size);

        buffer[size+lenbeginstr-1] = '\n';

	Sys_EnterCriticalSection(5);

	if ( com_logfile && com_logfile->integer ) {
    // TTimo: only open the qconsole.log if the filesystem is in an initialized state
    //   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
	    if ( !commanddump && FS_Initialized()) {
			struct tm *newtime;
			time_t aclock;

			time( &aclock );
			newtime = localtime( &aclock );

			commanddump = FS_FOpenFileWrite( "reliableupdateCmds.log" );

			if ( com_logfile->integer > 1 && commanddump) {
				// force it to not buffer so we get valid
				// data even if we are crashing
				FS_ForceFlush(commanddump);
			}
			if ( commanddump ) FS_Write(va("\nLogfile opened on %s\n", asctime( newtime )), strlen(va("\nLogfile opened on %s\n", asctime( newtime ))), commanddump);
	    }
	    if (commanddump && FS_Initialized()) {

		FS_Write("\nUncompressed: ", 15, commanddump);
		FS_Write(buffer, 300, commanddump);
		FS_Write("\nCompressed: ", 13, commanddump);
		FS_Write(cmd, size, commanddump);
	    }
	}
	Sys_LeaveCriticalSection(5);
}


*/
/*
==============
SV_Netchan_Decode

	// first 12 bytes of the data are always:
	long serverId;
	long messageAcknowledge;
	long reliableAcknowledge;

==============
*/
void SV_Netchan_Decode( client_t *client, byte *data, int remaining ) {
	int i, index;
	byte key, *string;
//	extclient_t *extcl = &svs.extclients[ client - svs.clients ];
	
//	string = (byte *)extcl->reliableCommands[ client->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ].command;
	string = (byte *)client->reliableCommands[ client->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 ) ].command;

	if(!remaining) return;
	key = client->challenge ^ client->serverId ^ client->messageAcknowledge;

	for ( i=0, index=0; i < remaining; i++ ) {

		if ( !string[index] ) {
			index = 0;
		}

		// modify the key with the last sent and acknowledged server command
		key ^= string[index] << ( i & 1 );
		data[i] ^= key;

		index++;
	}
}

/*
==============
SV_Netchan_Encode

	// first four bytes of the data are always:
	long reliableAcknowledge;

==============
*/

void SV_Netchan_Encode( client_t *client, byte *data, int cursize ) {

	int i, index;
	byte key, *string;

	string = (byte *)client->lastClientCommandString;
	key = client->challenge ^ client->netchan.outgoingSequence;

	for ( i = 0, index = 0; i < cursize - 4; i++ ) {

		if ( !string[index] ) {
			index = 0;
		}

		// modify the key with the last sent and acknowledged server command
		key ^= string[index] << ( i & 1 );
		data[i+4] ^= key;

		index++;
	}
}


/*
===============
SV_Netchan_Transmit

TTimo
show_bug.cgi?id=462
if there are some unsent fragments (which may happen if the snapshots
and the gamestate are fragmenting, and collide on send for instance)
then buffer them and make sure they get sent in correct order
================
*/
qboolean SV_Netchan_Transmit( client_t *client, byte *data, int cursize) {   //int length, const byte *data ) {
	qboolean nt_ret;
	int i;

//	SV_DumpCommands(client, (byte*)data, cursize, qfalse);

	if(cursize - 4){
		SV_Netchan_Encode(client, data, cursize);
	}
	nt_ret = Netchan_Transmit( &client->netchan, cursize, data );
	if(client->netchan.unsentFragments){
		return nt_ret;
	}

	i = client->netchan.outgoingSequence & 31;

	Com_Memset(&client->frames[i], 0, sizeof(clientSnapshot_t));

	client->frames[i].first_entity = svs.nextSnapshotEntities;
	client->frames[i].first_client = svs.nextSnapshotClients;

	return nt_ret;
}


/*
===============
SV_Netchan_TransmitNextFragment

TTimo
show_bug.cgi?id=462
if there are some unsent fragments (which may happen if the snapshots
and the gamestate are fragmenting, and collide on send for instance)
then buffer them and make sure they get sent in correct order
================
*/
__cdecl qboolean SV_Netchan_TransmitNextFragment( client_t *client ) {   //int length, const byte *data ) {
	int i;

	Netchan_TransmitNextFragment( &client->netchan );

	if(client->netchan.unsentFragments)
		return qtrue; //Return true if we have still unsent fragments
	
	i = client->netchan.outgoingSequence & 31;

	Com_Memset(&client->frames[i], 0, sizeof(clientSnapshot_t));

	client->frames[i].first_entity = svs.nextSnapshotEntities;
	client->frames[i].first_client = svs.nextSnapshotClients;

	return qfalse; //No more unsent fragments, this was the last fragment
}