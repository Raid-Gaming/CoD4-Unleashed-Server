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

#include "g_shared.h"
#include "g_sv_shared.h"
#include "hl2rcon.h"
#include "misc.h"
#include "player.h"
#include "q_shared.h"
#include "scr_vm.h"

__cdecl void ClientScr_SetSessionTeam(gclient_t* gcl, client_fields_t* gfl) {

  short index;
  int cid;
  mvabuf;

  if ((void*)gcl - (void*)level.clients >= MAX_CLIENTS * sizeof(gclient_t)) {
    Scr_Error("Client is not pointing to the level.clients array.");
    return;
  }

  index = Scr_GetConstString(0);

  if (index == stringIndex.axis)
    gcl->sess.sessionTeam = TEAM_RED;
  else if (index == stringIndex.allies)
    gcl->sess.sessionTeam = TEAM_BLUE;
  else if (index == stringIndex.spectator)
    gcl->sess.sessionTeam = TEAM_SPECTATOR;

  else if (index == stringIndex.none)
    gcl->sess.sessionTeam = TEAM_FREE;

  else {
    Scr_Error(va("'%s' is an illegal sessionteam string. Must be allies, axis, "
                 "none, or spectator.",
                 SL_ConvertToString(index)));
    return;
  }

  cid = gcl - level.clients;

  ClientUserinfoChanged(cid);

  HL2Rcon_EventClientEnterTeam(cid, gcl->sess.sessionTeam);
}
