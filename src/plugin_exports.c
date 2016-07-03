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



#include "plugin_handler.h"

/*=========================================*
 *                                         *
 *        Plugin Handler's exports         *
 *                                         *
 *   Functions in this file are available  *
 *           for use in plugins.           *
 *                                         *
 *=========================================*/

P_P_F void *Plugin_Malloc(size_t size)
{
    volatile int pID;
    //Identify the calling plugin
    pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugins: Error! Tried allocating memory for unknown plugin!\n");
        return NULL;
    }
    if(pluginFunctions.plugins[pID].enabled==qfalse){
        Com_Printf("^1WARNING^7: Tried allocating memory for a disabled plugin!\n");

    }
    return PHandler_Malloc(pID,size);

}
P_P_F void Plugin_Free(void *ptr)
{
    //Identify the calling plugin
    volatile int pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugins: Error! Tried freeing memory for unknown plugin!\n");
        return;
    }
    PHandler_Free(pID,ptr);
}
P_P_F void Plugin_Error(int code, const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    volatile int pID = PHandler_CallerID();

    if(pID<0){
        Com_PrintError("Plugin Error called from unknown plugin!\n");
        return;
    }

    va_start (argptr,fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end (argptr);

    PHandler_Error(pID,code,msg);
}
P_P_F int Plugin_ClientToSlot(client_t *client)
{
    return client-svs.clients;
}

P_P_F clientScoreboard_t Plugin_GetClientScoreboard(int clientNum)
{
    return level.clients[clientNum].pers.scoreboard;
}
P_P_F int Plugin_Cmd_GetInvokerUid()
{
    return Cmd_GetInvokerUID();
}
P_P_F int Plugin_Cmd_GetInvokerSlot()
{
    return Cmd_GetInvokerClnum();
}
P_P_F int Plugin_GetPlayerUid(int slot)
{
    return svs.clients[slot].uid;
}
P_P_F int Plugin_GetSlotCount()
{
    return sv_maxclients->integer;
}
P_P_F qboolean Plugin_IsSvRunning()
{
    if(com_sv_running)
        return com_sv_running->boolean;
    else
        return qfalse;
}
P_P_F char *Plugin_GetPlayerName(int slot)
{
    return svs.clients[slot].name;
}
P_P_F void Plugin_AddCommand(char *name, xcommand_t xcommand, int power)
{
    volatile int pID;
    pID = PHandler_CallerID();
    if(pID>=MAX_PLUGINS){
        Com_PrintError("Tried adding a command for a plugin with non existent pID. pID supplied: %d.\n",pID);
        return;
    }else if(pID<0){
        Com_PrintError("Plugin_AddCommand called from not within a plugin or from a disabled plugin!\n");
        return;
    }
    if(!pluginFunctions.plugins[pID].loaded){
        Com_PrintError("Tried adding a command for not loaded plugin! PID: %d.\n",pID);
    }
    Com_DPrintf("Adding a plugin command for plugin %d, command name: %s.\n",pID,name);
    Cmd_AddCommand(name,PHandler_CmdExecute_f);
    Cmd_SetPower(name, power);
    pluginFunctions.plugins[pID].cmd[pluginFunctions.plugins[pID].cmds].xcommand = xcommand;
    strcpy(pluginFunctions.plugins[pID].cmd[pluginFunctions.plugins[pID].cmds++].name,name);
    Com_DPrintf("Command added.\n");
   // pluginFunctions.plugins[pID].


}
P_P_F qboolean Plugin_TcpConnect( int connection, const char* remote)
{
    int pID;
    //Identify the calling plugin
    pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugins: Error! Tried open a TCP-Connection for unknown plugin!\n");
        return qfalse;
    }
    if(pluginFunctions.plugins[pID].enabled==qfalse){
        Com_Printf("^1WARNING^7: Tried open a TCP-Connection for a disabled plugin!\n");
        return qfalse;
    }
    if(connection >= PLUGIN_MAX_SOCKETS || connection < 0){
        Com_PrintError("Plugin_TcpConnect: Second argument can only be a value inside the range: 0...%d plugin ID: #%d\n", PLUGIN_MAX_SOCKETS);
        return qfalse;
    }
    return PHandler_TcpConnect(pID, remote, connection);

}

