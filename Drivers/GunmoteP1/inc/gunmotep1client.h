#if !defined(_gunmotep1_CLIENT_H_)
#define _gunmotep1_CLIENT_H_

#include "gunmotep1common.h"

typedef struct _gunmotep1_client_t* pgunmotep1_client;

pgunmotep1_client gunmotep1_alloc(void);

void gunmotep1_free(pgunmotep1_client gunmotep1);

BOOL gunmotep1_connect(pgunmotep1_client gunmotep1);

void gunmotep1_disconnect(pgunmotep1_client gunmotep1);

BOOL gunmotep1_update_mouse(pgunmotep1_client gunmotep1, BYTE button, USHORT x, USHORT y, BYTE wheelPosition);

BOOL gunmotep1_update_keyboard(pgunmotep1_client gunmotep1, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES]);

BOOL gunmotep1_write_message(pgunmotep1_client gunmotep1, gunmotep1MessageReport* pReport);

BOOL gunmotep1_read_message(pgunmotep1_client gunmotep1, gunmotep1MessageReport* pReport);

#endif

