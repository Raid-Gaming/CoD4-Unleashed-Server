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



#include "sv_auth.h"
#include "q_shared.h"
#include "qcommon.h"
#include "qcommon_io.h"
#include "cmd.h"
#include "nvconfig.h"
#include "msg.h"
#include "sys_net.h"
#include "server.h"
#include "net_game_conf.h"
#include "sha256.h"
#include "punkbuster.h"
#include "net_game.h"
#include "g_sv_shared.h"
#include "sec_main.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
/*
========================================================================

Temporary solution for admin authorization due to 3xP clan stealing CD-Keys

========================================================================
*/


authData_t auth_admins;

#define AUTH_DEFAULT_POWER 1


const char* Auth_FindSessionID(const char* sessionid)
{
	int i;
	authData_admin_t *user;
	
	if(strlen(sessionid) != 64)
	{
		return NULL;
	}
	
	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		
		if(!Q_stricmp(user->sessionid, sessionid))
		{
			return user->username;
		}
	}
	return NULL;
	
}

const char* Auth_GetSessionId(const char* username, const char *password)
{
	int handle;
	byte buff[129];
	unsigned long size;
	handle = Auth_Authorize(username, password);
	if(handle < 0)
	{
		return NULL;
	}
	size = sizeof(auth_admins.admins[handle].sessionid);
	Com_RandomBytes(buff, sizeof(buff));
	Sec_HashMemory( SEC_HASH_SHA256, buff, sizeof(buff), auth_admins.admins[handle].sessionid, &size, qfalse );
	return auth_admins.admins[handle].sessionid;
}

authData_admin_t* Auth_GetByUsername(const char* username)
{
	int i;
	authData_admin_t *user;
	
    for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		if(*user->username && !Q_stricmp(user->username, username))
		{
			return user;
		}
    }
    return NULL;
}

void Auth_WipeSessionId(const char* username)
{
	authData_admin_t *user;
	int i, id;
	id = -1;

	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++)
	{
		if(*user->username && !Q_stricmp(user->username,username))
		{
			id = i;
			break;
		}
	}

	if(id < 0)
	{
		return;
	}

	auth_admins.admins[id].sessionid[0] = '\0';
	return;
}



int Auth_Authorize(const char *login, const char *password){
    int i;
    char hstring[256];
    const char *sha256;
    authData_admin_t *user;
    int id = -1;
    
    for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		if(*user->username && !Q_stricmp(user->username,login))
		{
			id = i;
			break;
		}
    }
    if(id < 0){
	return id;
    }
    user = &auth_admins.admins[id];
    
    Com_sprintf(hstring, sizeof(hstring), "%s.%s", password, user->salt);

    sha256 = Com_SHA256(hstring);

    if(Q_strncmp(user->sha256, sha256, 128))
	return -1;

    return id;
}


int Auth_GetUID(const char *name){
    int i;
    authData_admin_t *user;
    int uid = -1;
    
	if(name[0] == '@' && isInteger(&name[1], 0) && atoi(&name[1]) > 0)
	{
		uid = atoi(&name[1]);
		
		for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
			if(user->uid == uid)
			{
				return uid;
			}
		}
		return -1;
	}
	
    for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		if(*user->username && !Q_stricmp(user->username,name))
			uid = user->uid;
    }
    return uid;
}


/*
 ============
 SV_SetAdmin_f
 ============
 */