P_P_F int Plugin_TcpGetData(int connection, void* buf, int size)
{
    int pID;
    //Identify the calling plugin
    pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugin_TcpGetData: Error! Tried get TCP data for unknown plugin!\n");
        return -1;
    }
    if(pluginFunctions.plugins[pID].enabled==qfalse){
        Com_Printf("^1WARNING^7: Plugin_TcpGetData: Tried get TCP data for a disabled plugin!\n");
        return -1;
    }
    if(connection >= PLUGIN_MAX_SOCKETS || connection < 0){
        Com_PrintError("Plugin_TcpGetData: First argument can only be a value inside the range: 0...%d plugin ID: #%d\n", PLUGIN_MAX_SOCKETS, pID);
        return -1;
    }
    if(buf == NULL){
        Com_PrintError("Plugin_TcpGetData: Third argument can not be a NULL-Pointer for plugin ID: #%d\n", pID);
        return -1;
    }

    return PHandler_TcpGetData(pID, connection, buf, size);
}


P_P_F qboolean Plugin_TcpSendData(int connection, void* data, int len)
{


    int pID;
    //Identify the calling plugin
    pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugin_TcpSendData: Error! Tried get TCP data for unknown plugin!\n");
        return qfalse;
    }
    if(pluginFunctions.plugins[pID].enabled==qfalse){
        Com_Printf("^1WARNING^7: Plugin_TcpSendData: Tried get TCP data for a disabled plugin!\n");
        return qfalse;
    }
    if(connection >= PLUGIN_MAX_SOCKETS || connection < 0){
        Com_PrintError("Plugin_TcpSendData: Second argument can only be a value inside the range: 0...%d plugin ID: #%d\n", PLUGIN_MAX_SOCKETS, pID);
        return qfalse;
    }
    if(data == NULL){
        Com_PrintError("Plugin_TcpSendData: Second argument can not be a NULL-Pointer for plugin ID: #%d\n", pID);
        return qfalse;
    }

    return PHandler_TcpSendData(pID, connection, data, len);
}

P_P_F void Plugin_TcpCloseConnection(int connection)
{
    int pID;
    //Identify the calling plugin
    pID = PHandler_CallerID();
    if(pID<0){
        Com_Printf("Plugin_TcpCloseConnection: Error! Tried get close a connection for unknown plugin!\n");
        return;
    }
    if(pluginFunctions.plugins[pID].enabled==qfalse){
        Com_Printf("^1WARNING^7: Plugin_TcpCloseConnection: Tried to close a connection for a disabled plugin!\n");
        return;
    }
    if(connection >= PLUGIN_MAX_SOCKETS || connection < 0){
        Com_PrintError("Plugin_TcpCloseConnection: Second argument can only be a value inside the range: 0...%d plugin ID: #%d\n", PLUGIN_MAX_SOCKETS, pID);
        return;
    }
    PHandler_TcpCloseConnection(pID, connection);
}

P_P_F qboolean Plugin_UdpSendData(netadr_t* to, void* data, int len)
{
    int pID;

    if(to == NULL){
        pID = PHandler_CallerID();
        Com_PrintError("Plugin_UdpSendData: First argument can not be a NULL-Pointer for plugin ID: #%d\n", pID);
        return qfalse;
    }

    if(data == NULL){
        pID = PHandler_CallerID();
        Com_PrintError("Plugin_UdpSendData: First argument can not be a NULL-Pointer for plugin ID: #%d\n", pID);
        return qfalse;
    }
    return Sys_SendPacket( len, data, to);
}

P_P_F void Plugin_ServerPacketEvent(netadr_t* to, void* data, int len)
{
    msg_t msg;
    msg.data = data;
    msg.cursize = len;
    msg.maxsize = len;
    msg.bit = 0;
    msg.overflowed = qfalse;
    msg.readonly = qtrue;

    SV_PacketEvent( to, &msg );
}

P_P_F void Plugin_SetPlayerUID(unsigned int clientslot, unsigned int uid)
{
    client_t *cl;
    int PID = PHandler_CallerID();
	mvabuf;

    if(clientslot > sv_maxclients->integer)
    {
        PHandler_Error(PID,P_ERROR_DISABLE, va("Plugin tried to set UID for bad client: %d\n", clientslot));
    }
    cl = &svs.clients[clientslot];
    cl->uid = uid;
}

P_P_F unsigned int Plugin_GetPlayerUID(unsigned int clientslot)
{
    client_t *cl;
    int PID = PHandler_CallerID();
	mvabuf;

	
    if(clientslot > sv_maxclients->integer)
    {
        PHandler_Error(PID,P_ERROR_DISABLE, va("Plugin tried to get UID for bad client: %d\n", clientslot));
    }
    cl = &svs.clients[clientslot];
    return cl->uid;
}

