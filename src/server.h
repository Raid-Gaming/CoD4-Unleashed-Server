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



#ifndef __SERVER_H__
#define __SERVER_H__

#include "q_shared.h"
#include "qcommon.h"
#include "sys_net.h"
#include "netchan.h"
#include "entity.h"
#include "player.h"
#include "filesystem.h"
#include "g_hud.h"
#include "sys_cod4defs.h"
#include "cvar.h"
#include "net_game_conf.h"

#ifndef COD4X17A
#include "net_reliabletransport.h"
#endif

#include <time.h>

#define SERVER_STRUCT_ADDR 0x13e78d00
#define sv (*((server_t*)(SERVER_STRUCT_ADDR)))

#define SERVERSTATIC_STRUCT_ADDR 0x8c51780
#define svs (*((serverStatic_t*)(SERVERSTATIC_STRUCT_ADDR)))

#define SERVERHEADER_STRUCT_ADDR 0x13f18f80
#define svsHeader (*((svsHeader_t*)(SERVERHEADER_STRUCT_ADDR)))

#define SERVERID_ADDR 0x8c51720
#define sv_serverId (*(int*)(SERVERID_ADDR))



// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024
// Allow a certain amount of challenges to have the same IP address
// to make it a bit harder to DOS one single IP address from connecting
// while not allowing a single ip to grab all challenge resources
#define MAX_CHALLENGES_MULTI (MAX_CHALLENGES / 2)

#define CLIENT_BASE_ADDR 0x90b4f8C


typedef enum {
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse
				// connection for a couple seconds
	CS_CONNECTED,		// has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,		// gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE		// client is fully in game
}clientState_t;


//*******************************************************************************

typedef struct {//(0x2146c);
	playerState_t	ps;			//0x2146c
	int		num_entities;
	int		num_clients;		// (0x2f68)
	int		first_entity;		// (0x2f6c)into the circular sv_packet_entities[]
	int		first_client;
							// the entities MUST be in increasing state number
							// order, otherwise the delta compression will fail
	unsigned int	messageSent;		// (0x243e0 | 0x2f74) time the message was transmitted
	unsigned int	messageAcked;		// (0x243e4 | 0x2f78) time the message was acked
	int		messageSize;		// (0x243e8 | 0x2f7c)   used to rate drop packets
	int		var_03;
} clientSnapshot_t;//size: 0x2f84

struct	sharedEntity_t;


typedef enum {
    UN_VERIFYNAME,
    UN_NEEDUID,
    UN_OK
}username_t;
#pragma pack(1)


typedef struct
{
	char num;
	char data[256];
	int dataLen;
}voices_t;

