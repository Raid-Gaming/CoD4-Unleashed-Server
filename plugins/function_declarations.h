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
    // Functions available for use in plugin API, sorted alphabetically by module. //
    // ----------------------------------------------------------------------------//

    //      == Commands ==
    
    __cdecl char* Plugin_Cmd_Argv(int arg);                // Get a command argument with index arg.
    __cdecl int Plugin_Cmd_Argc();                         // Get number of command arguments
    __cdecl char *Plugin_Cmd_Args( char* buff, int bufsize );

    //      == Common ==
    __cdecl void Plugin_G_LogPrintf( const char *fmt, ... );
    __cdecl void Plugin_Printf( const char *fmt, ...);                // Print to a correct place (rcon, player console, logs)
    __cdecl void Plugin_PrintWarning( const char *fmt, ...);          // Print to a correct place (rcon, player console, logs)
    __cdecl void Plugin_PrintError( const char *fmt, ...);            // Print to a correct place (rcon, player console, logs)
    __cdecl void Plugin_DPrintf( const char *fmt, ...);               // Same as Com_Printf, only shows up when developer is set to 1
    __cdecl char* Plugin_ParseGetToken(char* line);                 // Tokenize a string - get next token
    __cdecl int Plugin_ParseTokenLength(char* token);               // Tokenize a string - get the token's length
    __cdecl void Plugin_ParseReset(void);               			// Tokenize a string - Reset the parsers position
    __cdecl void Plugin_Cbuf_AddText(const char* text);
    
    //      == Cvars ==
    
    // All of the Cvars module functions are self explanatory
    __cdecl CONVAR_T* Plugin_Cvar_RegisterString(const char *var_name, const char *var_value, int flags, const char *var_description);
    __cdecl CONVAR_T* Plugin_Cvar_RegisterBool(const char *var_name, qboolean var_value, int flags, const char *var_description);
    __cdecl CONVAR_T* Plugin_Cvar_RegisterInt(const char *var_name, int var_value, int min_value, int max_value, int flags, const char *var_description);
