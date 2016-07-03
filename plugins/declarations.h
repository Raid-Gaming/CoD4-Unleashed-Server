/*
===========================================================================
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team

    This file is part of IceOps Plugin Handler source code.

    IceOps Plugin Handler source code is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    IceOps Plugin Handler source code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================
*/


#ifndef PLUGIN_INCLUDES

    #error Please include pinc.h instead!

#endif /*PLUGIN_INCLUDES*/

#include <string.h>


#define MAX_QPATH 64
#define GENTITYNUM_BITS     10  // JPW NERVE put q3ta default back for testing	// don't need to send any more
//#define	GENTITYNUM_BITS		11		// don't need to send any more		(SA) upped 4/21/2001 adjusted: tr_local.h (802-822), tr_main.c (1501), sv_snapshot (206)
#define MAX_GENTITIES       ( 1 << GENTITYNUM_BITS )
#define	MAX_STRING_CHARS	1024
#define MAX_INFO_STRING		1024
#define MAX_RELIABLE_COMMANDS 128
#define MAX_NAME_LENGTH		16
#define MAX_DOWNLOAD_WINDOW	8
#define MAX_OSPATH			256
#define PACKET_BACKUP		32
#define MAX_CLIENTS			64
#define NETCHAN_UNSENTBUFFER_SIZE 0x20000
#define NETCHAN_FRAGMENTBUFFER_SIZE 0x800

#define CLIENT_BASE_ADDR 0x90b4f8C
#define clientbase ((client_t*)CLIENT_BASE_ADDR)  //e.g. clientbase[i].username

//Types and structs
typedef int scr_entref_t;
typedef int	fileHandle_t;
typedef enum {qfalse, qtrue}	qboolean;
typedef void (*xcommand_t)();
typedef void (*xfunction_t)();
typedef void *client_t_ptr;
typedef unsigned char byte;

// Used for internet communication
typedef enum {
	NA_BAD = 0,					// an address lookup failed
	NA_BOT = 0,
	NA_LOOPBACK = 2,
	NA_BROADCAST = 3,
	NA_IP = 4,
	NA_IP6 = 5,
	NA_TCP = 6,
	NA_TCP6 = 7,
	NA_MULTICAST6 = 8,
	NA_UNSPEC = 9,
	NA_DOWN = 10
} netadrtype_t;

#pragma pack(1)

typedef struct {
	netadrtype_t	type;
	int				scope_id;
	unsigned short	port;
	int				sock;	//Socket FD. 0 = any socket
    union{
	    byte	ip[4];
	    byte	ipx[10];
	    byte	ip6[16];
	};
}netadr_t;

typedef struct {
	// sequencing variables
	int			outgoingSequence;
	int			sock;
	int			dropped;			// between last packet and previous
	int			incomingSequence;

	//Remote address
	netadr_t	remoteAddress;			// (0x10)
	int			qport;				// qport value to write when transmitting (0x24)
	// incoming fragment assembly buffer
	int			fragmentSequence;
	int			fragmentLength;	
	byte		*fragmentBuffer; // Old: (0x30)
	int			fragmentBufferSize;

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	qboolean	unsentFragments;
	int			unsentFragmentStart;
	int			unsentLength;
	byte		*unsentBuffer; //Old: (0x44)
	int			unsentBufferSize;
} netchan_t;

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef void convariable_t;

typedef struct{		// A structure representing a player's scoreboard
    int	score;
    int	deaths;
    int	kills;
    int	assists;
}clientScoreboard_t;

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {//Not Known
	int			serverTime;
	int			buttons;
	int			angles[3];
	byte weapon;
	byte offhandindex;
	byte field_16;
	byte field_17;
	int field_18;
	int field_1C;
} usercmd_t;


typedef enum{
    CVAR_BOOL,
    CVAR_FLOAT,
    CVAR_VEC2,
    CVAR_VEC3,
    CVAR_VEC4,
    CVAR_INT,
    CVAR_ENUM,
    CVAR_STRING,
    CVAR_COLOR
}cvarType_t;

