#include "../pinc.h"
PCL int OnInit(){	// Funciton called on server initiation
	//	Load function pointers
	//	Function pointers loaded, add your plugin's content here
	return 0;
}

PCL void OnMessageSent(char *message, int slot, qboolean *show, int mode){
	if(strstr(message, "^7just got owned by ^2k^7oncept COD4")){
		Plugin_Printf("[Anti Koncept] Koncept CoD4 Detected! (Client: %s, Message: \"%s\")\n",Plugin_GetPlayerName(slot), message);
		Plugin_G_LogPrintf("[Anti Koncept] Koncept CoD4 Detected! (Client: %s %s, Message: \"%s\")\n",Plugin_GetPlayerName(slot),Plugin_GetPlayerGUID(slot), message);
		Plugin_BanClient( slot, -1, 0, "Koncept Detected (Autoban)" );
	}
}
PCL void OnInfoRequest(pluginInfo_t *info){	// Function used to obtain information about the plugin
    // Memory pointed by info is allocated by the server binary, just fill in the fields

    // =====  MANDATORY FIELDS  =====
    info->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
    info->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;	// Requested handler version
    // Requested handler version

    // =====  OPTIONAL  FIELDS  =====
    info->pluginVersion.major = 1;
    info->pluginVersion.minor = 1;	// Plugin version
    strncpy(info->fullName,"Anti Koncept Plugin by NNJ (thecod4ninja)",sizeof(info->fullName));
}
