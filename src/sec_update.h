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
#ifndef SEC_UPDATE_H
#define SEC_UPDATE_H
#include "sec_init.h"
#include "q_platform.h"
#include "version.h"


void Sec_Update( qboolean getbasefiles );


typedef struct sec_file_s{
    char name[MAX_OSPATH];
    char path[MAX_OSPATH];
    int size;
    char hash[65];
    struct sec_file_s *next;
}sec_file_t;


#define SEC_UPDATE_VERSION "1.0"

#define SEC_UPDATE_INITIALBUFFSIZE 10240

#define SEC_TYPE 'e'

#ifdef COD4X17A
    #define SEC_VERSION 1.7
#else
    #define SEC_VERSION 1.8
#endif


//#undef QUOTE
#define SEC_UPDATE_HOST "update.iceops.in"
#define SEC_UPDATE_PHP(B,T) "/?ver=" SEC_VERSION "&build=" B "&type=" T
#define SEC_UPDATE_USER_AGENT "CoD4X AutoUpdater V. " SEC_UPDATE_VERSION
//#define SEC_UPDATE_BOUNDARY "------------------------------------874ryg7v"
#define SEC_UPDATE_PORT 80


#define SEC_UPDATE_DOWNLOAD(baseurl, qpath) "%s%s", baseurl, qpath
#define SEC_UPDATE_GETVERSION "/?ver=%g&os=%s&build=%d&type=%c", SEC_VERSION, OS_STRING, BUILD_NUMBER, SEC_TYPE
#define SEC_UPDATE_GETGROUNDVERSION "/?ver=%g&os=%s&build=%d&type=%c", SEC_VERSION, OS_STRING, 753, 'b'

#if !defined(COD4X17A) || defined(OFFICIAL) || defined(OFFICIALTESTING) || defined(OFFICIALBETA) || defined(OFFICIALDEBUG)
    #define CAN_UPDATE
#endif

#endif