typedef struct{
	char *name;
	char *description;
	short int flags;
	byte type;
	byte modified;
	union{
		float floatval;
		float value;
		int integer;
		char* string;
		byte boolean;
	};
	union{
		float latchedfloatval;
		float latchedvalue;
		int latchedinteger;
		char* latchedstring;
		byte latchedboolean;
	};
} cvar_t;

#define	CVAR_ARCHIVE		1	// set to cause it to be saved to vars.rc
								// used for system variables, not for player
								// specific configurations
#define	CVAR_USERINFO		2	// sent to server on connect or change
#define	CVAR_SERVERINFO		4	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		8	// these cvars will be duplicated on all clients
#define	CVAR_INIT		16	// don't allow change from console at all,
								// but can be set from the command line
#define	CVAR_LATCH		32	// will only change when C code next does
								// a Cvar_Get(), so it can't be changed
								// without proper initialization.  modified
								// will be set, even though the value hasn't
								// changed yet
#define	CVAR_ROM		64	// display only, cannot be set by user at all
#define CVAR_CHEAT		128	// can not be changed if cheats are disabled
#define	CVAR_TEMP		256	// can be set even when cheats are disabled, but is not archived
#define CVAR_NORESTART		1024	// do not clear when a cvar_restart is issued
#define	CVAR_USER_CREATED	16384	// created by a set command


#define cvardeclarations

//typedef int fileHandle_t;
typedef int clipHandle_t;

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t		trBase;
	vec3_t		trDelta;			// velocity, etc
} trajectory_t;


typedef struct{
	int	cullDist;
	int	period;
}lerp_loopFx_t;


typedef struct{
	int	bodyPitch;
	int	bodyRoll;
	int	steerYaw;
	int	materialTime;
	int	gunPitch;
	int	gunYaw;
	int	teamAndOwnerIndex;
}lerp_vehicle_t;


typedef struct{
	int	lerp;
}lerp_soundBlend_t;


typedef struct{
	int	launchTime;
}lerp_missile_t;


typedef struct{
	int	leanf;
	int	movementDir;
}lerp_player_t;


typedef struct{
	int	data[6];
}lerp_anonymous_t;

typedef struct {
	int		eFlags;
	trajectory_t	pos;	// for calculating position	0x0c
	trajectory_t	apos;	// for calculating angles	0x30
	union{
		lerp_anonymous_t	anonymous;
		lerp_player_t		player;
		lerp_missile_t		missile;
		lerp_soundBlend_t	soundBlend;
		lerp_loopFx_t		loopFx;
		lerp_vehicle_t		vehicle;
	}u;
}lerp_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s {//Confirmed names and offsets but not types

	int		number;			// entity index	//0x00
	int		eType;			// entityType_t	//0x04

	lerp_t		lerp;

	int		time2;			//0x70


	int		otherEntityNum;		//0x74 shotgun sources, etc
	int		attackerEntityNum;	//0x78

	int		groundEntityNum;	//0x7c -1 = in air

	int		loopSound;		//0x80 constantly loop this sound
	int		surfType;		//0x84


	clipHandle_t	index;			//0x88
	int		clientNum;		//0x8c 0 to (MAX_CLIENTS - 1), for players and corpses
	int		iHeadIcon;		//0x90
	int		iHeadIconTeam;		//0x94

	int		solid;			//0x98 for client side prediction, trap_linkentity sets this properly	0x98

	int		eventParm;		//0x9c impulse events -- muzzle flashes, footsteps, etc
	int		eventSequence;		//0xa0

	vec4_t		events;			//0xa4
	vec4_t		eventParms;		//0xb4

	// for players
	int		weapon;			//0xc4 determines weapon and flash model, etc
	int		weaponModel;		//0xc8
	int		legsAnim;		//0xcc mask off ANIM_TOGGLEBIT
	int		torsoAnim;		//0xd0 mask off ANIM_TOGGLEBIT

	union{
		int	helicopterStage;	//0xd4
	}un1;
	int		un2;			//0xd8
	int		fTorsoPitch;		//0xdc
	int		fWaistPitch;		//0xe0
	vec4_t		partBits;		//0xe4
} entityState_t;	//sizeof(entityState_t): 0xf4