//    __cdecl CONVAR_T* Plugin_Cvar_RegisterEnum(const char *var_name, char** valnames, int defaultval, int flags, const char *var_description);
    __cdecl CONVAR_T* Plugin_Cvar_RegisterFloat(const char *var_name, float var_value, float min_value, float max_value, int flags, const char *var_description);
    __cdecl void Plugin_Cvar_SetInt(CONVAR_T const* var, int val);
    __cdecl void Plugin_Cvar_SetBool(CONVAR_T const* var, qboolean val);
    __cdecl void Plugin_Cvar_SetString(CONVAR_T const* var, char const* string);
    __cdecl void Plugin_Cvar_SetFloat(CONVAR_T const* var, float val);
    __cdecl int Plugin_Cvar_GetInteger(CONVAR_T const *var);
    __cdecl qboolean Plugin_Cvar_GetBoolean(CONVAR_T const *var);
    __cdecl float Plugin_Cvar_GetValue(CONVAR_T const *var);
    __cdecl const char* Plugin_Cvar_GetString(CONVAR_T const *var);

    __cdecl void Plugin_Cvar_VariableStringBuffer(const char* cvarname, char* buff, size_t size);
    __cdecl float Plugin_Cvar_VariableValue( const char *var_name );
    __cdecl int Plugin_Cvar_VariableIntegerValue( const char *var_name );
    __cdecl int Plugin_Cvar_VariableBooleanValue( const char *var_name );
    __cdecl const char* Plugin_Cvar_VariableString( const char *var_name );
    // Sets a cvar by name and by a string value which gets interpreted correctly depending on the cvar type
    __cdecl void Plugin_Cvar_Set( const char *var_name, const char* value );


    //      == File handling functions == - Do we really need those?

    __cdecl int Plugin_FS_SV_FOpenFileRead(const char *filename, fileHandle_t *fp); // Open a file for reading
    __cdecl fileHandle_t Plugin_FS_SV_FOpenFileWrite(const char *filename);         // Open a file for writing
    __cdecl int Plugin_FS_Read(void *buffer, int len, fileHandle_t f);              // Read data from file
    __cdecl int Plugin_FS_ReadLine(void *buffer, int len, fileHandle_t f);          // Read a line from file
    __cdecl int Plugin_FS_Write(const void *buffer, int len, fileHandle_t h);       // Write to file
    __cdecl qboolean Plugin_FS_FCloseFile(fileHandle_t f);                          // Cloase an open file

    //Writes the provided buffer into the file named by qpath. This is the most easiest way to write a file
    __cdecl int Plugin_FS_SV_WriteFile( const char *qpath, const void *buffer, int size);


    //      == Networking ==

    __cdecl int Plugin_NET_StringToAdr(const char* string, netadr_t* , netadrtype_t);
    __cdecl qboolean Plugin_NET_CompareAdr (netadr_t *a, netadr_t *b);
    __cdecl qboolean Plugin_NET_CompareBaseAdrMask(netadr_t *a, netadr_t *b, int netmask);
    __cdecl qboolean Plugin_NET_CompareBaseAdr (netadr_t *a, netadr_t *b);
    __cdecl const char *Plugin_NET_AdrToString (netadr_t *a);
    __cdecl const char *Plugin_NET_AdrToStringShort (netadr_t *a);



    //      == Plugin Handler's functions ==

    __cdecl clientScoreboard_t Plugin_GetClientScoreboard(int clientNum);    // Get the scoreboard of a player
    __cdecl int Plugin_Cmd_GetInvokerUid();                                  // Get UID of command invoker
    __cdecl int Plugin_Cmd_GetInvokerSlot();                                 // Get slot number of command invoker
    __cdecl int Plugin_GetPlayerUid(int slot);                               // Get UID of a plyer
    __cdecl int Plugin_GetSlotCount();                                       // Get number of server slots
    __cdecl qboolean Plugin_IsSvRunning();                                   // Is server running?
    __cdecl void Plugin_ChatPrintf(int slot, const char *fmt, ...);          // Print to player's chat (-1 for all)
    __cdecl void Plugin_BoldPrintf(int slot, const char *fmt, ...);          // Print to the player's screen (-1 for all)
    __cdecl char *Plugin_GetPlayerName(int slot);                            // Get a name of a player
    __cdecl void Plugin_AddCommand(char *name, xcommand_t command, int defaultpower); // Add a server command
    __cdecl void *Plugin_Malloc(size_t size);                                // Same as stdlib.h function malloc
    __cdecl void Plugin_Free(void *ptr);                                     // Same as stdlib.h function free
    __cdecl void Plugin_Error(int code, const char *fmt, ...);               // Notify the server of an error, action depends on code parameter
    __cdecl int Plugin_GetLevelTime();                                       // Self explanatory
    __cdecl int Plugin_GetServerTime();                                      // Self explanatory

	//	-- Functions for clients --
	
	__cdecl void Plugin_DropClient( int clientnum, const char *reason );	// Kicks the client from server
	__cdecl void Plugin_BanClient( unsigned int clientnum, int seconds, int invokerid, char *reason ); //Bans the client for seconds from server. Seconds can be "-1" to create a permanent ban. invokerid can be 0 or the numeric uid. banreason can be NULL or a valid char* pointer.

    //  -- TCP Connection functions --
    /* 
    connection is a static constant number. Every plugin can use a connection 0 up to 3. This is not a socket. This is handled internal.
    You can not use the same number for 2 open connections on the same time.
    
    */
    __cdecl qboolean Plugin_TcpConnect(int connection, const char* remote);      // Open a new TCP connection - Returns qfalse if failed, remote can be a domainname
    __cdecl int Plugin_TcpGetData(int connection, void *buf, int size);          // Receive TCP data - buf and size is the receiving buffer. It returns -1 if the connection is closed. It returns 0 when no new data is available. All other return values is the number of bytes received.
    __cdecl qboolean Plugin_TcpSendData(int connection, void *data, int len);    // Send TCP data - buf and len point to the buffer which has the data to send. Len is the amount to bytes to send. Returns qfalse if something has failed.
    __cdecl void Plugin_TcpCloseConnection(int connection);                      // Close an open TCP connection
    __cdecl qboolean Plugin_UdpSendData(netadr_t* to, void* data, int len);      // Send UDP data
    __cdecl void Plugin_ServerPacketEvent(netadr_t* to, void* data, int len);    // Receive UDP data


    //  -- UIDS / GUIDs --

    __cdecl void Plugin_SetPlayerUID(unsigned int clientslot, unsigned int uid);     // Set player's UID
    __cdecl unsigned int Plugin_GetPlayerUID(unsigned int clientslot);               // Get player's UID
    __cdecl const char* Plugin_GetPlayerGUID(unsigned int clientslot);               // Get player's GUID
    __cdecl void Plugin_SetPlayerGUID(unsigned int clientslot, const char* guid);    // Set player's GUID
    __cdecl int Plugin_DoesServerUseUids(void);                                      // Self explanatory
    __cdecl void Plugin_SetServerToUseUids(int useuids);                             // Self explanatory


    //      == System functions ==

    __cdecl int Plugin_Milliseconds();  // Milliseconds since server start
    __cdecl void Plugin_RandomBytes( byte *string, int len );

    //      == Scriptfunctions ==
    __cdecl void Plugin_ScrAddFunction(char *name, void (*function)());
    __cdecl void Plugin_ScrAddMethod(char *name, void (*function)(scr_entref_t object));
    __cdecl void Plugin_ScrReplaceFunction(char *name, xfunction_t function);
    __cdecl void Plugin_ScrReplaceMethod(char *name, xfunction_t function);

    __cdecl void Plugin_Scr_AddEntity(gentity_t* ent);
    __cdecl int Plugin_Scr_GetNumParam( void );
    __cdecl int Plugin_Scr_GetInt( unsigned int );
    __cdecl float Plugin_Scr_GetFloat( unsigned int );
    __cdecl char* Plugin_Scr_GetString( unsigned int );
    __cdecl gentity_t* Plugin_Scr_GetEntity( unsigned int );
    __cdecl short Plugin_Scr_GetConstString( unsigned int );
    __cdecl unsigned int Plugin_Scr_GetType( unsigned int );
    __cdecl void Plugin_Scr_GetVector( unsigned int, vec3_t* );
    __cdecl void Plugin_Scr_Error( const char *string);
    __cdecl void Plugin_Scr_ParamError( int, const char *string);
    __cdecl void Plugin_Scr_ObjectError( const char *string);
    __cdecl void Plugin_Scr_AddInt(int value);
    __cdecl void Plugin_Scr_AddFloat(float);
    __cdecl void Plugin_Scr_AddBool(qboolean);
    __cdecl void Plugin_Scr_AddString(const char *string);
    __cdecl void Plugin_Scr_AddUndefined(void);
    __cdecl void Plugin_Scr_AddVector( vec3_t vec );
    __cdecl void Plugin_Scr_AddArray( void );
    __cdecl void Plugin_Scr_MakeArray( void );
    __cdecl short Plugin_Scr_ExecEntThread( gentity_t* ent, int callbackHook, unsigned int numArgs);
    __cdecl short Plugin_Scr_ExecThread( int callbackHook, unsigned int numArgs);
    __cdecl void Plugin_Scr_FreeThread( short threadId);



    __cdecl void Plugin_Scr_NotifyLevel(int constString, unsigned int numArgs);
    __cdecl void Plugin_Scr_NotifyNum(int entityNum, unsigned int entType, unsigned int constString, unsigned int numArgs);
    __cdecl void Plugin_Scr_Notify(gentity_t* ent, unsigned short constString, unsigned int numArgs);
    __cdecl int Plugin_Scr_AllocString(const char*);



    __cdecl playerState_t *Plugin_SV_GameClientNum( int num ); //Retrives the playerState_t* object from a client number

    __cdecl gentity_t* Plugin_GetGentityForEntityNum(int entnum);
    __cdecl client_t* Plugin_GetClientForClientNum(int clientnum);

    __cdecl const char* Plugin_SL_ConvertToString(int index);

    __cdecl void Plugin_SV_SetConfigstring(int index, const char *text);
    __cdecl void Plugin_SV_GetConfigstring( int index, char *buffer, int bufferSize );