typedef struct client_s {//90b4f8c
	clientState_t		state;
	int			unksnapshotvar;		// must timeout a few frames in a row so debugging doesn't break
	int			deltaMessage;		// (0x8) frame last client usercmd message
	qboolean		rateDelayed;		// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	netchan_t		netchan;	//(0x10)
	//DemoData
	fileHandleData_t	demofile;
	qboolean		demorecording;
	qboolean		demowaiting;
	char			demoName[MAX_QPATH];
	int			demoArchiveIndex;
	int			demoMaxDeltaFrames;
	int			demoDeltaFrameCount;

	int			authentication;
	qboolean		playerauthorized;
	qboolean		noPb;
	username_t		usernamechanged;
	int			bantime;
	int			clienttimeout;
	int			uid;
	char			OS;
	int			power;
	char			originguid[33];
	qboolean		firstSpawn;
	game_hudelem_t		*hudMsg;
	int			msgType;
	unsigned int		currentAd;
	int			enteredWorldTime;
	byte			entityNotSolid[MAX_GENTITIES / 8];//One bit for entity
	byte			entityInvisible[MAX_GENTITIES / 8];//One bit for entity
	unsigned int		clFrames;
	unsigned int		clFrameCalcTime;
	unsigned int		clFPS;
	float			jumpHeight;
	int			gravity;
	int			playerMoveSpeed;
	qboolean		needPassword;
	qboolean		needPasswordNotified;
	char			loginname[32];
	//Free Space
	qboolean		enteredWorldForFirstTime;
	unsigned int		connectedTime;
	char			xversion[8];
	int			protocol;
	qboolean		needupdate;
	qboolean		updateconnOK;
#ifdef COD4X17A
	byte			dummy767[110];
#else
	netreliablemsg_t relmsg;
#endif	
	byte			free[518];

	char			name[64];

	int			unknownUsercmd1;	//0x63c
	int			unknownUsercmd2;	//0x640
	int			unknownUsercmd3;	//0x644
	int			unknownUsercmd4;	//0x648

	const char*		delayDropMsg;		//0x64c
	char			userinfo[MAX_INFO_STRING];		// name, etc (0x650)
	reliableCommands_t	reliableCommands[MAX_RELIABLE_COMMANDS];	// (0xa50)
	int			reliableSequence;	// (0x20e50)last added reliable message, not necesarily sent or acknowledged yet
	int			reliableAcknowledge;	// (0x20e54)last acknowledged reliable message
	int			reliableSent;		// last sent reliable message, not necesarily acknowledged yet
	int			messageAcknowledge;	// (0x20e5c)
	int			gamestateMessageNum;	// (0x20e60) netchan->outgoingSequence of gamestate
	int			challenge; //0x20e64
//Unknown where the offset error is
	usercmd_t		lastUsercmd;		//(0x20e68)
	int			lastClientCommand;	//(0x20e88) reliable client message sequence
	char			lastClientCommandString[MAX_STRING_CHARS]; //(0x20e8c)
	sharedEntity_t		*gentity;		//(0x2128c)

	char			shortname[MAX_NAME_LENGTH];	//(0x21290) extracted from userinfo, high bits masked
	int			wwwDl_var01;
	// downloading
	char			downloadName[MAX_QPATH]; //(0x212a4) if not empty string, we are downloading
	fileHandle_t		download;		//(0x212e4) file being downloaded
 	int			downloadSize;		//(0x212e8) total bytes (can't use EOF because of paks)
 	int			downloadCount;		//(0x212ec) bytes sent
	int			downloadClientBlock;	//(0x212f0) last block we sent to the client, awaiting ack
	int			downloadCurrentBlock;	//(0x212f4) current block number
	int			downloadXmitBlock;	//(0x212f8) last block we xmited
	unsigned char		*downloadBlocks[MAX_DOWNLOAD_WINDOW];	//(0x212fc) the buffers for the download blocks
	int			downloadBlockSize[MAX_DOWNLOAD_WINDOW];	//(0x2131c)
	qboolean		downloadEOF;		//(0x2133c) We have sent the EOF block
	int			downloadSendTime;	//(0x21340) time we last got an ack from the client
	char			wwwDownloadURL[MAX_OSPATH]; //(0x21344) URL from where the client should download the current file

	qboolean		wwwDownload;		// (0x21444)
	qboolean		wwwDownloadStarted;	// (0x21448)
	qboolean		wwwDl_var02;		// (0x2144c)
	qboolean		wwwDl_var03;
	int			nextReliableTime;	// (0x21454) svs.time when another reliable command will be allowed
	int			floodprotect;		// (0x21458)
	int			lastPacketTime;		// (0x2145c)svs.time when packet was last received
	int			lastConnectTime;	// (0x21460)svs.time when connection started
	int			nextSnapshotTime;	// (0x21464) send another snapshot when svs.time >= nextSnapshotTime
	int			timeoutCount;
	clientSnapshot_t	frames[PACKET_BACKUP];	// (0x2146c) updates can be delta'd from here
	int			ping;		//(0x804ec)
	int			rate;		//(0x804f0)		// bytes / second
	int			snapshotMsec;	//(0x804f4)	// requests a snapshot every snapshotMsec unless rate choked
	int			unknown6;
	int			pureAuthentic; 	//(0x804fc)
	byte			unsentBuffer[NETCHAN_UNSENTBUFFER_SIZE]; //(0x80500)
	byte			fragmentBuffer[NETCHAN_FRAGMENTBUFFER_SIZE]; //(0xa0500)
	char			pbguid[33]; //0xa0d00
	byte			pad;
	short			clscriptid; //0xa0d22
	int			canNotReliable; 
	int			serverId; //0xa0d28
	voices_t		voicedata[40];
	int			unsentVoiceData;//(0xa35f4)
	byte			mutedClients[MAX_CLIENTS];
	byte			hasVoip;//(0xa3638)
	byte			stats[8192];		//(0xa3639)
	byte			receivedstats;		//(0xa5639)
	byte			dummy1;
	byte			dummy2;
} client_t;//0x0a563c


