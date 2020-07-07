/*
===========================================================================
        Copyright (c) 2015-2019 atrX of Raid Gaming
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team
    Copyright (C) 1999-2005 Id Software, Inc.

    This file is part of CoD4-Unleashed-Server source code.

    CoD4-Unleashed-Server source code is free software: you can redistribute it
and/or modify it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    CoD4-Unleashed-Server source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================
*/
#include "sec_update.h"
#include "cmd.h"
#include "cvar.h"
#include "filesystem.h"
#include "httpftp.h"
#include "msg.h"
#include "netchan.h"
#include "sec_common.h"
#include "sec_crypto.h"
#include "sec_main.h"
#include "sys_cod4defs.h"
#include "sys_main.h"
#include "sys_net.h"

#include <errno.h>
#include <unistd.h>

#define BUFSIZE 10240

cvar_t* canupdate;

char* Sec_StrTok(char* str, char* tokens, int id) {
  static char* mem[100] = {NULL};
  char *ptr, *ptr2;
  if (id < 0 || id > 99 || tokens == NULL)
    return NULL;
  if (str == NULL) {
    if (mem[id] == NULL) {
      return NULL;
    }

    for (ptr = mem[id]; *ptr != 0; ++ptr) {
      // printf("---%c\n",*ptr);
      for (ptr2 = tokens; *ptr2 != 0; ++ptr2) {
        // printf("------%c\n",*ptr2);
        if (*ptr == *ptr2) {
          // printf("DEBUG!!!!!!!! %p:\"%s\",
          // %p:\"%s\",%p:\"%s\".\n\n",ptr,ptr,ptr2,ptr2,mem[id],mem[id]);
          *ptr = 0;
          ptr2 = mem[id];
          mem[id] = ptr + 1;
          // printf("DEBUG!!!!!!!! %p:\"%s\",
          // %p:\"%s\".\n\n",ptr,ptr,ptr2,ptr2);
          //__asm__("int $3");
          return ptr2;
        }
      }
    }
    if (ptr != mem[id]) {
      ptr = mem[id];
    } else
      ptr = NULL;
    mem[id] = NULL;
    return ptr;
  } else {
    // printf("Debugging: \"%s\"\n",str);
    // mem[id]=str;
    for (ptr = str; *ptr != 0 && mem[id] == NULL; ++ptr) {
      for (ptr2 = tokens; *ptr2 != 0 && mem[id] == NULL; ++ptr2) {
        if (*ptr != *ptr2) {
          mem[id] = ptr;
        }
      }
    }
    if (mem[id] == NULL)
      return NULL;
    return Sec_StrTok(NULL, tokens, id); // BECAUSE I CAN.
  }
}

void Sec_FreeFileStruct(sec_file_t* file) {
  if (file->next != NULL)
    Sec_FreeFileStruct(file->next);
  Sec_Free(file);
}

// Nulled out
void Sec_Update(qboolean getbasefiles) {}
