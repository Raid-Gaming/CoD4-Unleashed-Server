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
#include "sys_net.h"
#include "msg.h"

#ifndef __SV_AUTH_H__
#define __SV_AUTH_H__


#define MAX_AUTH_ADMINS 512


typedef struct{
	char username[32];
	char salt[129];
	char sha256[65];
	char sessionid[65];
	int power;
	int uid;
}authData_admin_t; // size: 32 + 129 + 65 = 289 B

typedef struct{
	authData_admin_t admins[MAX_AUTH_ADMINS];
	int maxUID;
}authData_t; // size: MAX_AUTH_ADMINS * 289 = 512 * 289 = 147.968 ~= 148kB


void Auth_ChangeAdminPassword( int uid, const char *password );
int Auth_Authorize(const char *login, const char *password);
void Auth_WipeSessionId(const char *username);


qboolean Auth_AddAdminToList(const char* username, const char* password, const char* salt, int power, int uid);
void Auth_ClearAdminList( void );

void Auth_Init();
int Auth_GetUID(const char* username);
const char* Auth_GetNameByUID( int uid );
int Auth_GetClPowerByUID(int uid);
authData_admin_t* Auth_GetAdminFromIndex( int index );


qboolean Auth_InfoAddAdmin(const char* line);
void Auth_WriteAdminConfig(char* buffer, int size);

const char* Auth_FindSessionID(const char* sessionid);
const char* Auth_GetSessionId(const char* username, const char *password);

#endif