typedef struct {
	netadr_t		adr;
	int			challenge;
	int			clientChallenge;
	int			time;				// time the last packet was sent to the autherize server
	int			pingTime;			// time the challenge response was sent to client
	int			firstTime;			// time the adr was first used, for authorize timeout checks
	char			pbguid[33];
	qboolean		connected;
	int			ipAuthorize;
} challenge_t;


#define	MAX_STREAM_SERVERS	6
#define	MAX_MASTER_SERVERS	8	// max recipients for heartbeat packets
// this structure will be cleared only when the game dll changes


typedef struct{
int	challengeslot;
int	firsttime;
int	lasttime;
int	attempts;
}connectqueue_t;	//For fair queuing players who wait for an empty slot

#define MAX_TRANSCMDS 32
typedef struct{
	char cmdname[32];
	char cmdargument[1024];
}translatedCmds_t;

/*

Some Info:
svs.nextSnapshotEntities 0x13f18f94
svs.numSnapshotEntites 0x13f18f8c
svc_snapshot = 6;
svs.snapflagServerbit 0x13f18f88  //copied from real svs. to something else

*/

#pragma pack(push, 1)

typedef struct
{
	int buffer;
	int msgsize;

}svArchiveBuf_t;

typedef struct clientState_s
{
	int number;
	byte b[0x60];
}clientState_ts;

typedef struct
{
	entityState_t ent;
	int field_F4;
	int field_F8;
	int field_FC;
	int field_100;
	int field_104;
	int field_108;
	int field_10C;
	int field_110;
	int field_114;
}archivedEntity_t;

typedef struct
{
	int num;
	int field_4;
	int field_8;
	int field_C;
	int field_10;
	int field_14;
	int field_18;
}entityUnknownStr_t;

typedef struct
{
	playerState_t ps;
	int field_2F64;
	int field_2F68;
	int field_2F6C;
	int field_2F70;
	int field_2F74;
	int field_2F78;
	int field_2F7C;
	int field_2F80;
	int field_2F84;
	int field_2F88;
	int field_2F8C;
	int field_2F90;
	int field_2F94;
	int field_2F98;
	int field_2F9C;
	int field_2FA0;
	int field_2FA4;
	int field_2FA8;
	int field_2FAC;
	int field_2FB0;
	int field_2FB4;
	int field_2FB8;
	int field_2FBC;
	int field_2FC0;
	int field_2FC4;
	int field_2FC8;
}entityUnknownStr2_t;

typedef struct
{
	int adr[5];
	int challenge;
	int time;
	int pingTime;
	int firstTime;
	int firstPing;
	qboolean connected;
	char pbguid[33];
	char field_4D;
	char field_4E;
	char field_4F;
}challenge2_t;

typedef struct{
	int time;
	char guid[32];
}banlist_t;

typedef struct {//0x8c51780

	entityUnknownStr_t entUnknown1[512];
	archivedEntity_t archivedEntities[16384];
	
	qboolean	initialized;				//0x90b4f80 sv_init has completed

	int		time;					// will be strictly increasing across level changes

	int		snapFlagServerBit;			// ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

	client_t	clients[MAX_CLIENTS];				// [sv_maxclients->integer];



	int		numSnapshotEntities;		//0xba0de8c sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int		numSnapshotClients;
	int		nextSnapshotEntities;		//0xba0de94 next snapshotEntities to use
	int		nextSnapshotClients;
	
	entityState_t snapshotEntities[0x2A000];
	clientState_ts snapshotClients[0x20000];

	int nextArchivedSnapshotFrames; //0xee95e9c
	
	svArchiveBuf_t archiveSnaps[1200];
	byte archiveSnapBuffer[0x2000000];
	int nextArchivedSnapshotBuffer;
	int nextCachedSnapshotEntities; //0x10e98420
	int nextCachedSnapshotClients;
	int nextCachedSnapshotFrames;
	entityUnknownStr2_t entUnknown2[4096];
	
	int nextHeartbeatTime;
	
	int field_4;
	
	challenge2_t challenges[MAX_CHALLENGES];
	
	int redirectAddress[5];
	int authorizeAddress[5];
	
	char netProfilingBuf[1504];
	
	banlist_t banlist[16];
	
	int field_14850;
	
	vec3_t mapCenter;
	
	char field_14860[112];
	
}serverStatic_t; //Size: 0xb227580


