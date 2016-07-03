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
#include "sec_update.h"
#include "sec_crypto.h"
#include "sec_common.h"
#include "sec_main.h"
#include "sys_main.h"
#include "msg.h"
#include "sys_net.h"
#include "netchan.h"
#include "filesystem.h"
#include "cmd.h"
#include "cvar.h"
#include "sys_cod4defs.h"
#include "httpftp.h"

#include <unistd.h>
#include <errno.h>

#define BUFSIZE 10240

cvar_t *canupdate;

char *Sec_StrTok(char *str,char *tokens,int id){
    static char *mem[100] = {NULL};
    char *ptr,*ptr2;
    if(id<0||id>99||tokens==NULL)
	return NULL;
    if(str==NULL){
	if(mem[id]==NULL){
	    return NULL;
	}
	
	for(ptr=mem[id];*ptr != 0;++ptr){
	    //printf("---%c\n",*ptr);
	    for(ptr2=tokens;*ptr2!=0;++ptr2){
		//printf("------%c\n",*ptr2);
		if(*ptr == *ptr2){
		    //printf("DEBUG!!!!!!!! %p:\"%s\", %p:\"%s\",%p:\"%s\".\n\n",ptr,ptr,ptr2,ptr2,mem[id],mem[id]);
		    *ptr=0;
		    ptr2=mem[id];
		    mem[id]=ptr+1;
		    //printf("DEBUG!!!!!!!! %p:\"%s\", %p:\"%s\".\n\n",ptr,ptr,ptr2,ptr2);
		    //__asm__("int $3");
		    return ptr2;
		}
	    }
	}
	if(ptr!=mem[id]){
	    ptr = mem[id];
	}
	else
	    ptr = NULL;
	mem[id]=NULL;
	return ptr;
    }
    else{
    //printf("Debugging: \"%s\"\n",str);
	//mem[id]=str;
	for(ptr=str;*ptr != 0 && mem[id]==NULL;++ptr){
	    for(ptr2=tokens;*ptr2!=0 && mem[id]==NULL;++ptr2){
		if(*ptr != *ptr2){
		    mem[id]=ptr;
		}
	    }
	}
	if(mem[id] == NULL) return NULL;
	return Sec_StrTok(NULL,tokens,id); // BECAUSE I CAN.
    }	
}

void Sec_FreeFileStruct(sec_file_t *file){
    if(file->next != NULL)
	Sec_FreeFileStruct(file->next);
    Sec_Free(file);
}