static void Auth_SetAdmin_f() {
	
    int power, i;
    int uid = 0;
	const char *name;
	char stdname[32];
	authData_admin_t* free = NULL;
	authData_admin_t* user;
	
    power = atoi(Cmd_Argv(2));
	
    if ( Cmd_Argc() != 3 || power < 1 || power > 100) {
        Com_Printf( "Usage: AdminAddAdmin <user> <power>\n" );
        Com_Printf( "Where user is one of the following: online-playername | online-playerslot | uid\n" );
	Com_Printf( "Where power is one of the following: Any number between 1 and 100\n" );
	Com_Printf( "online-playername can be a fraction of the playername. uid is a number > 0 and gets written with a leading \"@\" character\n" );
        Com_Printf( "Note: This command can also be used to change the power of an admin\n" );
	Com_Printf("^1IMPORTANT: ^7This command is for the high privileged badmin only\n");
	Com_Printf("Don't create non admin accounts (VIP) with a level of 10 or more points\n");
	return;
    }
	
    uid = SV_GetPlayerUIDByHandle(Cmd_Argv(1));
    name = SV_GetPlayerNameByHandle(Cmd_Argv(1));
	
	if(uid < 1)
	{
		Com_Printf("No such player with a valid UID found. Please consider using \"AdminAddAdminWithPassword\" command unless this was a misstake\n");
		return;
	}
	
	NV_ProcessBegin();
	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		
		if(user->uid == uid )
		{
			user->power = power;
			if(name && strlen(name) > 2)
			{
				if(!user->username[0] || !Q_stricmpn(user->username, "uid_", 4))
				{
					Q_strncpyz(user->username, name, sizeof(user->username));
				}
			}
			Com_Printf("Updated an admin with name %s, uid %d and power %d\n", user->username, user->uid, user->power);
			NV_ProcessEnd();
			return;
		}
		
		if(!free && !*user->username )
		{
			free = user;
		}
	}
	
	if(!free){
		Com_Printf("Too many registered admins. Limit is: %d\n", MAX_AUTH_ADMINS);
		return;
	}
	
	if(name && strlen(name) > 2)
	{
		Q_strncpyz(free->username, name, sizeof(free->username));
	}else{
		Com_sprintf(stdname, sizeof(stdname), "uid_%d", uid);
		Q_strncpyz(free->username, stdname, sizeof(free->username));
	}
	
	free->sha256[0] = '\0';
	free->salt[0] = '\0';
	free->power = power;
	free->uid = uid;
	Com_Printf("Added a new admin with name %s, uid %d and power %d\n", free->username, free->uid, free->power);
	NV_ProcessEnd();
}




void Auth_SetAdminWithPassword_f( void ){

	const char* username;
	const char* password;
	const char* sha256;
	byte buff[129];
	char salt[65];
	unsigned long size = sizeof(salt);
	int power, i,uid;
	authData_admin_t* user;
	authData_admin_t* free = NULL;
	mvabuf;


	if(Cmd_Argc() != 4){
		Com_Printf("Usage: AdminAddAdminWithPassword <username> <password> <power>\n", Cmd_Argv(0));
		Com_Printf( "Where username is loginname for this user\n" );
		Com_Printf( "Where password is the initial 6 characters long or longer password for this user which should get changed by the user on first login\n" );		
		Com_Printf( "Where power is one of the following: Any number between 1 and 100\n" );
		Com_Printf( "Note: Use the command \"AdminAddAdmin\" to change the power of an admin\n" );
		Com_Printf("^1IMPORTANT: ^7This command is for the high privileged badmin only\n");
		return;
	}

	username = Cmd_Argv(1);
	password = Cmd_Argv(2);
	power = atoi(Cmd_Argv(3));
	
	if(!username || !*username || !password || strlen(password) < 6 || power < 1 || power > 100){
		Com_Printf("Usage: AdminSetWithPassword <username> <password> <power>\n", Cmd_Argv(0));
		Com_Printf( "Where username is loginname for this user\n" );
		Com_Printf( "Where password is the initial 6 characters long or longer password for this user which should get changed by the user on first login\n" );		
		Com_Printf( "Where power is one of the following: Any number between 1 and 100\n" );
		Com_Printf("^1IMPORTANT: ^7This command is for the high privileged badmin only\n");
		return;
	}

	NV_ProcessBegin();
	
	uid = ++auth_admins.maxUID;
	    
		
	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++)
	{

		if(!Q_stricmp(user->username, username)){
			Com_Printf("An admin with this username is already registered\n");
			return;
		}

		if(!free && user->uid < 1)
		{
			free = user;
		}
	}
	
	if(!free)
	{
		Com_Printf("Too many registered admins. Limit is: %d\n", MAX_AUTH_ADMINS);
		return;
	}

	Com_RandomBytes(buff, sizeof(buff));
	Sec_HashMemory(SEC_HASH_SHA256,buff,sizeof(buff),salt,&size,qfalse);


	sha256 = Com_SHA256(va("%s.%s", password, salt));

	Q_strncpyz(free->username, username, sizeof(free->username));
	Q_strncpyz(free->sha256, sha256, sizeof(free->sha256));
	Q_strncpyz(free->salt, (char*)salt, sizeof(free->salt));
	free->power = power;
	free->uid = uid;
	Com_Printf("Registered user with Name: %s Power: %d UID: %d\n", free->username, power, uid);
	NV_ProcessEnd();
}