#pragma pack(pop)

typedef struct {
	unsigned long long	nextHeartbeatTime;
#ifdef COD4X17A
	challenge_t		challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting
#endif
	netadr_t		redirectAddress;			// for rcon return messages
	netadr_t		authorizeAddress;			// ??? for rcon return messages
	client_t		*redirectClient;		//used for SV_ExecuteRemoteControlCmd()
	int			secret;
	unsigned int		frameNextSecond;
	unsigned int		frameNextTenSeconds;
	connectqueue_t		connectqueue[11];
}serverStaticExt_t;

typedef struct {
	qboolean		cmdSystemInitialized;
	int			randint;
	translatedCmds_t	translatedCmd[MAX_TRANSCMDS];
	int			challenge;
	int			useuids;
	int			masterServer_id;
	char			masterServer_challengepassword[33];
	netadr_t		masterServer_adr;
}permServerStatic_t;


typedef enum {
	SS_DEAD,			// no map loaded
	SS_LOADING,			// spawning level entities
	SS_GAME				// actively running
} serverState_t;

typedef struct svEntity_s {//Everything is not validated except size
	struct svEntity_s *nextEntityInWorldSector;
	
	entityState_t		baseline;		// 0x04  for delta compression of initial sighting
	int			numClusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			lastCluster;		// if all the clusters don't fit in clusternums
	int			areanum, areanum2;
	int			snapshotCounter;	// used to prevent double adding from portal views
	int			unk[11];
} svEntity_t; //size: 0x178

#define MAX_BPS_WINDOW 20

#pragma pack(push, 1)

typedef struct {//0x13e78d00
	serverState_t		state;
	int			timeResidual;		// <= 1000 / sv_frame->value
	int			frameusec;		// Frameusec set every Level-startup to the desired value from sv_fps
	qboolean		restarting;		// if true, send configstring changes during SS_LOADING
	int			serverId;		//restartedServerId;	serverId before a map_restart
	int			checksumFeed;		// 0x14 the feed key that we use to compute the pure checksum strings
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
	// the serverId associated with the current checksumFeed (always <= serverId)
/*	int			checksumFeedServerId;	
	int			snapshotCounter;	// incremented for each snapshot built

	int			nextFrameTime;		// when time > nextFrameTime, process world
	struct cmodel_s		*models[MAX_MODELS];*/
	byte			unk[0x800];

	unsigned short		unkConfigIndex;		//0x13e79518
	unsigned short		configstringIndex[MAX_CONFIGSTRINGS]; //(0x13e7951a)

	short			unk3; //0x13e7a82e
	svEntity_t		svEntities[MAX_GENTITIES]; //0x1b30 (0x13e7a830) size: 0x5e000

	// the game virtual machine will update these on init and changes
	sharedEntity_t		*gentities;	//0x5fb30  (0x13ed8830)
	int			gentitySize;	//0x5fb34  (0x13ed8834)
	int			num_entities;		// current number, <= MAX_GENTITIES

	playerState_t		*gameClients;		//0x5fb3c
	int			gameClientSize;		//0x5fb40 (13ed8840)will be > sizeof(playerState_t) due to game private data
/*
	int				restartTime;
	int				time;*/
	
	int			field_5FB44;
	int			field_5FB48;
	int			bpsWindow[MAX_BPS_WINDOW];
	int			bpsWindowSteps;
	int			bpsTotalBytes;
	int			bpsMaxBytes;
	int			ubpsWindow[MAX_BPS_WINDOW];
	int			ubpsTotalBytes;
	int			ubpsMaxBytes;
	float		ucompAve;
	int			ucompNum;
	char		gametype[MAX_QPATH]; //(0x13ed8908)
	qboolean	unk5;
	qboolean	unk6;
} server_t;//Size: 0x5fc50

