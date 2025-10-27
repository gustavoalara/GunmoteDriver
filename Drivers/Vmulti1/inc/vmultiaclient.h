#if !defined(_vmultia_CLIENT_H_)
#define _vmultia_CLIENT_H_

#include "vmultiacommon.h"

typedef struct _vmultia_client_t* pvmultia_client;

pvmultia_client vmultia_alloc(void);

void vmultia_free(pvmultia_client vmultia);

BOOL vmultia_connect(pvmultia_client vmultia);

void vmultia_disconnect(pvmultia_client vmultia);

BOOL vmultia_update_mouse(pvmultia_client vmultia, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL vmultia_update_keyboard(pvmultia_client vmultia, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL vmultia_write_message(pvmultia_client vmultia, vmultiaMessageReport* pReport);

BOOL vmultia_read_message(pvmultia_client vmultia, vmultiaMessageReport* pReport);

#endif

