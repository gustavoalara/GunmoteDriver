#if !defined(_vmultia_COMMON_H_)
#define _vmultia_COMMON_H_

//
//These are the device attributes returned by vmultia in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//

#define vmultia_PID              0xBA1C
#define vmultia_VID              0x001F
#define vmultia_VERSION          0x0001

//
// These are the report ids
//

#define REPORTID_FEATURE        0x02
#define REPORTID_MOUSE          0x03
#define REPORTID_KEYBOARD       0x07
#define REPORTID_MESSAGE        0x10
#define REPORTID_CONTROL        0x40

//
// Control defined report size
//

#define CONTROL_REPORT_SIZE      0x41

//
// Report header
//

#pragma pack(1)
typedef struct _vmultia_CONTROL_REPORT_HEADER
{

    BYTE        ReportID;

    BYTE        ReportLength;

} vmultiaControlReportHeader;
#pragma pack()

//
// Keyboard specific report infomation
//

#define KBD_LCONTROL_BIT     1
#define KBD_LSHIFT_BIT       2
#define KBD_LALT_BIT         4
#define KBD_LGUI_BIT         8
#define KBD_RCONTROL_BIT     16
#define KBD_RSHIFT_BIT       32
#define KBD_RALT_BIT         64
#define KBD_RGUI_BIT         128

#define KBD_KEY_CODES        6

#pragma pack(1)
typedef struct _vmultia_KEYBOARD_REPORT
{

    BYTE      ReportID;

    // Left Control, Left Shift, Left Alt, Left GUI
    // Right Control, Right Shift, Right Alt, Right GUI
    BYTE      ShiftKeyFlags;

    BYTE      Reserved;

    // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
    // for a list of key codes
    BYTE      KeyCodes[KBD_KEY_CODES];

} vmultiaKeyboardReport;

typedef struct _vmultia_KEYBOARD_OUTPUT_REPORT
{
    // Num Lock, Caps Lock, Scroll Lock, Compose, Kana
    BYTE      LedFlags;
} vmultiaKeyboardOutputReport;

#pragma pack()


//
// Mouse specific report information
//

#define MOUSE_BUTTON_1     0x01
#define MOUSE_BUTTON_2     0x02
#define MOUSE_BUTTON_3     0x04

#define MOUSE_MIN_COORDINATE   0x0000
#define MOUSE_MAX_COORDINATE   0x7FFF

#define MIN_WHEEL_POS   -127
#define MAX_WHEEL_POS    127

#pragma pack(1)
typedef struct _vmultia_MOUSE_REPORT
{

    BYTE        ReportID;

    BYTE        Button;

    USHORT      XValue;

    USHORT      YValue;

    BYTE        WheelPosition;

} vmultiaMouseReport;
#pragma pack()


//
// Feature report infomation
//

#define DEVICE_MODE_MOUSE        0x00
#define DEVICE_MODE_SINGLE_INPUT 0x01
#define DEVICE_MODE_MULTI_INPUT  0x02

#pragma pack(1)
typedef struct _vmultia_FEATURE_REPORT
{

    BYTE      ReportID;

    BYTE      DeviceMode;

    BYTE      DeviceIdentifier;

} vmultiaFeatureReport;

typedef struct _vmultia_MAXCOUNT_REPORT
{

    BYTE         ReportID;

    BYTE         MaximumCount;

} vmultiaMaxCountReport;
#pragma pack()

//
// Message specific report information
//

#define MESSAGE_SIZE 0x20

#pragma pack(1)
typedef struct _vmultia_MESSAGE_REPORT
{

    BYTE        ReportID;

    char        Message[MESSAGE_SIZE];

} vmultiaMessageReport;
#pragma pack()

#endif

