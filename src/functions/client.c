#include "client.h"

void PlayerCmd_GetProtocolVersion(scr_entref_t arg) {
	gentity_t* gentity;
	int entityNum = 0;
	mvabuf;

	if (HIWORD(arg)) {
		Scr_ObjectError("Not an entity");
	} else {
		entityNum = LOWORD(arg);
		gentity = &g_entities[entityNum];

		if (!gentity->client) {
			Scr_ObjectError(va("Entity: %i is not a player", entityNum));
		}
	}

    client_t *cl = &svs.clients[entityNum];
	Scr_AddInt(cl->protocol);
}
