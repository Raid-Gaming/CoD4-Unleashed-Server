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



#ifndef __SYS_COD4DEFS_H__
#define __SYS_COD4DEFS_H__


#define EXECUTABLE_NAME "cod4x17a_dedrun"

#define PRODUCT_NAME				"CoD4x Dedicated Server"
#define BASEGAME					"main"
#define CLIENT_WINDOW_TITLE     	"CoD4x Dedicated Server"
#define CLIENT_WINDOW_MIN_TITLE 	"CoD4x"

#define PORT_MASTER 20810
#define MASTER_SERVER_NAME "cod4master.activision.com"
#define MASTER_SERVER_NAME2 "cod4master.iceops.co"
#define HEARTBEAT_GAME "COD-4"
#define HEARTBEAT_DEAD "flatline"

#define PRODUCT_VERSION "1.0"

#ifdef COD4X17A
    #define Q3_VERSION "1.7a"
#else
    #define Q3_VERSION "1.8"
#endif

#define GAME_STRING "CoD4x"

#include "version.h"
#ifndef BUILD_NUMBER
#define BUILD_NUMBER -1
#endif

#define MAX_CLIENTS 64
#define MAX_CONFIGSTRINGS 2442

#endif