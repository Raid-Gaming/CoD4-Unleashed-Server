/*
 *  webadmin.h
 *  CoD4U_testing
 *
 *  Created by Florian on 5/23/14.
 *  Copyright 2014 Dorg. All rights reserved.
 *
 */

#include "cmd.h"
#include "filesystem.h"
#include "g_shared.h"
#include "g_sv_shared.h"
#include "httpftp.h"
#include "msg.h"
#include "q_shared.h"
#include "qcommon_io.h"
#include "qcommon_mem.h"
#include "server.h"
#include "sv_auth.h"

#include <string.h>

qboolean HTTPCreateWebadminMessage(ftRequest_t* request, msg_t* msg,
                                   char* sessionkey, httpPostVals_t* values);
