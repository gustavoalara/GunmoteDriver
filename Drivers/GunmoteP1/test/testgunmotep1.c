#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "gunmotep1client.h"

//
// Function prototypes
//

VOID
SendHidRequests(
    pgunmotep1_client gunmotep1,
    BYTE requestType
    );

//
// Implementation
//

void
Usage(void)
{
    printf("Usage: testgunmotep1 < /mouse | /keyboard | /message>\n");
}

INT __cdecl
main(
    int argc,
    PCHAR argv[]
    )
{
    BYTE   reportId;
    pgunmotep1_client gunmotep1;

    UNREFERENCED_PARAMETER(argv);

    //
    // Parse command line
    //

    if (argc == 1)
    {
        Usage();
        return 1;
    }
    else if (strcmp(argv[1], "/mouse") == 0)
    {
        reportId = REPORTID_MOUSE;
    }
    else if (strcmp(argv[1], "/keyboard") == 0)
    {
        reportId = REPORTID_KEYBOARD;
    }
    else if (strcmp(argv[1], "/message") == 0)
    {
        reportId = REPORTID_MESSAGE;
    }
    else
    {
        Usage();
        return 1;
    }

    //
    // File device
    //

    gunmotep1 = gunmotep1_alloc();

    if (gunmotep1 == NULL)
    {
        return 2;
    }

    if (!gunmotep1_connect(gunmotep1))
    {
        gunmotep1_free(gunmotep1);
        return 3;
    }

    printf("...sending request(s) to our device\n");
    SendHidRequests(gunmotep1, reportId);

    gunmotep1_disconnect(gunmotep1);

    gunmotep1_free(gunmotep1);

    return 0;
}

VOID
SendHidRequests(
    pgunmotep1_client gunmotep1,
    BYTE requestType
    )
{
    switch (requestType)
    {
        

        case REPORTID_MOUSE:
            //
            // Send the mouse report
            //
            printf("Sending mouse report\n");
            gunmotep1_update_mouse(gunmotep1, 0, 1000, 10000, 0);
            break;

        case REPORTID_KEYBOARD:
        {
            //
            // Send the keyboard report
            //

            // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
            // for a list of key codes            
                        
            BYTE shiftKeys = KBD_LGUI_BIT;
            BYTE keyCodes[KBD_KEY_CODES] = {0, 0, 0, 0, 0, 0};
            printf("Sending keyboard report\n");

            // Windows key
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            shiftKeys = 0;
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            Sleep(100);
            
            // 'Hello'
            shiftKeys = KBD_LSHIFT_BIT;
            keyCodes[0] = 0x0b; // 'h'
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            shiftKeys = 0;
            keyCodes[0] = 0x08; // 'e'
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x0f; // 'l'
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x0f; // 'l'
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x12; // 'o'
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            
            // Toggle caps lock
            keyCodes[0] = 0x39; // caps lock
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);
            keyCodes[0] = 0x0;
            gunmotep1_update_keyboard(gunmotep1, shiftKeys, keyCodes);

            break;
        }

        case REPORTID_MESSAGE:
        {
            gunmotep1MessageReport report;

            printf("Writing vendor message report\n");

            memcpy(report.Message, "Hello gunmotep1\x00", 13);

            if (gunmotep1_write_message(gunmotep1, &report))
            {
                memset(&report, 0, sizeof(report));
                printf("Reading vendor message report\n");
                if (gunmotep1_read_message(gunmotep1, &report))
                {
                    printf("Success!\n    ");
                    printf(report.Message);
                }
            }

            break;
        }
    }
}