void Sec_Update( qboolean getbasefiles ){
    char buff[SEC_UPDATE_INITIALBUFFSIZE];
    char *ptr,*ptr2, *testfile;
	char filepathbuf[MAX_OSPATH];
    char baseurl[1024];
    char name1[256],name2[256];
    sec_file_t files, *currFile = &files;
    qboolean dlExec = qfalse;
    int len;
    char hash[128];
    long unsigned size;
	ftRequest_t* filetransferobj;
	ftRequest_t* curfileobj;
	int transret;
	mvabuf;

	
    if(!Sec_Initialized()){
	return;
    }
    
#ifdef CAN_UPDATE
    Com_Printf("\n-----------------------------\n");
    Com_Printf(" CoD4X Auto Update\n");
    Com_Printf(" Current version: %g\n",SEC_VERSION);
    Com_Printf(" Current build: %d\n",BUILD_NUMBER);
    Com_Printf(" Current type: %s\n",SEC_TYPE == 's' ? "stable      " : "experimental");
    Com_Printf("-----------------------------\n\n");

    canupdate = Cvar_RegisterBool("allowupdating", qtrue, 0, "This enables autoupdating of CoD4 server with new versions.");

    if(getbasefiles == qtrue)
    {

        Com_sprintf(buff, sizeof(buff), "http://" SEC_UPDATE_HOST SEC_UPDATE_GETGROUNDVERSION);

    }else{

        if(canupdate->boolean == qfalse)
            return;

        Com_sprintf(buff, sizeof(buff), "http://" SEC_UPDATE_HOST SEC_UPDATE_GETVERSION);
    }
#else
    if(getbasefiles == qtrue)
    {
        Com_sprintf(buff, sizeof(buff), "http://" SEC_UPDATE_HOST SEC_UPDATE_GETGROUNDVERSION);
    }else{
        return;
    }
#endif
	
	filetransferobj = FileDownloadRequest( buff );

    if(filetransferobj == NULL){
		return;
    }

	do {
		transret = FileDownloadSendReceive( filetransferobj );
		usleep(20000);
	} while (transret == 0);

    if(transret < 0)
	{
		FileDownloadFreeRequest(filetransferobj);
		return;
    }
    /* Need to catch errors */
 //   FS_WriteFile("tmp.txt", va("%d", status), 1);

    // TODO: Do something with the status?

//    FS_WriteFile("tmp2.txt", packet.header, packet.headerLength);
//    FS_WriteFile("tmp3.txt", packet.content, packet.contentLength);
    if(filetransferobj->code <= 0){
		Com_PrintError("Receiving data. Error code: %d.\n", filetransferobj->code);
		FileDownloadFreeRequest(filetransferobj);
		return;
    }
    if(filetransferobj->code == 204){
		Com_Printf("\nServer is up to date.\n\n");
		FileDownloadFreeRequest(filetransferobj);
		return;
    }
    else if(filetransferobj->code != 200){
		Com_PrintWarning("The update server's malfunction.\nStatus code: %d.\n", filetransferobj->code);
		FileDownloadFreeRequest(filetransferobj);
		return;
    }

    Com_Memset(&files, 0, sizeof(files));

    /* We need to parse filenames etc */
    ptr = Sec_StrTok((char*)(filetransferobj->recvmsg.data + filetransferobj->headerLength),"\n",42); // Yes, 42.
    if(ptr == NULL || Q_stricmpn("baseurl: ", ptr, 9))
    {
	    Com_PrintWarning("Sec_Update: Corrupt data from update server. Update aborted.\n");
		FileDownloadFreeRequest(filetransferobj);
		return;
    }
    Q_strncpyz(baseurl, ptr +9, sizeof(baseurl));

    ptr = Sec_StrTok(NULL,"\n",42); // Yes, 42 again.

	while(ptr != NULL){
		
		currFile->next = Sec_GMalloc(sec_file_t,1);
		currFile = currFile->next;
		Com_Memset(currFile,0,sizeof(sec_file_t));
		ptr2 = strchr(ptr,' ');
		if(ptr2 == NULL){
			Com_PrintWarning("Sec_Update: Corrupt data from update server. Update aborted.\nDebug:\"%s\"\n",ptr);
			FileDownloadFreeRequest(filetransferobj);
			return;
		}
		*ptr2++ = 0;
		Q_strncpyz(currFile->path,ptr,sizeof(currFile->path));
		ptr = ptr2;
		ptr2 = strchr(ptr,' ');
		if(ptr2 == NULL){
			Com_PrintWarning("Sec_Update: Corrupt data from update server. Update aborted.\nDebug:\"%s\"\n",ptr);
			FileDownloadFreeRequest(filetransferobj);
			return;
		}
		*ptr2++ = 0;
		if(!isInteger(ptr, 0)){
			Com_PrintWarning("Sec_Update: Corrupt data from update server - size is not a number. Update aborted.\nDebug:\"%s\"\n",ptr);
			FileDownloadFreeRequest(filetransferobj);
			return;
		}
		currFile->size = atoi(ptr);
		Q_strncpyz(currFile->hash,ptr2,sizeof(currFile->hash));
		Q_strncpyz(currFile->name,currFile->path, sizeof(currFile->name));
		//printf("DEBUG: File to download: link: \"%s\", name: \"%s\", size: %d, hash: \"%s\"\n\n",file.path,file.name,file.size,file.hash);

		Com_sprintf(buff, sizeof(buff), SEC_UPDATE_DOWNLOAD(baseurl, currFile->path));
		
		curfileobj = FileDownloadRequest(buff);
		if(curfileobj == NULL)
		{
			FileDownloadFreeRequest(filetransferobj);
			return;	
		}

		Com_Printf("Downloading file: \"%s\"\n\n",currFile->name);

		do {
			transret = FileDownloadSendReceive( curfileobj );
			Com_Printf("%s", FileDownloadGenerateProgress( curfileobj ));
			usleep(20000);
		} while (transret == 0);
		
		Com_Printf("\n");

		if(transret < 0)
		{
			FileDownloadFreeRequest(curfileobj);
			FileDownloadFreeRequest(filetransferobj);
			return;
		}

		Q_strncpyz(buff,currFile->name, sizeof(buff));
		Q_strcat(buff, sizeof(buff),".new");

		if(curfileobj->code != 200){
			Com_PrintError("Downloading has failed! Error code: %d. Update aborted.\n", curfileobj->code);
			FileDownloadFreeRequest(filetransferobj);
			FileDownloadFreeRequest(curfileobj);
			return;
		}

		len = FS_SV_BaseWriteFile(buff, curfileobj->recvmsg.data + curfileobj->headerLength, curfileobj->contentLength);
		if(len != curfileobj->contentLength){

			len = FS_SV_HomeWriteFile(buff, curfileobj->recvmsg.data + curfileobj->headerLength, curfileobj->contentLength);
			if(len != curfileobj->contentLength)
			{
				Com_PrintError("Opening \"%s\" for writing! Update aborted.\n",buff);
				FileDownloadFreeRequest(filetransferobj);
				FileDownloadFreeRequest(curfileobj);
				return;
			}
		}

		ptr = Sec_StrTok(NULL,"\n",42); // Yes, 42 again.

		size = sizeof(hash);
		
		if(!Sec_HashMemory(SEC_HASH_SHA256, curfileobj->recvmsg.data + curfileobj->headerLength, curfileobj->contentLength, hash, &size,qfalse)){
			Com_PrintError("Hashing the file \"%s\". Error code: %s.\nUpdate aborted.\n",currFile->name,Sec_CryptErrStr(SecCryptErr));
			FileDownloadFreeRequest(filetransferobj);
			FileDownloadFreeRequest(curfileobj);
			return;
		}

		FileDownloadFreeRequest(curfileobj);
		
		if(!Q_strncmp(hash, currFile->hash, size)){
			Com_Printf("Successfully downloaded file \"%s\".\n", currFile->name);
		}
		else{
			Com_PrintError("File \"%s\" is corrupt!\nUpdate aborted.\n",currFile->name);
			Com_DPrintf("Hash: \"%s\", correct hash: \"%s\".\n",hash,currFile->hash);
			FileDownloadFreeRequest(filetransferobj);
			return;
		}
		
	}

	FileDownloadFreeRequest(filetransferobj);

    Com_Printf("All files downloaded successfully. Applying update...\n");

    currFile = files.next;
    do{
		Com_Printf("Updating file %s...\n", currFile->name);
		Q_strncpyz(name1, currFile->name, sizeof(name1));

		Q_strcat(name1, sizeof(name1), ".old");

		Q_strncpyz(name2, currFile->name, sizeof(name2));

		Q_strcat(name2, sizeof(name2), ".new");

		testfile = FS_SV_GetFilepath(name1, filepathbuf, sizeof(filepathbuf));
		if(testfile != NULL)
		{ // Old file exists, back it up
			FS_SV_BaseRemove( name1 );
			FS_SV_HomeRemove( name1 );
			testfile = FS_SV_GetFilepath(name1, filepathbuf, sizeof(filepathbuf));
			if(testfile != NULL)
			{
				Com_PrintWarning("Couldn't remove backup file: %s\n", testfile);
			}
			if(FS_SV_HomeFileExists(name1) == qtrue)
			{
				Com_PrintError("Couldn't remove backup file from fs_homepath: %s\n", name1);
			}
		}
		// Check if an old file exists with this name
		testfile = FS_SV_GetFilepath(currFile->name, filepathbuf, sizeof(filepathbuf));
		if(testfile != NULL)
		{ // Old file exists, back it up
			FS_SV_Rename(currFile->name, name1);
		}
		testfile = FS_SV_GetFilepath(currFile->name, filepathbuf, sizeof(filepathbuf));
		// We couldn't back it up. Now we try to just delete it.
		if(testfile != NULL)
		{
			FS_SV_BaseRemove( currFile->name );
			FS_SV_HomeRemove( currFile->name );
			testfile = FS_SV_GetFilepath( currFile->name, filepathbuf, sizeof(filepathbuf) );
			if(testfile != NULL)
			{
				Com_PrintWarning("Couldn't remove file: %s\n", testfile);
			}
			if(FS_SV_HomeFileExists(currFile->name) == qtrue)
			{
				Com_PrintError("Couldn't remove file from fs_homepath: %s\n", currFile->name);
				Com_PrintError("Update has failed!\n");
				return;
			}
		}

		if(Q_strncmp(currFile->name, EXECUTABLE_NAME, 15)){
			/* This is not the executable file */
			FS_SV_Rename(name2, currFile->name);
			testfile = FS_SV_GetFilepath(currFile->name, filepathbuf, sizeof(filepathbuf));
			if(testfile == NULL)
			{
				Com_PrintError("Failed to rename file %s to %s\n", name2,currFile->name);
				Com_PrintError("Update has failed!\n");
				return;
			}
			Com_Printf("Update on file %s successfully applied.\n",currFile->name);

		}else{
			/* This is the executable file */
			testfile = FS_SV_GetFilepath(name2, filepathbuf, sizeof(filepathbuf));
			if(testfile == NULL)
			{
				Com_PrintError("Can not find file %s\n", name2);
				Com_PrintError("Update has failed!\n");
				return;
			}
			if(FS_SetPermissionsExec(name2) == qfalse)
			{
				Com_PrintError("CRITICAL ERROR: failed to change mode of the file \"%s\"! Aborting, manual installation might be required.\n", name2);
				return;
			}
			FS_RenameOSPath(Sys_ExeFile(), va("%s.dead", Sys_ExeFile()));
			FS_RemoveOSPath(va("%s.dead", Sys_ExeFile()));
			FS_RemoveOSPath(Sys_ExeFile());
			if(FS_FileExistsOSPath(Sys_ExeFile()))
			{
				Com_PrintError("Failed to delete file %s\n", Sys_ExeFile());
				Com_PrintError("Update has failed!\n");
				return;
			}
			FS_RenameOSPath(testfile, Sys_ExeFile());
			if(!FS_FileExistsOSPath(Sys_ExeFile()))
			{
				Com_PrintError("Failed to rename file %s\n", testfile);
				Com_PrintError("Update has failed! Manual reinstallation of file %s is required. This server is now broken!\n", Sys_ExeFile());
				return;
			}
			Com_Printf("Update on file %s successfully applied.\n", Sys_ExeFile());
			dlExec = qtrue;
		}
		currFile = currFile->next;

    }while(currFile != NULL);

    Sec_FreeFileStruct(files.next);
    Com_Printf("Finalizing update...\n");


    if(dlExec == qtrue)
    {
		Sys_Restart("System has been updated and will restart now.");
    }else{
        FS_Restart( 0 );
    }
}