void Auth_UnsetAdmin_f( void ){

	int i, uid;
	authData_admin_t* user;

	if(Cmd_Argc() != 2){
		Com_Printf("Usage: AdminRemoveAdmin <user>\n");
		Com_Printf("Where user is one of the following: name of admin | uid\n" );
		Com_Printf("Name has to be the full known admin name. uid is a number > 0 and gets written with a leading \"@\" character\n" );
		Com_Printf("Note: Use the command \"AdminListAdmins\" to get a list of known admins\n");		
		Com_Printf("^1IMPORTANT: ^7This command is for the high privileged badmin only\n");
		return;
	}
    
	uid = Auth_GetUID(Cmd_Argv(1));
    if(uid < 0){
		Com_Printf("Admin %s not found.\n", Cmd_Argv(1));
		return;
    }

	NV_ProcessBegin();

	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){

		if(user->uid == uid)
		{
			Com_Printf("Removed %s from the list of admins\n", user->username);
			Com_Memset(user, 0, sizeof(authData_admin_t));
			NV_ProcessEnd();
			return;
		}
	}
	Com_Printf("Admin %s not found. This should not happen\n", Cmd_Argv(1));
	NV_ProcessEnd();

}


void Auth_ListAdmins_f( void ){

	int i;
	authData_admin_t* user;

	Com_Printf("------- BAdmins: -------\n");
	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){
		if(*user->username)
			Com_Printf("  %2d:   Name: %s, Power: %d, UID: @%d\n", i+1, user->username, user->power, user->uid);
	}
	Com_Printf("---------------------------------\n");
}

authData_admin_t* Auth_GetAdminFromIndex( int index )
{

	if(index >= MAX_AUTH_ADMINS)
	{
		return NULL;
	}
	return &auth_admins.admins[index];

}


void Auth_ChangeAdminPassword( int uid, const char* password ){

	const char* sha256;
	byte buff[129];
	char salt[65];
	unsigned long size = sizeof(salt);
	authData_admin_t *user, *user2;
	int i;
	//int uid = -1;
	mvabuf;

	
	if(!password || strlen(password) < 6){
		Com_Printf("Error: the new password must have at least 6 characters\n");
		return;
	}

	NV_ProcessBegin();
	
	for(i = 0, user2 = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user2++){
		if(*user2->username && user2->uid == uid){
		    user = user2;
		}
	}
	if(user == NULL){
	    Com_Printf("Error: unknown admin @%d!\n",uid);
	    return;
	}


	Com_RandomBytes(buff, sizeof(buff));

	Sec_HashMemory(SEC_HASH_SHA256,buff,sizeof(buff),salt,&size,qfalse);

	sha256 = Com_SHA256(va("%s.%s", password, salt));

	Q_strncpyz(user->sha256, sha256, sizeof(user->sha256));
	Q_strncpyz(user->salt, (char *)salt, sizeof(user->salt));

	NV_ProcessEnd();

	Com_Printf("Password changed\n");
}


