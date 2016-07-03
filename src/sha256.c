/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include "sha256.h"
#include "qcommon_io.h"
#include "sec_main.h"
const char* Com_SHA256( const char* string )
{
    static char finalsha[65];
    unsigned long size = sizeof(finalsha);
    if(!Sec_HashMemory(SEC_HASH_SHA256,(void *)string,strlen(string),finalsha,&size,qfalse))
	Com_Printf("Warning: Com_SHA256, error while hashing! Error:%s\n",Sec_CryptErrStr(SecCryptErr));
    /*hash_state md;
    
    
    sha256_desc.init(&md);
    sha256_desc.process(&md, (const unsigned char *)string, strlen(string));
    sha256_desc.done(&md,(unsigned char *)finalsha);
    finalsha[64]=0;*/
    return finalsha;
}