#pragma pack(pop)






typedef struct{//13F18F80
	client_t	*clients;
	int			time;
	int			snapFlagServerBit;// ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()
	int			numSnapshotEntities;	//0x13f18f8c sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int			numSnapshotClients;
	int			nextSnapshotEntities;	//0x13f18f94 next snapshotEntities to use
	int			nextSnapshotClients;	//0x13f18f98
	entityState_t		*snapshotEntities;	//0x13f18f9c
	clientState_ts		*snapshotClients;	//0x13f18fa0
	svEntity_t		*svEntities;		//0x13f18fa4
	
	vec3_t mapCenter;
	archivedEntity_t *archivedEntities;
	entityUnknownStr2_t *entUnknown2;
	byte *archiveSnapBuffer;
	entityUnknownStr_t *entUnknown1;
	int nextCachedSnapshotFrames;
	int nextArchivedSnapshotFrames;
	int nextCachedSnapshotEntities;
	int nextCachedSnapshotClients;
	int num_entities;
	int maxClients;
	int fps;
	qboolean canArchiveData;
	sharedEntity_t *gentities;
	int gentitySize;
	clientSession_t *gclientstate;
	gclient_t *gplayerstate;
	int gclientSize;
	
	
	/*
	int			unkBig[7];
	int			var_01;			//0x13f18fc4
	int			var_02;
	int			var_03;
	int			var_04;			//0x13f18fd0
	 */
}svsHeader_t;


int SV_NumForGentity( sharedEntity_t *ent );
sharedEntity_t *SV_GentityNum( int num );
playerState_t *SV_GameClientNum( int num );
svEntity_t  *SV_SvEntityForGentity( sharedEntity_t *gEnt );
sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt );

//
// sv_client.c
//
void SV_ChallengeResponse( int );

void SV_PBAuthChallengeResponse( int );

void SV_Heartbeat_f( void );

void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK, qboolean inDl );

void SV_SendClientSnapshot( client_t *cl );

qboolean SV_Acceptclient(int);
client_t* SV_ReadPackets(netadr_t *from, unsigned short qport);
void SV_GetVoicePacket(netadr_t *from, msg_t* msg);
void SV_UserVoice(client_t* cl, msg_t* msg);
void SV_PreGameUserVoice(client_t* cl, msg_t* msg);
void SV_BuildClientSnapshot(client_t* cl);
void SV_ArchiveSnapshot(msg_t* msg);

void QDECL SV_SendServerCommand_IW(client_t *cl, int type, const char *fmt, ...);
void QDECL SV_SendServerCommand(client_t *cl, const char *fmt, ...);

__optimize3 __regparm2 void SV_PacketEvent( netadr_t *from, msg_t *msg );

void SV_AddServerCommand( client_t *cl, int type, const char *cmd );

void Scr_SpawnBot(void);

char*	SV_IsGUID(char* guid);

void SV_Shutdown( const char* finalmsg);

void SV_WriteGameState(msg_t*, client_t*);

void SV_GetServerStaticHeader(void);

void SV_ShowClientUnAckCommands( client_t *client );


void SV_WriteDemoMessageForClient( byte *msg, int dataLen, client_t *client );
void SV_StopRecord( client_t *cl );
void SV_RecordClient( client_t* cl, char* basename );
void SV_DemoSystemShutdown( void );
void SV_WriteDemoArchive(client_t *client);

void SV_SendClientVoiceData(client_t *client);

void SV_InitCvarsOnce( void );

void SV_Init( void );

__optimize2 __regparm1 qboolean SV_Frame( unsigned int usec );

unsigned int SV_FrameUsec( void );

void SV_RemoveAllBots( void );

const char* SV_GetMapRotation( void );

void SV_AddOperatorCommands(void);