void Auth_ChangePasswordByMasterAdmin_f()
{
    int uid;
    
    if(Cmd_Argc()!= 3){
		Com_Printf("Usage: AdminChangePassword <user> <newPassword>\n");
		Com_Printf("Where user is one of the following: name of admin | uid\n" );
		Com_Printf("Name has to be the full known admin name. uid is a number > 0 and gets written with a leading \"@\" character\n" );
		Com_Printf("Note: Use the command \"AdminListAdmins\" to get a list of known admins\n");		
		Com_Printf("^1IMPORTANT: ^7This command is for the high privileged badmin only\n");
		return;
    }
    
    uid = Auth_GetUID(Cmd_Argv(1));
    if(uid < 0){
		Com_Printf("Admin %s not found.\n",Cmd_Argv(1));
		return;
    }
    Auth_ChangeAdminPassword( uid, Cmd_Argv(2) );
}

void Auth_ChangeOwnPassword_f()
{	
	const char* name;
	const char* password;
	int uid, id;
	
	
	if(Cmd_Argc()!= 3){
		Com_Printf("Usage: ChangePassword <oldPassword> <newPassword>\n");
		Com_Printf("Use this command to change your current password to a new one\n");
		return;
    }

	uid = Cmd_GetInvokerUID();
    
	if(uid < 1)
    {
        Com_Printf("This command can not be used from this place\nYou have no account it seems\n");
		return;
    }
	
	password = Cmd_Argv(1);
	name = Auth_GetNameByUID(uid);
	
	if(name == NULL)
	{
        Com_Printf("Error: No such name has been found. It seems like you aren't an admin.\n");
		return;		
	}
	id = Auth_Authorize(name, password);
	if(id < 0 || id > MAX_AUTH_ADMINS)
	{
		Com_Printf("Error: Your old password doesn't match\n");
		return;
	}
	Auth_ChangeAdminPassword( uid, Cmd_Argv(2) );

}

void Auth_ClearAdminList( )
{
    Com_Memset(auth_admins.admins, 0, sizeof(auth_admins.admins));
    auth_admins.maxUID = 300000000;
}

qboolean Auth_AddAdminToList(const char* username, const char* password, const char* salt, int power, int uid){

	authData_admin_t* user;
	authData_admin_t* free = NULL;
	int i;

	if(uid < 0 || !username || !*username || power < 0 || power > 100 /*!password || strlen(password) < 6 || !salt || strlen(salt) != 64*/)
		return qfalse;

	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++){

		if(user->uid == uid)
		{
			if(username[0])
			{
				if(!user->username[0] || !Q_strncmp(user->username, "uid_", 4))
				{
					Q_strncpyz(user->username, username, sizeof(user->username));	
				}
			}
			
			if(salt && salt[0] && password && password[0])
			{
				Q_strncpyz(user->sha256, password, sizeof(user->sha256));
				Q_strncpyz(user->salt, salt, sizeof(user->salt));	
			}
			
			if(power > 0 && power > user->power)
			{
				user->power = power;
			}
			
			return qtrue;
		}
		if(!Q_stricmp(user->username, username)){
			return qfalse;
		}

		if(!free && user->uid < 1){
			free = user;
			break; // We don't need to go to the end if we have already found an empty spot
		}
	}
	if(!free)
		return qfalse;

	Q_strncpyz(free->username, username, sizeof(free->username));
	Q_strncpyz(free->sha256, password, sizeof(free->sha256));
	Q_strncpyz(free->salt, salt, sizeof(free->salt));
	free->uid = uid;
	if(power > 0)
	{
		free->power = power;
	}
	if(uid > auth_admins.maxUID && uid < 400000000)
	    auth_admins.maxUID = uid;
	
	return qtrue;
}