typedef struct {
	//entityState_t	s;				//Duplicated struct is removed
	byte		linked;				//0xf4 qfalse if not in any good cluster

	byte		bmodel;				//0xf5 if false, assume an explicit mins / maxs bounding box
							// only set by trap_SetBrushModel
	short		unknown1;
	int		unknown2[4];

	vec3_t		mins, maxs;		//0x108  //0x114  from SharedEntity_t

	int		contents;		// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
						// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		//0x124  //0x130 derived from mins/maxs and origin + rotation

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;		//0x13c
	vec3_t		currentAngles;		//0x148

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->r.ownerNum == passEntityNum	(don't interact with your own missiles)
	// entity[ent->r.ownerNum].r.ownerNum == passEntityNum	(don't interact with other missiles from owner)
	short			ownerNum;	//0x154
	short			unknown3;
	int			eventTime;
} entityShared_t;

typedef struct{
	int	sprintButtonUpRequired;
	int	sprintDelay;
	int	lastSprintStart;
	int	lastSprintEnd;
	int	sprintStartMaxLength;
}sprintState_t;

typedef struct{
	int	yaw;
	int	timer;
	int	transIndex;
	int	flags;
}mantleState_t;

typedef struct playerState_s {
	int		commandTime;  // 0
	int		pm_type;  // 4
	int		bobCycle;  // 8
	int		pm_flags;  // 12
	int		weapFlags;  // 16
	int		otherFlags;  // 20
	int		pm_time;  // 24
	vec3_t		origin;  // 28
	
	// http://zeroy.com/script/player/getvelocity.htm
	vec3_t		velocity;  // 40

	int		var_01;  //
	int		var_02;  //

	int		weaponTime;  // 60
	int		weaponDelay;  // 64
	int		grenadeTimeLeft;  // 68
	int		throwBackGrenadeOwner;  // 72
	int		throwBackGrenadeTimeLeft;  // 76
	int		weaponRestrictKickTime;  // 80
	int		foliageSoundTime;  // 84
	int		gravity;  // 88
	int		leanf;  // 92
	int		speed;  // 96
	vec3_t		delta_angles;  // 100
	
	/*The ground entity's rotation will be added onto the player's view.  In particular, this will 
	* cause the player's yaw to rotate around the entity's z-axis instead of the world z-axis. 
	* Any rotation that the reference entity undergoes will affect the player.
	* http://zeroy.com/script/player/playersetgroundreferenceent.htm */
	int		groundEntityNum;  // 112

	vec3_t		vLadderVec;  // 116
	int		jumpTime;  // 128
	float		jumpOriginZ;  // 132
	
	// Animations as in mp/playeranim.script and animtrees/multiplayer.atr, it also depends on mp/playeranimtypes.txt (the currently used weapon)
	int		legsTimer;  // 136
	int		legsAnim;  // 140
	int		torsoTimer;  // 144
	int		torsoAnim;  // 148

	int		var_03;  //
	int		var_04;  //

	int		damageTimer;  // 160
	int		damageDuration;  // 164
	int		flinchYawAnim;  // 168
	int		movementDir;  // 172
	int		eFlags;  // 176
	int		eventSequence;  // 180

	vec4_t		events;  // 184
	vec4_t		eventParms;  // 200

	int		var_05;  //

	int		clientNum;  // 220
	int		offHandIndex;  // 224
	int		offhandSecondary;  // 228
	int		weapon;  // 232
	int		weaponstate;  // 236
	int		weaponShotCount;  // 240
	int		fWeaponPosFrac;  // 244
	int		adsDelayTime;  // 248
	
	// http://zeroy.com/script/player/resetspreadoverride.htm
	// http://zeroy.com/script/player/setspreadoverride.htm
	int		spreadOverride;  // 252
	int		spreadOverrideState;  // 256
	
	int		viewmodelIndex;  // 260

	vec3_t		viewangles;  // 264
	int		viewHeightTarget;  // 276
	int		viewHeightCurrent;  // 280
	int		viewHeightLerpTime;  // 284
	int		viewHeightLerpTarget;  // 288
	int		viewHeightLerpDown;  // 292
	vec2_t		viewAngleClampBase;  // 296
	vec2_t		viewAngleClampRange;  // 304

	int		damageEvent;  // 312
	int		damageYaw;  // 316
	int		damagePitch;  // 320
	int		damageCount;  // 324

	int		unk1[261];  // 328

	vec4_t		weapons;  // 1372

	vec4_t		weaponold;  // 1388

	vec4_t		weaponrechamber;  // 1404

	int		proneDirection;  // 1420
	int		proneDirectionPitch;  // 1424
	int		proneTorsoPitch;  // 1428
	int		viewlocked;  // 1432
	int		viewlocked_entNum;  // 1436

	int		cursorHint;  // 1440
	int		cursorHintString;  // 1444
	int		cursorHintEntIndex;  // 1448

	int		iCompassPlayerInfo;  // 1452
	int		radarEnabled;  // 1456

	int		locationSelectionInfo;  // 1460
	sprintState_t	sprintState;  // 1464
	
	// used for leaning?
	int		fTorsoPitch;  // 1484
	int		fWaistPitch;  // 1488

	int		holdBreathScale;  // 1492
	int		holdBreathTimer;  // 1496
	
	// Scales player movement speed by this amount, ???it's actually a float???
	// http://zeroy.com/script/player/setmovespeedscale.htm
	int		moveSpeedScaleMultiplier;  // 1500
	
	mantleState_t	mantleState;  // 1504
	int		meleeChargeYaw;  // 1520
	int		meleeChargeDist;  // 1524
	int		meleeChargeTime;  // 1528
	int		perks;  // 1532

	vec4_t		actionSlotType;  // 1536
	vec4_t		actionSlotParam;  // 1552

	int		var_06; // 1568

	int		weapAnim;  // 1572
	int		aimSpreadScale;  // 1576
	
	// http://zeroy.com/script/player/shellshock.htm
	int		shellshockIndex;  // 1580
	int		shellshockTime;  // 1584
	int		shellshockDuration;  // 1588

	// http://zeroy.com/script/player/setdepthoffield.htm
	int		dofNearStart;  // 1592
	int		dofNearEnd;  // 1596
	int		dofFarStart;  // 1600
	int		dofFarEnd;  // 1604
	int		dofNearBlur;  // 1608
	int		dofFarBlur;  // 1612
	int		dofViewmodelStart;  // 1616
	int		dofViewmodelEnd;  // 1620

	int		unk2[145];  // 1624

	int		deltaTime;  // 2204
	int		killCamEntity;  // 2208

	int		unk3[2480];  // 2212
} playerState_t;//Size: 0x2f64




typedef enum {
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse
				// connection for a couple seconds
	CS_CONNECTED,		// has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,		// gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE		// client is fully in game
}clientState_t;

// the server looks at a sharedEntity, which is the start of the game's gentity_t structure
typedef struct {
	entityState_t	s;				// communicated by server to clients
	entityShared_t	r;				// shared by both the server system and game
} sharedEntity_t;


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

typedef struct
{
	char num;
	char data[256];
	int dataLen;
}voices_t;


typedef struct
{
    int checksum;
    byte bytedata[2000];
    int longdata[1547];
}statData_t;

typedef struct client_s {//90b4f8c
	clientState_t		state;
	int			unksnapshotvar;		// must timeout a few frames in a row so debugging doesn't break
	int			deltaMessage;		// (0x8) frame last client usercmd message
	qboolean		rateDelayed;		// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	netchan_t		netchan;	//(0x10)
	//DemoData
	byte			demofile[284];
	qboolean		demorecording;
	qboolean		demowaiting;
	char			demoName[MAX_QPATH];
	int			demoArchiveIndex;
	int			demoMaxDeltaFrames;
	int			demoDeltaFrameCount;

	int			authentication;
	qboolean		playerauthorized;
	qboolean		noPb;
	int			usernamechanged;
	int			bantime;
	int			clienttimeout;
	int			uid;
	char			OS;
	int			power;
	char			originguid[33];
	qboolean		firstSpawn;
	void		*hudMsg;
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
	byte			free[652];
	char			name[64];

	int			unknownUsercmd1;	//0x63c
	int			unknownUsercmd2;	//0x640
	int			unknownUsercmd3;	//0x644
	int			unknownUsercmd4;	//0x648

	const char*		delayDropMsg;		//0x64c
	char			userinfo[MAX_INFO_STRING];		// name, etc (0x650)
	byte		reliableCommands[MAX_RELIABLE_COMMANDS * (MAX_STRING_CHARS + 2 * sizeof(int))];	// (0xa50)
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
	fileHandle_t	download;		//(0x212e4) file being downloaded
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
	statData_t		stats;		//(0xa3639)
	byte			receivedstats;		//(0xa5639)
	byte			dummy1;
	byte			dummy2;
} client_t;//0x0a563c



typedef struct gentity_s gentity_t;


struct gentity_s {
	entityState_t s;
	entityShared_t r;               // shared by both the server system and game

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s    *client;            // NULL if not a client		0x15c

	qboolean inuse;

	byte unknown[0x110]; //0x164


/*
	char        *classname;         // set in QuakeEd
	int spawnflags;                 // set in QuakeEd

	qboolean neverFree;             // if true, FreeEntity will only unlink
									// bodyque uses this

	int flags;                      // FL_* variables

	char        *model;
	char        *model2;
	int freetime;                   // level.time when the object was freed

	int eventTime;                  // events will be cleared EVENT_VALID_MSEC after set
	qboolean freeAfterEvent;
	qboolean unlinkAfterEvent;

	qboolean physicsObject;         // if true, it can be pushed by movers and fall off edges
									// all game items are physicsObjects,
	float physicsBounce;            // 1.0 = continuous bounce, 0.0 = no bounce
	int clipmask;                   // brushes with this content value will be collided against
									// when moving.  items and corpses do not collide against
									// players, for instance

	// movers
	moverState_t moverState;
	int soundPos1;
	int sound1to2;
	int sound2to1;
	int soundPos2;
	int soundLoop;
	// JOSEPH 1-26-00
	int sound2to3;
	int sound3to2;
	int soundPos3;
	// END JOSEPH

	int soundKicked;
	int soundKickedEnd;

	int soundSoftopen;
	int soundSoftendo;
	int soundSoftclose;
	int soundSoftendc;

	gentity_t   *parent;
	gentity_t   *nextTrain;
	gentity_t   *prevTrain;
	// JOSEPH 1-26-00
	vec3_t pos1, pos2, pos3;
	// END JOSEPH

	char        *message;

	int timestamp;              // body queue sinking, etc   //0x1bc

	float angle;                // set in editor, -1 = up, -2 = down
	char        *target;
	char        *targetname;
	char        *team;
	char        *targetShaderName;
	char        *targetShaderNewName;
	gentity_t   *target_ent;

	float speed;
	float closespeed;           // for movers that close at a different speed than they open
	vec3_t movedir;

	int gDuration;
	int gDurationBack;
	vec3_t gDelta;
	vec3_t gDeltaBack;

	int nextthink;
	void ( *think )( gentity_t *self );
	void ( *reached )( gentity_t *self );       // movers call this when hitting endpoint
	void ( *blocked )( gentity_t *self, gentity_t *other );
	void ( *touch )( gentity_t *self, gentity_t *other, trace_t *trace );
	void ( *use )( gentity_t *self, gentity_t *other, gentity_t *activator );
	void ( *pain )( gentity_t *self, gentity_t *attacker, int damage, vec3_t point );
	void ( *die )( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod );

	int pain_debounce_time;
	int fly_sound_debounce_time;            // wind tunnel
	int last_move_time;

	int health;		//0x1A0 ??

	qboolean takedamage;	//0x16b

	int damage;
	int splashDamage;           // quad will increase this without increasing radius
	int splashRadius;
	int methodOfDeath;
	int splashMethodOfDeath;

	int count;

	gentity_t   *chain;
	gentity_t   *enemy;
	gentity_t   *activator;
	gentity_t   *teamchain;     // next entity in team
	gentity_t   *teammaster;    // master of the team

	int watertype;
	int waterlevel;

	int noise_index;

	// timing variables
	float wait;		//
	float random;		//

	// Rafael - sniper variable
	// sniper uses delay, random, radius
	int radius;
	float delay;

	// JOSEPH 10-11-99
	int TargetFlag;
	float duration;
	vec3_t rotate;
	vec3_t TargetAngles;
	// END JOSEPH

	gitem_t     *item;          // for bonus items

	// Ridah, AI fields
	char        *aiAttributes;
	char        *aiName;
	int aiTeam;
	void ( *AIScript_AlertEntity )( gentity_t *ent );
	qboolean aiInactive;
	int aiCharacter;            // the index of the type of character we are (from aicast_soldier.c)
	// done.

	char        *aiSkin;
	char        *aihSkin;

	vec3_t dl_color;
	char        *dl_stylestring;
	char        *dl_shader;
	int dl_atten;


	int key;                    // used by:  target_speaker->nopvs,

	qboolean active;	//0x16c
	qboolean botDelayBegin;

	// Rafael - mg42
	float harc;
	float varc;

	int props_frame_state;

	// Ridah
	int missionLevel;               // mission we are currently trying to complete
									// gets reset each new level
	// done.

	// Rafael
	qboolean is_dead;
	// done

	int start_size;
	int end_size;

	// Rafael props

	qboolean isProp;

	int mg42BaseEnt;

	gentity_t   *melee;

	char        *spawnitem;

	qboolean nopickup;

	int flameQuota, flameQuotaTime, flameBurnEnt;

	int count2;

	int grenadeExplodeTime;         // we've caught a grenade, which was due to explode at this time
	int grenadeFired;               // the grenade entity we last fired

	int mg42ClampTime;              // time to wait before an AI decides to ditch the mg42

	char        *track;

	// entity scripting system
	char                *scriptName;

	int numScriptEvents;
	g_script_event_t    *scriptEvents;  // contains a list of actions to perform for each event type
	g_script_status_t scriptStatus;     // current status of scripting
	// the accumulation buffer
	int scriptAccumBuffer[G_MAX_SCRIPT_ACCUM_BUFFERS];

	qboolean AASblocking;
	float accuracy;

	char        *tagName;       // name of the tag we are attached to
	gentity_t   *tagParent;

	float headshotDamageScale;

	int lastHintCheckTime;                  // DHM - Nerve
	// -------------------------------------------------------------------------------------------
	// if working on a post release patch, new variables should ONLY be inserted after this point
	// DHM - Nerve :: the above warning does not really apply to MP, but I'll follow it for good measure

	int voiceChatSquelch;                   // DHM - Nerve
	int voiceChatPreviousTime;              // DHM - Nerve
	int lastBurnedFrameNumber;              // JPW - Nerve   : to fix FT instant-kill exploit*/
};//Size: 0x274


#pragma pack(push, 1)


#include "plugin_declarations.h"
#include "function_declarations.h" // Function descriptions are available in this file
#include "callback_declarations.h"