P_P_F const char* Plugin_GetPlayerGUID(unsigned int clientslot)
{
    client_t *cl;
    int PID = PHandler_CallerID();
	mvabuf;
	
    if(clientslot > sv_maxclients->integer)
    {
        PHandler_Error(PID,P_ERROR_DISABLE, va("Plugin tried to get GUID for bad client: %d\n", clientslot));
    }
    cl = &svs.clients[clientslot];
    return cl->pbguid;
}

P_P_F void Plugin_SetPlayerGUID(unsigned int clientslot, const char* guid)
{
    client_t *cl;
    int PID = PHandler_CallerID();
	mvabuf;

    if(clientslot > sv_maxclients->integer)
    {
        PHandler_Error(PID,P_ERROR_DISABLE, va("Plugin tried to get GUID for bad client: %d\n", clientslot));
    }
    cl = &svs.clients[clientslot];
    Q_strncpyz(cl->pbguid, guid, sizeof(cl->pbguid));
}

P_P_F void Plugin_SetPlayerNoPB(unsigned int clientslot)
{
#ifdef PUNKBUSTER
    client_t *cl;
	mvabuf;
	
    int PID = PHandler_CallerID();
    if(clientslot > sv_maxclients->integer)
    {
        PHandler_Error(PID,P_ERROR_DISABLE, va("Plugin tried to set No PunkBuster flag for bad client: %d\n", clientslot));
    }
    cl = &svs.clients[clientslot];
    cl->noPb = qtrue;
#endif
}

P_P_F int Plugin_DoesServerUseUids(void)
{
    return psvs.useuids;
}

P_P_F void Plugin_SetServerToUseUids(int useuids)
{
    psvs.useuids = useuids;
}

P_P_F int Plugin_GetLevelTime(void)
{
    return level.time;
}

P_P_F int Plugin_GetServerTime(void)
{
    return svs.time;
}

P_P_F void Plugin_ScrAddFunction(char *name, xfunction_t function)
{
    int PID = PHandler_CallerID();

    PHandler_ScrAddFunction(name, function, qfalse, PID);
}

P_P_F void Plugin_ScrAddMethod(char *name, xfunction_t function)
{
    int PID = PHandler_CallerID();

    PHandler_ScrAddMethod(name, function, qfalse, PID);
}

P_P_F void Plugin_ScrReplaceFunction(char *name, xfunction_t function)
{
    int PID = PHandler_CallerID();

    PHandler_ScrAddFunction(name, function, qtrue, PID);
}

P_P_F void Plugin_ScrReplaceMethod(char *name, xfunction_t function)
{
    int PID = PHandler_CallerID();

    PHandler_ScrAddMethod(name, function, qtrue, PID);
}


P_P_F void Plugin_ChatPrintf(int slot, const char *fmt, ... )
{
    //int ebp,*ptr;
    char str[256];
    client_t *cl;
    va_list vl;

    if(slot < 0)
        cl = NULL;
    else if( slot < sv_maxclients->integer)
        cl = &svs.clients[slot];
    else
        return;

    va_start(vl,fmt);
    Q_vsnprintf(str, sizeof(str), fmt,vl);
    va_end(vl);
    SV_SendServerCommand(cl, "h \"%s\"", str);
}

P_P_F void Plugin_BoldPrintf(int slot, const char *fmt, ... )
{
    char str[256];
    client_t *cl;
    va_list vl;

    if(slot < 0)
        cl = NULL;
    else if( slot < sv_maxclients->integer)
        cl = &svs.clients[slot];
    else
        return;

    va_start(vl,fmt);
    Q_vsnprintf(str, sizeof(str), fmt, vl);
    va_end(vl);
    SV_SendServerCommand(cl, "c \"%s\"", str);
}


P_P_F int Plugin_Cvar_GetInteger(void *cvar)
{
    cvar_t* var = cvar;
    int PID = PHandler_CallerID();

    if(var == NULL)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin to get Cvar of NULL-Pointer\n");
        return 0;
    }

    if(var->type != CVAR_INT)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin tried to get Cvar of different type\n");
        return 0;
    }

    return var->integer;
}

P_P_F qboolean Plugin_Cvar_GetBoolean(void *cvar)
{
    cvar_t* var = cvar;
    int PID = PHandler_CallerID();

    if(var == NULL)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin to get Cvar of NULL-Pointer\n");
        return 0;
    }

    if(var->type != CVAR_BOOL)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin tried to get Cvar of different type\n");
        return 0;
    }
    return var->boolean;
}