static void Auth_Login_f(){
    
    client_t *invoker;
    int id,clientNum;
    
    
    if(Cmd_Argc() != 3){
		Com_Printf("Usage: %s <loginname> <password>\n",Cmd_Argv(0));
		return;
    }

    clientNum = Cmd_GetInvokerClnum();
    if(clientNum < 0)
    {
        Com_Printf("This command can only be used from the ingame adminsystem\n");
		return;
    }
    if(clientNum < 0 || clientNum > 63){
		Com_Error(ERR_FATAL,"Auth_Login_f: index out of bounds.\n");
		return;
    }

    if(Cmd_GetInvokerUID() > 0)
    {
        Com_Printf("You have already an user id. You can not use this command (twice)\n");
        return;
    }

    invoker = &svs.clients[clientNum];

    id = Auth_Authorize(Cmd_Argv(1),Cmd_Argv(2));
    if(id < 0 || id > MAX_AUTH_ADMINS){
		//Com_PrintLogFile("Failed login attempt from slot %d with login %s. Client dropped.",clientNum,Cmd_Argv(1));
		SV_DropClient(invoker,"Incorrect login credentials.\n");
		return;
    }

    invoker->uid = auth_admins.admins[id].uid;
    invoker->power = auth_admins.admins[id].power;;
    Com_Printf("^2Successfully authorized. UID: %d, name: %s, power: %d\n",
			   auth_admins.admins[id].uid, auth_admins.admins[id].username, invoker->power);
}



/*
============
Auth_SetCommandPower_f
Changes minimum-PowerLevel of a command
============
*/

static void Auth_SetCommandPower_f() {

    const char* command;
    int power;


    if ( Cmd_Argc() != 3 || atoi(Cmd_Argv(2)) < 1 || atoi(Cmd_Argv(2)) > 100) {
		Com_Printf( "Usage: AdminChangeCommandPower <command> <minpower>\n" );
		Com_Printf( "Where power is one of the following: Any number between 1 and 100\n" );
		Com_Printf( "Where command is any command you can invoke from console / rcon but no cvars\n" );
		return;
    }

    NV_ProcessBegin();

    command = Cmd_Argv(1);
    power = atoi(Cmd_Argv(2));

    if(Cmd_SetPower(command, power))
    {
        Com_Printf("changed required power of cmd: %s to new power: %i\n", command, power);
    }else{
        Com_Printf("Failed to change power of cmd: %s Maybe this is not a valid command.\n", command);
    }
    NV_ProcessEnd();

}




void Auth_Init()
{

	static qboolean	initialized;

	if ( initialized ) {
		return;
	}
	initialized = qtrue;
	
	Auth_ClearAdminList();

	Cmd_AddPCommand("AdminRemoveAdmin", Auth_UnsetAdmin_f, 95);
	Cmd_AddPCommand("AdminAddAdmin", Auth_SetAdmin_f, 95);
	Cmd_AddPCommand("AdminAddAdminWithPassword", Auth_SetAdminWithPassword_f, 95);
	Cmd_AddPCommand("AdminListAdmins", Auth_ListAdmins_f, 80);
	Cmd_AddPCommand("AdminChangePassword", Auth_ChangePasswordByMasterAdmin_f, 95);
	Cmd_AddPCommand("AdminChangeCommandPower", Auth_SetCommandPower_f, 98);
	Cmd_AddPCommand("Login", Auth_Login_f, 1);
	Cmd_AddPCommand("ChangePassword", Auth_ChangeOwnPassword_f, 10);
}


void Auth_WriteAdminConfig(char* buffer, int size)
{
    char infostring[MAX_INFO_STRING];
    int i;
    authData_admin_t *admin;
	mvabuf;

	
    Q_strcat(buffer, size, "\n//Admins authorization data\n");

    for ( admin = auth_admins.admins, i = 0; i < MAX_AUTH_ADMINS ; admin++, i++ ){

        *infostring = 0;

	if(!*admin->username)
		continue;

        Info_SetValueForKey(infostring, "type", "authAdmin");
        Info_SetValueForKey(infostring, "power", va("%i", admin->power));
        Info_SetValueForKey(infostring, "uid", va("%i", admin->uid));
        Info_SetValueForKey(infostring, "password", admin->sha256);
        Info_SetValueForKey(infostring, "salt", admin->salt);
        Info_SetValueForKey(infostring, "username", admin->username);
        Q_strcat(buffer, size, infostring);
        Q_strcat(buffer, size, "\\\n");
    }
}

