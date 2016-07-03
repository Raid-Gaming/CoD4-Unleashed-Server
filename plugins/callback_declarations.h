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

    // ----------------------------------------------------------------------------//
    // Callbacks available for use in plugin API, sorted alphabetically by module. //
    // ----------------------------------------------------------------------------//

	PCL void OnInfoRequest(pluginInfo_t *info);
	PCL int OnInit();
	PCL void OnMessageSent(char* message, int slot, qboolean *show, int mode);
	PCL void OnPreFastRestart();
	PCL void OnExitLevel();
	PCL void OnPostFastRestart();
	PCL void OnSpawnServer();
	PCL void OnFrame();
	PCL void OnOneSecond();
	PCL void OnTenSeconds();
	PCL void OnUdpNetEvent(netadr_t* from, void* data, int size, qboolean* returnNow);
	PCL void OnUdpNetSend(netadr_t* to, void* data, int len, qboolean* returnNow);
	PCL void OnPlayerConnect(int clientnum, netadr_t* netaddress, char* pbguid, char* userinfo, int authstatus, char* deniedmsg,  int deniedmsgbufmaxlen);
	PCL void OnPlayerConnectAuthFail(netadr_t* netaddress, char* pbguid, char* userinfo, int* authstatus, qboolean *denied);
	PCL void OnPlayerDC(client_t* client, const char* reason);
	PCL void OnClientSpawn(gentity_t* ent);
	PCL void OnClientEnterWorld(client_t* client);
	PCL void OnClientUserinfoChanged(client_t* client);
	PCL void OnClientMoveCommand(client_t* client, usercmd_t* ucmd);
	PCL void OnPlayerWantReservedSlot(netadr_t* from, char* pbguid, char* userinfo, int authstate, qboolean *isallowed);