P_P_F float Plugin_Cvar_GetValue(void *cvar)
{
    cvar_t* var = cvar;
    int PID = PHandler_CallerID();

    if(var == NULL)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin to get Cvar of NULL-Pointer\n");
        return 0;
    }

    if(var->type != CVAR_FLOAT)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin tried to get Cvar of different type\n");
        return 0;
    }

    return var->value;
}

P_P_F const char* Plugin_Cvar_GetString(void *cvar)
{
    cvar_t* var = cvar;
    int PID = PHandler_CallerID();

    if(var == NULL)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin to get Cvar of NULL-Pointer\n");
        return 0;
    }

    if(var->type != CVAR_STRING)
    {
        PHandler_Error(PID, P_ERROR_DISABLE, "Plugin tried to get Cvar of different type\n");
        return 0;
    }

    return var->string;
}

P_P_F void Plugin_DropClient( unsigned int clientnum, const char *reason )
{
    if(clientnum > sv_maxclients->integer)
		return;

	SV_DropClient(&svs.clients[clientnum], reason);
}



P_P_F void Plugin_BanClient( unsigned int clientnum, int duration, int invokerid, char *banreason )
{
    
	client_t *cl;
	char* guid;
	time_t expire;
	char* temp;
    time_t aclock;
	char endtime[32];
    char dropmsg[MAX_STRING_CHARS];

	if(clientnum > sv_maxclients->integer)
		return;
	
	cl = &svs.clients[clientnum];

	time(&aclock);
	
	if(duration == -1)
	{
		expire = duration;
		Q_strncpyz(endtime, "never", sizeof(endtime));
	}
	else
	{
		expire = (aclock+(time_t)(duration*60));
		temp = ctime(&expire);
		temp[strlen(temp)-1] = 0;
		Q_strncpyz(endtime, temp, sizeof(endtime));
	
	}
	
	if(strlen(cl->pbguid) == 32)
	{
		guid = &cl->pbguid[24];
	}
	else if(cl->uid < 1)
	{
		Com_Printf("Error: This player has no valid ID and got banned by IP only\n");
		SV_DropClient(cl, "Invalid ID\n");
		SV_PlayerAddBanByip(&cl->netchan.remoteAddress, "INVALID USER", 0, "INVALID", 0, expire);
		return;
	}
	
	if(banreason == NULL)
	{
		banreason = "N/A";
	}
	
	SV_AddBan(cl->uid, invokerid, guid, cl->name, expire, banreason);

	if( cl->uid > 0 )
	{
		Com_Printf( "Banrecord added for player: %s uid: %i\n", cl->name, cl->uid);
		SV_PrintAdministrativeLog( "Banned player: %s uid: %i until %s with the following reason: %s", cl->name, cl->uid, endtime, banreason);
		Com_sprintf(dropmsg, sizeof(dropmsg), "You have been banned from this server\nYour ban will expire on: %s\nYour UID is: %i    Banning admin UID is: %i\nReason for this ban:\n%s",
			endtime, cl->uid, invokerid, banreason);

	}else{
		Com_Printf( "Banrecord added for player: %s guid: %s\n", cl->name, cl->pbguid);
		SV_PrintAdministrativeLog( "Banned player: %s guid: %s until %s with the following reason: %s", cl->name, cl->pbguid, endtime, banreason);
		Com_sprintf(dropmsg, sizeof(dropmsg), "You have been banned from this server\nYour ban will expire on: %s\nYour GUID is: %s    Banning admin UID is: %i\nReason for this ban:\n%s",
			endtime, cl->pbguid, invokerid, banreason);

		if(cl->authentication < 1)
		{
			SV_PlayerAddBanByip(&cl->netchan.remoteAddress, banreason, 0, cl->pbguid, 0, expire);
		}
	}
	SV_DropClient(cl, dropmsg);
}

P_P_F gentity_t* Plugin_GetGentityForEntityNum(int entnum)
{
    return &g_entities[entnum];
}

P_P_F client_t* Plugin_GetClientForClientNum(int clientnum)
{
    return &svs.clients[clientnum];
}

P_P_F int Plugin_FS_SV_WriteFile( const char *qpath, const void *buffer, int size)
{
    return FS_SV_HomeWriteFile( qpath, buffer, size);
}
