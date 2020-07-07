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
#ifndef SEC_MAIN_H
#define SEC_MAIN_H

#include "../lib_tomcrypt/main.h"
#include "q_shared.h"
#include "qcommon_io.h"
#include "sec_common.h"
#include "sec_crypto.h"
#include "sec_init.h"
extern int SecCryptErr;
char* Sec_CryptErrStr(int);

#endif