__optimize3 __regparm1 void SV_GetChallenge(netadr_t *from);
__optimize3 __regparm1 void SV_AuthorizeIpPacket( netadr_t *from );
__optimize3 __regparm1 void SV_DirectConnect( netadr_t *from );
__optimize3 __regparm2 void SV_ReceiveStats(netadr_t *from, msg_t* msg);
void SV_UserinfoChanged( client_t *cl );
void SV_DropClient( client_t *drop, const char *reason );
void SV_DelayDropClient(client_t *client, const char *dropmsg);
__optimize3 __regparm3 void SV_UserMove( client_t *cl, msg_t *msg, qboolean delta );
void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd );
void SV_WriteDownloadToClient( client_t *cl, msg_t *msg );
void SV_SendClientGameState( client_t *client );

void SV_Netchan_Decode( client_t *client, byte *data, int remaining );
void SV_Netchan_Encode( client_t *client, byte *data, int cursize );
qboolean SV_Netchan_Transmit( client_t *client, byte *data, int cursize);
qboolean SV_Netchan_TransmitNextFragment( client_t *client );
void SV_SysAuthorize(char* s);
int SV_ClientAuthMode(void);
qboolean SV_FriendlyPlayerCanBlock(void);
qboolean SV_FFAPlayerCanBlock(void);
const char* SV_GetMessageOfTheDay(void);
const char* SV_GetNextMap(void);
void QDECL SV_EnterLeaveLog( const char *fmt, ... );


qboolean SV_ClientCommand( client_t *cl, msg_t *msg, qboolean inDl);

void SV_WriteRconStatus( msg_t *msg );

void G_PrintAdvertForPlayer(client_t*);
void G_PrintRuleForPlayer(client_t*);
void G_AddRule(const char* newtext);
void G_AddAdvert(const char* newtext);
void G_SetupHudMessagesForPlayer(client_t*);


void SV_SayToPlayers(int clnum, int team, char* text);


__optimize3 __regparm2 void SV_ExecuteClientMessage( client_t *cl, msg_t *msg );

void SV_GetUserinfo( int index, char *buffer, int bufferSize );

qboolean SV_Map(const char* levelname);
void SV_MapRestart( qboolean fastrestart );

void __cdecl SV_SetConfigstring(int index, const char *text);
//SV_SetConfigstring SV_SetConfigstring = (tSV_SetConfigstring)(0x8173fda);
const char* SV_GetGuid(unsigned int clnum);
qboolean SV_ExecuteRemoteCmd(int, const char*);
qboolean SV_UseUids();
int SV_GetUid(unsigned int);
void SV_SetUid(unsigned int clnum, unsigned int uid);
sharedEntity_t* SV_AddBotClient();
sharedEntity_t* SV_RemoveBot();
qboolean SV_AddBan(int, int, char*, char*, time_t, char*);

//sv_ingameadmin.c:
void SV_RemoteCmdInit();
void SV_RemoteCmdClearAdminList();
int SV_RemoteCmdGetClPower(client_t* cl);
int SV_RemoteCmdGetClPowerByUID(int uid);
void SV_ExecuteBroadcastedCmd(int uid, const char *msg);
qboolean SV_RemoteCmdAddAdmin(int uid, char* guid, int power);
qboolean SV_RemoteCmdInfoAddAdmin(const char* infostring);
void SV_RemoteCmdWriteAdminConfig(char* buffer, int size);
void QDECL SV_PrintAdministrativeLog( const char *fmt, ... );

void SV_RemoteCmdSetAdmin(int uid, char* guid, int power);
void SV_RemoteCmdUnsetAdmin(int uid, char* guid);
void SV_RemoteCmdSetPermission(char* command, int power);
void SV_RemoteCmdListAdmins( void );
__cdecl qboolean SV_GameCommand(void);

void SV_GetConfigstring( int index, char *buffer, int bufferSize );