qboolean Auth_InfoAddAdmin(const char* line)
{
        char password[65];
        char salt[65];
        char username[32];
        int power;
        int uid;

        power = atoi(Info_ValueForKey(line, "power"));
        uid = atoi(Info_ValueForKey(line, "uid"));
        Q_strncpyz(password, Info_ValueForKey(line, "password") , sizeof(password));
        Q_strncpyz(salt, Info_ValueForKey(line, "salt") , sizeof(salt));
        Q_strncpyz(username, Info_ValueForKey(line, "username") , sizeof(username));

        if(!Auth_AddAdminToList(username, password, salt, power,uid)){
            Com_Printf("Error: duplicated username or bad power or too many admins\n");
            return qfalse;
        }
        return qtrue;
}
/* For compatibility */
qboolean SV_RemoteCmdInfoAddAdmin(const char* infostring)
{
	char stdname[32];
	int uid;
	int power;
	
	uid = atoi(Info_ValueForKey(infostring, "uid"));
	power = atoi(Info_ValueForKey(infostring, "power"));
	
	if(uid < 1)
	{
		Com_Printf("^1WARNING: Read invalid uid from admin config\n");
		return qfalse;
	}

	
	Com_sprintf(stdname, sizeof(stdname), "uid_%d", uid);
	
	if(!Auth_AddAdminToList(stdname, "", "", power,uid)){
		Com_Printf("Error: duplicated username or bad power or too many admins\n");
		return qfalse;
	}
	return qtrue;	
	
}


/*
 ============
 SV_RemoteCmdGetClPower
 ============
 */

int Auth_GetClPowerByUID(int uid){

	int i;
	authData_admin_t *user;

	if(uid < 1) return 1;

	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++)
	{
		if(user->uid == uid)
		{
			return user->power;
		}
	}
	return 1;
}


/*
 ============
 SV_RemoteCmdGetClPower
 ============
 */

int Auth_GetClPower(client_t* cl){

	if(cl->uid < 1) return 1;
	if(cl->power > 1) return cl->power;

	return Auth_GetClPowerByUID(cl->uid);
}

/*
 ============
 Auth_GetNameByUID
 ============
 */

const char* Auth_GetNameByUID(int uid){
	
	int i;
	authData_admin_t *user;
	
	if(uid < 1) return NULL;
	
	for(i = 0, user = auth_admins.admins; i < MAX_AUTH_ADMINS; i++, user++)
	{
		if(user->uid == uid)
		{
			return user->username;
		}
	}
	return NULL;
}
	
	
	
/*
void Auth_ClearAdminList()
{

    adminPower_t *admin, **this;
	
    for ( this = &adminpower, admin = *this; admin ; admin = *this ){
        *this = admin->next;
        Z_Free(admin);
    }
	
}


qboolean SV_RemoteCmdAddAdmin(int uid, char* guid, int power)
{
	adminPower_t *admin;
	
	if(uid < 1){
		Com_Printf("Error: Invalid uid\n");
		return qfalse;
	}
	
	if(power < 1 || power > 100){
		Com_Printf("Error: Invalid powerlevel(%i). Powerlevel can not be less than 1 or greater than 100.\n");
		return qfalse;
	}
	
	admin = Z_Malloc(sizeof(adminPower_t));
	
	if(admin)
	{
		admin->uid = uid;
		admin->power = power;
		admin->next = adminpower;
		adminpower = admin;
		return qtrue;
		
	}else{
		return qfalse;
	}
}
*/


