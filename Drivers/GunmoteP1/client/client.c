#include <windows.h>
#include <hidsdi.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "gunmotep1client.h"

#if __GNUC__
    #define __in
    #define __in_ecount(x)
    typedef void* PVOID;
    typedef PVOID HDEVINFO;
    WINHIDSDI BOOL WINAPI HidD_SetOutputReport(HANDLE, PVOID, ULONG);
#endif

typedef struct _gunmotep1_client_t
{
    HANDLE hControl;
    HANDLE hMessage;
    BYTE controlReport[CONTROL_REPORT_SIZE];
} gunmotep1_client_t;

//
// Function prototypes
//

HANDLE
SearchMatchingHwID (
    USAGE myUsagePage,
    USAGE myUsage
    );

HANDLE
OpenDeviceInterface (
    HDEVINFO HardwareDeviceInfo,
    PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    USAGE myUsagePage,
    USAGE myUsage
    );

BOOLEAN
CheckIfOurDevice(
    HANDLE file,
    USAGE myUsagePage,
    USAGE myUsage
    );

BOOL
HidOutput(
    BOOL useSetOutputReport,
    HANDLE file,
    PCHAR buffer,
    ULONG bufferSize
    );

//
// Copied this structure from hidport.h
//

typedef struct _HID_DEVICE_ATTRIBUTES {

    ULONG           Size;
    //
    // sizeof (struct _HID_DEVICE_ATTRIBUTES)
    //
    //
    // Vendor ids of this hid device
    //
    USHORT          VendorID;
    USHORT          ProductID;
    USHORT          VersionNumber;
    USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, * PHID_DEVICE_ATTRIBUTES;

//
// Implementation
//

pgunmotep1_client gunmotep1_alloc(void)
{
    return (pgunmotep1_client)malloc(sizeof(gunmotep1_client_t));
}

void gunmotep1_free(pgunmotep1_client gunmotep1)
{
    free(gunmotep1);
}

BOOL gunmotep1_connect(pgunmotep1_client gunmotep1)
{
    //
    // Find the HID devices
    //

    gunmotep1->hControl = SearchMatchingHwID(0xff00, 0x0001);
    if (gunmotep1->hControl == INVALID_HANDLE_VALUE || gunmotep1->hControl == NULL)
        return FALSE;
    gunmotep1->hMessage = SearchMatchingHwID(0xff00, 0x0002);
    if (gunmotep1->hMessage == INVALID_HANDLE_VALUE || gunmotep1->hMessage == NULL)
    {
        gunmotep1_disconnect(gunmotep1);
        return FALSE;
    }

    //
    // Set the buffer count to 10 on the message HID
    //

    if (!HidD_SetNumInputBuffers(gunmotep1->hMessage, 10))
    {
        printf("failed HidD_SetNumInputBuffers %d\n", GetLastError());
        gunmotep1_disconnect(gunmotep1);
        return FALSE;
    }

    return TRUE;
}

void gunmotep1_disconnect(pgunmotep1_client gunmotep1)
{
    if (gunmotep1->hControl != NULL)
        CloseHandle(gunmotep1->hControl);
    if (gunmotep1->hMessage != NULL)
        CloseHandle(gunmotep1->hMessage);
    gunmotep1->hControl = NULL;
    gunmotep1->hMessage = NULL;
}

BOOL gunmotep1_update_mouse(pgunmotep1_client gunmotep1, BYTE button, USHORT x, USHORT y, BYTE wheelPosition)
{
    gunmotep1ControlReportHeader* pReport = NULL;
    gunmotep1MouseReport* pMouseReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(gunmotep1ControlReportHeader) + sizeof(gunmotep1MouseReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (gunmotep1ControlReportHeader*)gunmotep1->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(gunmotep1MouseReport);

    //
    // Set the input report
    //

    pMouseReport = (gunmotep1MouseReport*)(gunmotep1->controlReport + sizeof(gunmotep1ControlReportHeader));
    pMouseReport->ReportID = REPORTID_MOUSE;
    pMouseReport->Button = button;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    return HidOutput(FALSE, gunmotep1->hControl, (PCHAR)gunmotep1->controlReport, CONTROL_REPORT_SIZE);
}

BOOL gunmotep1_update_keyboard(pgunmotep1_client gunmotep1, BYTE shiftKeyFlags, BYTE keyCodes[KBD_KEY_CODES])
{
    gunmotep1ControlReportHeader* pReport = NULL;
    gunmotep1KeyboardReport* pKeyboardReport = NULL;

    if (CONTROL_REPORT_SIZE <= sizeof(gunmotep1ControlReportHeader) + sizeof(gunmotep1KeyboardReport))
    {
        return FALSE;
    }

    //
    // Set the report header
    //

    pReport = (gunmotep1ControlReportHeader*)gunmotep1->controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(gunmotep1KeyboardReport);

    //
    // Set the input report
    //

    pKeyboardReport = (gunmotep1KeyboardReport*)(gunmotep1->controlReport + sizeof(gunmotep1ControlReportHeader));
    pKeyboardReport->ReportID = REPORTID_KEYBOARD;
    pKeyboardReport->ShiftKeyFlags = shiftKeyFlags;
    memcpy(pKeyboardReport->KeyCodes, keyCodes, KBD_KEY_CODES);

    // Send the report
    return HidOutput(FALSE, gunmotep1->hControl, (PCHAR)gunmotep1->controlReport, CONTROL_REPORT_SIZE);
}

BOOL gunmotep1_write_message(pgunmotep1_client gunmotep1, gunmotep1MessageReport* pReport)
{
    gunmotep1ControlReportHeader* pReportHeader;
    ULONG bytesWritten;

    //
    // Set the report header
    //

    pReportHeader = (gunmotep1ControlReportHeader*)gunmotep1->controlReport;
    pReportHeader->ReportID = REPORTID_CONTROL;
    pReportHeader->ReportLength = sizeof(gunmotep1MessageReport);

    //
    // Set the body
    //

    pReport->ReportID = REPORTID_MESSAGE;
    memcpy(gunmotep1->controlReport + sizeof(gunmotep1ControlReportHeader), pReport, sizeof(gunmotep1MessageReport));

    //
    // Write the report
    //

    if (!WriteFile(gunmotep1->hControl, gunmotep1->controlReport, CONTROL_REPORT_SIZE, &bytesWritten, NULL))
    {
        printf("failed WriteFile %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL gunmotep1_read_message(pgunmotep1_client gunmotep1, gunmotep1MessageReport* pReport)
{
    ULONG bytesRead;

    //
    // Read the report
    //

    if (!ReadFile(gunmotep1->hMessage, pReport, sizeof(gunmotep1MessageReport), &bytesRead, NULL))
    {
        printf("failed ReadFile %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

HANDLE
SearchMatchingHwID (
    USAGE myUsagePage,
    USAGE myUsage
    )
{
    HDEVINFO                  hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA  deviceInterfaceData;
    SP_DEVINFO_DATA           devInfoData;
    GUID                      hidguid;
    int                       i;

    HidD_GetHidGuid(&hidguid);

    hardwareDeviceInfo =
            SetupDiGetClassDevs ((LPGUID)&hidguid,
                                            NULL,
                                            NULL, // Define no
                                            (DIGCF_PRESENT |
                                            DIGCF_INTERFACEDEVICE));

    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //
    // Enumerate devices of this interface class
    //

    printf("\n....looking for our HID device (with UP=0x%x "
                "and Usage=0x%x)\n", myUsagePage, myUsage);

    for (i = 0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                            0, // No care about specific PDOs
                            (LPGUID)&hidguid,
                            i, //
                            &deviceInterfaceData);
                            i++)
    {

        //
        // Open the device interface and Check if it is our device
        // by matching the Usage page and Usage from Hid_Caps.
        // If this is our device then send the hid request.
        //

        HANDLE file = OpenDeviceInterface(hardwareDeviceInfo, &deviceInterfaceData, myUsagePage, myUsage);

        if (file != INVALID_HANDLE_VALUE)
        {
            SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
            return file;
        }

        //
        //device was not found so loop around.
        //

    }

    printf("Failure: Could not find our HID device \n");

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    return INVALID_HANDLE_VALUE;
}

HANDLE
OpenDeviceInterface (
    HDEVINFO hardwareDeviceInfo,
    PSP_DEVICE_INTERFACE_DATA deviceInterfaceData,
    USAGE myUsagePage,
    USAGE myUsage
    )
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;

    DWORD        predictedLength = 0;
    DWORD        requiredLength = 0;
    HANDLE       file = INVALID_HANDLE_VALUE;

    SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            NULL, // probing so no output buffer yet
                            0, // probing so output buffer length of zero
                            &requiredLength,
                            NULL
                            ); // not interested in the specific dev-node

    predictedLength = requiredLength;

    deviceInterfaceDetailData =
         (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);

    if (!deviceInterfaceDetailData)
    {
        printf("Error: OpenDeviceInterface: malloc failed\n");
        goto cleanup;
    }

    deviceInterfaceDetailData->cbSize =
                    sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            deviceInterfaceDetailData,
                            predictedLength,
                            &requiredLength,
                            NULL))
    {
        printf("Error: SetupDiGetInterfaceDeviceDetail failed\n");
        free (deviceInterfaceDetailData);
        goto cleanup;
    }

    file = CreateFile ( deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_READ,
                            NULL, // no SECURITY_ATTRIBUTES structure
                            OPEN_EXISTING, // No special create flags
                            0, // No special attributes
                            NULL); // No template file

    if (INVALID_HANDLE_VALUE == file) {
        printf("Error: CreateFile failed: %d\n", GetLastError());
        goto cleanup;
    }

    if (CheckIfOurDevice(file, myUsagePage, myUsage)){

        goto cleanup;

    }

    CloseHandle(file);

    file = INVALID_HANDLE_VALUE;

cleanup:

    free (deviceInterfaceDetailData);

    return file;

}


BOOLEAN
CheckIfOurDevice(
    HANDLE file,
    USAGE myUsagePage,
    USAGE myUsage)
{
    PHIDP_PREPARSED_DATA Ppd = NULL; // The opaque parser info describing this device
    HIDD_ATTRIBUTES                 Attributes; // The Attributes of this hid device.
    HIDP_CAPS                       Caps; // The Capabilities of this hid device.
    BOOLEAN                         result = FALSE;

    if (!HidD_GetPreparsedData (file, &Ppd))
    {
        printf("Error: HidD_GetPreparsedData failed \n");
        goto cleanup;
    }

    if (!HidD_GetAttributes(file, &Attributes))
    {
        printf("Error: HidD_GetAttributes failed \n");
        goto cleanup;
    }

    if (Attributes.VendorID == gunmotep1_VID && Attributes.ProductID == gunmotep1_PID)
    {
        if (!HidP_GetCaps (Ppd, &Caps))
        {
            printf("Error: HidP_GetCaps failed \n");
            goto cleanup;
        }

        if ((Caps.UsagePage == myUsagePage) && (Caps.Usage == myUsage))
        {
            printf("Success: Found my device.. \n");
            result = TRUE;
        }
    }

cleanup:

    if (Ppd != NULL)
    {
        HidD_FreePreparsedData (Ppd);
    }

    return result;
}

BOOL
HidOutput(
    BOOL useSetOutputReport,
    HANDLE file,
    PCHAR buffer,
    ULONG bufferSize
    )
{
    ULONG bytesWritten;
    if (useSetOutputReport)
    {
        //
        // Send Hid report thru HidD_SetOutputReport API
        //

        if (!HidD_SetOutputReport(file, buffer, bufferSize))
        {
            printf("failed HidD_SetOutputReport %d\n", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (!WriteFile(file, buffer, bufferSize, &bytesWritten, NULL))
        {
            printf("failed WriteFile %d\n", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}