extern cvar_t* sv_protocol;
extern cvar_t* sv_padPackets;
extern cvar_t* sv_demoCompletedCmd;
extern cvar_t* sv_mapDownloadCompletedCmd;
extern cvar_t* sv_wwwBaseURL;
extern cvar_t* sv_maxPing;
extern cvar_t* sv_minPing;
extern cvar_t* sv_authorizemode;
extern cvar_t* sv_privateClients;
extern cvar_t* sv_privatePassword;
extern cvar_t* sv_reconnectlimit;
extern cvar_t* sv_wwwDlDisconnected;
extern cvar_t* sv_allowDownload;
extern cvar_t* sv_wwwDownload;
extern cvar_t* sv_autodemorecord;
extern cvar_t* sv_modStats;
extern cvar_t* sv_password;
extern cvar_t* sv_mapRotation;
extern cvar_t* sv_mapRotationCurrent;
extern cvar_t* sv_cheats;
extern cvar_t* sv_consayname;
extern cvar_t* sv_contellname;
extern cvar_t* sv_maxclients;
extern cvar_t* sv_g_gametype;
extern cvar_t* sv_pure;
extern cvar_t* sv_fps;
extern cvar_t* sv_serverid;
extern cvar_t* sv_maxRate;
extern cvar_t* sv_mapname;
extern cvar_t* sv_floodProtect;
extern cvar_t* sv_showAverageBPS;
extern cvar_t* sv_hostname;
extern cvar_t* sv_shownet;

void __cdecl SV_StringUsage_f(void);
void __cdecl SV_ScriptUsage_f(void);
void __cdecl SV_BeginClientSnapshot( client_t *cl, msg_t* msg);
void __cdecl SV_EndClientSnapshot( client_t *cl, msg_t* msg);
void __cdecl SV_ClientThink( client_t *cl, usercmd_t * );
void __cdecl SV_SpawnServer(const char* levelname);
void __cdecl SV_SetGametype( void );
void __cdecl SV_InitCvars( void );
void __cdecl SV_RestartGameProgs( int savepersist );
void __cdecl SV_ResetSekeletonCache(void);
void __cdecl SV_PreFrame(void);
void __cdecl SV_SendClientMessages(void);
void __cdecl SV_SetServerStaticHeader(void);
void __cdecl SV_ShutdownGameProgs(void);
void __cdecl SV_FreeClients(void);
void __cdecl SV_GameSendServerCommand(int clientnum, int svscmd_type, const char *text);
void __cdecl SV_SetConfigstring(int index, const char *text);
void __cdecl SV_FreeClient(client_t* drop);
void __cdecl SV_FreeClientScriptId(client_t *cl);
void __cdecl SV_LinkEntity(gentity_t*);
void __cdecl SV_UnlinkEntity(gentity_t*);
void serverStatus_Write();

int SV_GetPlayerUIDByHandle(const char* handle);
client_t* SV_GetPlayerClByHandle(const char* handle);
const char* SV_GetPlayerNameByHandle(const char* handle);

//sv_banlist.c
void SV_InitBanlist( void );
qboolean  SV_ReloadBanlist();
char* SV_PlayerIsBanned(int uid, char* pbguid, netadr_t *addr, char* message, int len);
char* SV_PlayerBannedByip(netadr_t *netadr, char* message, int len);	//Gets called in SV_DirectConnect
void SV_PlayerAddBanByip(netadr_t *remote, char *reason, int uid, char* guid, int adminuid, int expire);		//Gets called by future implemented ban-commands and if a prior ban got enforced again - This function can also be used to unset bans by setting 0 bantime
qboolean SV_RemoveBan(int uid, char* guid, char* name);
void SV_DumpBanlist( void );
void SV_AddSafeCommands();
extern	serverStaticExt_t	svse;	// persistant server info across maps
extern	permServerStatic_t	psvs;	// persistant even if server does shutdown

struct moveclip_s;
struct trace_s;


__cdecl void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg );
__cdecl void SV_SendMessageToClient( msg_t *msg, client_t *client );
__cdecl void SV_WriteSnapshotToClient(client_t* client, msg_t* msg);
__cdecl void SV_ClipMoveToEntity(struct moveclip_s *clip, svEntity_t *entity, struct trace_s *trace);
void SV_Cmd_Init();
void SV_CopyCvars();

#ifdef COD4X18UPDATE
void SV_ConnectWithUpdateProxy(client_t *cl);
#endif

#endif



