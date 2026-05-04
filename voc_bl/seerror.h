/* $Header: $ */

// *********************************************************************************
// *                                                                               *
// * Module Name: SEError.h                                                        *
// *                                                                               *
// * Description: Adaptec Satellite Express Error File.                            *
// *                                                                               *
// *                                                                               *
// *                                                                               *
// * Author     : George Kostas Grous                                              *
// *                                                                               *
// * Date       : 04/29/98                                                         *
// *                                                                               *
// * Notes      :                                                                  *
// *                                                                               *
// * Date       : Comment                                                          *
// * -----------  ---------------------------------------------------------------- *
// * 06/05/98     Updated error codes.                                             *
// *                                                                               *
// *                                                                               *
// *                                                                               *
// *********************************************************************************
// *                                                                               *
// * Copyright 1997 - 1998 Adaptec, Inc.,  All Rights Reserved                     *
// *                                                                               *
// * This software contains the valuable trade secrets of Adaptec.  The            *
// * software is protected under copyright laws as an unpublished work of          *
// * Adaptec.  Notice is for informational purposes only and does not imply        *
// * publication.  The user of this software may make copies of the software       *
// * for use with parts manufactured by Adaptec or under license from Adaptec      *
// * and for no other use.                                                         *
// *                                                                               *
/* *********************************************************************************
 *
 * $Revision: $
 * $Date: $
 * $Author: $
 * $History: $
 * 
 * $NoKeywords: $
 *
 *********************************************************************************/
 /*
 MessageIdTypedef=DWORD

 The MessageIdTypedef keyword gives a typedef name that is used in a
 type cast for each message code in the generated include file. Each
 message code appears in the include file with the format: #define
 name ((type) 0xnnnnnnnn) The default value for type is empty, and no
 type cast is generated. It is the programmer's responsibility to
 specify a typedef statement in the application source code to define
 the type. The type used in the typedef must be large enough to
 accommodate the entire 32-bit message code.



 The SeverityNames keyword defines the set of names that are allowed
 as the value of the Severity keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 severity name is a number that, when shifted left by 30, gives the
 bit pattern to logical-OR with the Facility value and MessageId
 value to form the full 32-bit message code. The default value of
 this keyword is:

 SeverityNames=(
   Success=0x0
   Informational=0x1
   Warning=0x2
   Error=0x3
   )

 Severity values occupy the high two bits of a 32-bit message code.
 Any severity value that does not fit in two bits is an error. The
 severity codes can be given symbolic names by following each value
 with :name



 The FacilityNames keyword defines the set of names that are allowed
 as the value of the Facility keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 facility name is a number that, when shift it left by 16 bits, gives
 the bit pattern to logical-OR with the Severity value and MessageId
 value to form the full 32-bit message code. The default value of
 this keyword is:

 FacilityNames=(
   System=0x0FF
   Application=0xFFF
   )

 Facility codes occupy the low order 12 bits of the high order
 16-bits of a 32-bit message code. Any facility code that does not
 fit in 12 bits is an error. This allows for 4,096 facility codes.
 The first 256 codes are reserved for use by the system software. The
 facility codes can be given symbolic names by following each value
 with :name


 The LanguageNames keyword defines the set of names that are allowed
 as the value of the Language keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 language name is a number and a file name that are used to name the
 generated resource file that contains the messages for that
 language. The number corresponds to the language identifier to use
 in the resource table. The number is separated from the file name
 with a colon.


 Any new names in the source file which don't override the built-in
 names are added to the list of valid languages. This allows an
 application to support private languages with descriptive names.


-------------------------------------------------------------------------
 MESSAGE DEFINITION SECTION

 Following the header section is the body of the Message Compiler
 source file. The body consists of zero or more message definitions.
 Each message definition begins with one or more of the following
 statements:

 MessageId = [number|+number]
 Severity = severity_name
 Facility = facility_name
 SymbolicName = name

 The MessageId statement marks the beginning of the message
 definition. A MessageID statement is required for each message,
 although the value is optional. If no value is specified, the value
 used is the previous value for the facility plus one. If the value
 is specified as +number then the value used is the previous value
 for the facility, plus the number after the plus sign. Otherwise, if
 a numeric value is given, that value is used. Any MessageId value
 that does not fit in 16 bits is an error.

 The Severity and Facility statements are optional. These statements
 specify additional bits to OR into the final 32-bit message code. If
 not specified they default to the value last specified for a message
 definition. The initial values prior to processing the first message
 definition are:

 Severity=Success
 Facility=Application

 The value associated with Severity and Facility must match one of
 the names given in the FacilityNames and SeverityNames statements in
 the header section. The SymbolicName statement allows you to
 associate a C/C++ symbolic constant with the final 32-bit message
 code.
 */
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define SE_FACILITY_SYSTEM               0xFF
#define SE_FACILITY_RUNTIME              0x200
#define SE_FACILITY_PSI_PARSER           0x300
#define SE_FACILITY_NDVBS                0x100


//
// Define the severity codes
//
#define SE_SEVERITY_WARNING              0x2
#define SE_SEVERITY_SUCCESS              0x0
#define SE_SEVERITY_INFORMATIONAL        0x1
#define SE_SEVERITY_ERROR                0x3


//
// MessageId: SE_OK
//
// MessageText:
//
//  The requested operation completed successfully.
//
#define SE_OK                            0x00FF0000L

//
// MessageId: SE_ERROR
//
// MessageText:
//
//  The requested operation could not be completed.
//
#define SE_ERROR                         0xC0FF0001L

//
// MessageId: SE_INSUFFICIENT_RESOURCES
//
// MessageText:
//
//  There is insufficient system resources to complete the requested operation.
//
#define SE_INSUFFICIENT_RESOURCES        0xC0FF0002L

//
// MessageId: SE_INVALID_BOARD_NUMBER
//
// MessageText:
//
//  The board number is invalid.
//
#define SE_INVALID_BOARD_NUMBER          0xC2000003L

//
// MessageId: SE_BOARD_NOT_ALLOCATED
//
// MessageText:
//
//  The requested board was not allocated.
//
#define SE_BOARD_NOT_ALLOCATED           0xC2000004L

//
// MessageId: SE_BUFFER_TOO_SMALL
//
// MessageText:
//
//  The user-specified buffer was too small for the operation to complete successfully.
//
#define SE_BUFFER_TOO_SMALL              0xC2000005L

//
// MessageId: SE_INVALID_PARAMETER
//
// MessageText:
//
//  A parameter is invalid.
//
#define SE_INVALID_PARAMETER             0xC2000006L

//
// MessageId: SE_INITIALIZATION_FAILURE
//
// MessageText:
//
//  NDVBS Initialization failure.
//
#define SE_INITIALIZATION_FAILURE        0xC0FF0007L

//
// MessageId: SE_DEVICE_FAILURE
//
// MessageText:
//
//  Hardware failure.
//
#define SE_DEVICE_FAILURE                0xC0FF0008L

//
// MessageId: SE_INVALID_IOCTL
//
// MessageText:
//
//  An invalid IOCTL was issued by the control DLL.
//
#define SE_INVALID_IOCTL                 0xC0FF0009L

//
// MessageId: SE_IOCTL_NOT_SUPPORTED
//
// MessageText:
//
//  The specified IOCTL is no longer supported.
//
#define SE_IOCTL_NOT_SUPPORTED           0xC0FF000AL

//
// MessageId: SE_NDVBS_INTERNAL_ERROR
//
// MessageText:
//
//  An internal NDVBS error was detected.
//
#define SE_NDVBS_INTERNAL_ERROR          0xC100000BL

//
// MessageId: SE_SERVICE_NOT_FOUND
//
// MessageText:
//
//  The selected service was not found.
//
#define SE_SERVICE_NOT_FOUND             0xC200000CL

//
// MessageId: SE_NOT_IMPLEMENTED
//
// MessageText:
//
//  The selected feature is not implemented.
//
#define SE_NOT_IMPLEMENTED               0xC200000DL

//
// MessageId: SE_TIMEOUT
//
// MessageText:
//
//  The selected operation timed out.
//
#define SE_TIMEOUT                       0xC200000EL

//
// MessageId: SE_INVALID_HANDLE
//
// MessageText:
//
//  The handle used for this operation is invalid.
//
#define SE_INVALID_HANDLE                0xC200000FL

//
// MessageId: SE_FILE_OPEN_ERROR
//
// MessageText:
//
//  The specified file could not be opened.
//
#define SE_FILE_OPEN_ERROR               0xC0FF0010L

//
// MessageId: SE_FILE_READ_ERROR
//
// MessageText:
//
//  An error occurred while reading from the specified file.
//
#define SE_FILE_READ_ERROR               0xC0FF0011L

//
// MessageId: SE_FILE_CLOSE_ERROR
//
// MessageText:
//
//  An error occurred while closing the specified file.
//
#define SE_FILE_CLOSE_ERROR              0xC0FF0012L

//
// MessageId: SE_INVALID_FILE_ERROR
//
// MessageText:
//
//  The specified file is invalid.
//
#define SE_INVALID_FILE_ERROR            0xC0FF0013L

//
// MessageId: SE_INFO_NOT_AVAILABLE
//
// MessageText:
//
//  The requested information is not available.
//
#define SE_INFO_NOT_AVAILABLE            0xC2000014L

//
// MessageId: SE_INVALID_INPUT_BUFFER_SIZE
//
// MessageText:
//
//  The input buffer size is invalid.
//
#define SE_INVALID_INPUT_BUFFER_SIZE     0xC1000100L

//
// MessageId: SE_INVALID_OUTPUT_BUFFER_SIZE
//
// MessageText:
//
//  The output buffer size is invalid.
//
#define SE_INVALID_OUTPUT_BUFFER_SIZE    0xC1000101L

//
// MessageId: SE_PARAMETER_OUT_OF_RANGE
//
// MessageText:
//
//  One or more input parameters are out of range.
//
#define SE_PARAMETER_OUT_OF_RANGE        0xC2000201L

//
// MessageId: SE_BOARD_ALREADY_ALLOCATED
//
// MessageText:
//
//  The specified board is already allocated. No further action is required.
//
#define SE_BOARD_ALREADY_ALLOCATED       0x82000202L

//
// MessageId: SE_OK_MASTER_STATUS_PENDING
//
// MessageText:
//
//  Master status is pending.
//
#define SE_OK_MASTER_STATUS_PENDING      0x42000203L

//
// MessageId: SE_APP_NOT_REGISTERED
//
// MessageText:
//
//  The required registration procedure was not performed.
//
#define SE_APP_NOT_REGISTERED            0xC2000204L

//
// MessageId: SE_APP_NOT_MASTER
//
// MessageText:
//
//  The requested function call requires the application to be registered as master before it can continue.
//
#define SE_APP_NOT_MASTER                0xC2000205L

//
// MessageId: SE_MUX_STREAM_NOT_FOUND
//
// MessageText:
//
//  The specified stream was not found.
//
#define SE_MUX_STREAM_NOT_FOUND          0xC2000301L

//
// MessageId: SE_NO_TUNER_NOT_LOCKED
//
// MessageText:
//
//  The tuner is not locked.
//
#define SE_NO_TUNER_NOT_LOCKED           0xC2000302L

//
// MessageId: SE_DMOD_NOT_LOCKED
//
// MessageText:
//
//  DMOD is not locked.
//
#define SE_DMOD_NOT_LOCKED               0xC2000303L

//
// MessageId: SE_LNB_FREQ_UNKNOWN
//
// MessageText:
//
//  The LNB frequency is unknown.
//
#define SE_LNB_FREQ_UNKNOWN              0xC2000304L

//
// MessageId: SE_LNB_NOT_SPECIFIED
//
// MessageText:
//
//  The LNB was not specfied.
//
#define SE_LNB_NOT_SPECIFIED             0xC2000305L

//
// MessageId: SE_TUNER_LB_FREQUENCY_OUT_OF_RANGE
//
// MessageText:
//
//  The specified frequency is out of range.
//
#define SE_TUNER_LB_FREQUENCY_OUT_OF_RANGE 0xC1000306L

//
// MessageId: SE_TUNER_POLARIZATION_OUT_OF_RANGE
//
// MessageText:
//
//  The specified polarization is out of range.
//
#define SE_TUNER_POLARIZATION_OUT_OF_RANGE 0xC1000307L

//
// MessageId: SE_TUNER_SYMBOL_RATE_OUT_OF_RANGE
//
// MessageText:
//
//  The specified symbol rate is out of range.
//
#define SE_TUNER_SYMBOL_RATE_OUT_OF_RANGE 0xC1000308L

//
// MessageId: SE_TUNER_22KHZ_FLAG_OUT_OF_RANGE
//
// MessageText:
//
//  The specified flag is out of range.
//
#define SE_TUNER_22KHZ_FLAG_OUT_OF_RANGE 0xC1000309L

//
// MessageId: SE_TUNER_SAT_FREQUENCY_OUT_OF_RANGE
//
// MessageText:
//
//  The specified frequency is out of range.
//
#define SE_TUNER_SAT_FREQUENCY_OUT_OF_RANGE 0xC100030AL

//
// MessageId: SE_TUNER_LNB_NOT_SET
//
// MessageText:
//
//  The tuner LNB was not set.
//
#define SE_TUNER_LNB_NOT_SET             0xC100030BL

//
// MessageId: SE_TUNER_TUNER_NOT_LOCKED
//
// MessageText:
//
//  The tuner is not locked.
//
#define SE_TUNER_TUNER_NOT_LOCKED        0xC100030CL

//
// MessageId: SE_TUNER_DMOD_NOT_LOCKED
//
// MessageText:
//
//  DMOD is not locked.
//
#define SE_TUNER_DMOD_NOT_LOCKED         0xC100030DL

//
// MessageId: SE_LNB_LOW_TOO_SMALL
//
// MessageText:
//
//  The LNB value is out of range.
//
#define SE_LNB_LOW_TOO_SMALL             0xC100030EL

//
// MessageId: SE_LNB_HIGH_LESS_THAN_LOW
//
// MessageText:
//
//  The LNB value is out of range.
//
#define SE_LNB_HIGH_LESS_THAN_LOW        0xC100030FL

//
// MessageId: SE_DELIVERY_INHIBITED_BY_CA
//
// MessageText:
//
//  Delivery was inhibited by CA.
//
#define SE_DELIVERY_INHIBITED_BY_CA      0xC1000401L

//
// MessageId: SE_NO_FILTER_AVAILABLE
//
// MessageText:
//
//  There is no filters available.
//
#define SE_NO_FILTER_AVAILABLE           0xC1000402L

//
// MessageId: SE_UNKNOWN_TABLE_FILTER_HANDLE
//
// MessageText:
//
//  The table filter handle is unknown.
//
#define SE_UNKNOWN_TABLE_FILTER_HANDLE   0xC1000403L

//
// MessageId: SE_NO_INCOMING_STREAM
//
// MessageText:
//
//  No incoming stream.
//
#define SE_NO_INCOMING_STREAM            0xC1000404L

//
// MessageId: SE_TABLE_INVALID_FILTER_SIZE
//
// MessageText:
//
//  The filter size is invalid.
//
#define SE_TABLE_INVALID_FILTER_SIZE     0xC1000405L

//
// MessageId: SE_TABLE_INVALID_FILTER_HANDLE
//
// MessageText:
//
//  The filter handle is invalid.
//
#define SE_TABLE_INVALID_FILTER_HANDLE   0xC1000406L

//
// MessageId: SE_PSI_PAT_NOT_PRESENT
//
// MessageText:
//
//  Program Association Table is not present.
//
#define SE_PSI_PAT_NOT_PRESENT           0xC3000407L

//
// MessageId: SE_PSI_NIT_NOT_PRESENT
//
// MessageText:
//
//  Network Information Table is not present.
//
#define SE_PSI_NIT_NOT_PRESENT           0xC3000408L

//
// MessageId: SE_PSI_PMT_NOT_PRESENT
//
// MessageText:
//
//  Program Map Table is not present for the service.
//
#define SE_PSI_PMT_NOT_PRESENT           0xC3000409L

//
// MessageId: SE_PSI_TDT_NOT_PRESENT
//
// MessageText:
//
//  Time Date Table is not present.
//
#define SE_PSI_TDT_NOT_PRESENT           0xC300040AL

//
// MessageId: SE_PSI_CAT_NOT_PRESENT
//
// MessageText:
//
//  Conditional Access Table was not present.
//
#define SE_PSI_CAT_NOT_PRESENT           0xC300040BL

//
// MessageId: SE_PSI_SERVICE_NOT_PRESENT_IN_PAT
//
// MessageText:
//
//  Service not present in the program association table.
//
#define SE_PSI_SERVICE_NOT_PRESENT_IN_PAT 0xC300040CL

//
// MessageId: SE_PSI_STREAM_NOT_PRESENT_IN_PMT
//
// MessageText:
//
//  Stream not present in the service.
//
#define SE_PSI_STREAM_NOT_PRESENT_IN_PMT 0xC300040DL

//
// MessageId: SE_UNKNOWN_STREAM_HANDLE
//
// MessageText:
//
//  The stream handle specified is unknown.
//
#define SE_UNKNOWN_STREAM_HANDLE         0xC1000501L

//
// MessageId: SE_STREAM_NOT_STOPPED
//
// MessageText:
//
//  The stream capture is not stopped.
//
#define SE_STREAM_NOT_STOPPED            0xC1000502L

//
// MessageId: SE_STREAM_NOT_STARTED
//
// MessageText:
//
//  The stream capture is not started.
//
#define SE_STREAM_NOT_STARTED            0xC1000503L

//
// MessageId: SE_STREAM_ALREADY_STARTED
//
// MessageText:
//
//  The stream capture is already started.
//
#define SE_STREAM_ALREADY_STARTED        0xC1000504L

//
// MessageId: SE_PROGRAM_ALREADY_OPENED
//
// MessageText:
//
//  The program capture is already opened.
//
#define SE_PROGRAM_ALREADY_OPENED        0xC1000505L

//
// MessageId: SE_PROGRAM_NOT_OPENED
//
// MessageText:
//
//  The program capture is not opened.
//
#define SE_PROGRAM_NOT_OPENED            0xC1000506L

//
// MessageId: SE_INVALID_STREAM_TYPE
//
// MessageText:
//
//  The specified stream type is invalid.
//
#define SE_INVALID_STREAM_TYPE           0xC1000507L

//
// MessageId: SE_TUNER_RSPND_INVALID_SYMBOL
//
// MessageText:
//
//  The specified symbol is invalid.
//
#define SE_TUNER_RSPND_INVALID_SYMBOL    0xC1000600L

//
// MessageId: SE_TUNER_RSPND_INVALID_FREQUENCY
//
// MessageText:
//
//  The specified frequency is invalid.
//
#define SE_TUNER_RSPND_INVALID_FREQUENCY 0xC1000601L

//
// MessageId: SE_TUNER_RSPND_INVALID_XPND
//
// MessageText:
//
//  An error was detected.
//
#define SE_TUNER_RSPND_INVALID_XPND      0xC1000602L

//
// MessageId: SE_TUNER_RSPND_INVALID_PID
//
// MessageText:
//
//  The specified PID is invalid.
//
#define SE_TUNER_RSPND_INVALID_PID       0xC1000603L

//
// MessageId: SE_TUNER_RSPND_INVALID_POLARITY
//
// MessageText:
//
//  The specified polarity is invalid.
//
#define SE_TUNER_RSPND_INVALID_POLARITY  0xC1000604L

//
// MessageId: SE_INVALID_COMMAND_ID
//
// MessageText:
//
//  The specified command id is invalid.
//
#define SE_INVALID_COMMAND_ID            0xC1000605L

//
// MessageId: SE_INVALID_TUNER_ID
//
// MessageText:
//
//  The specified tuner id is invalid.
//
#define SE_INVALID_TUNER_ID              0xC1000606L

//
// MessageId: SE_INVALID_TUNING_TYPE
//
// MessageText:
//
//  The specified tuning type is invalid.
//
#define SE_INVALID_TUNING_TYPE           0xC1000607L

//
// MessageId: SE_INVALID_TUNER_TYPE
//
// MessageText:
//
//  The specified tuner type is invalid.
//
#define SE_INVALID_TUNER_TYPE            0xC1000608L

//
// MessageId: SE_INVALID_ADDRESS
//
// MessageText:
//
//  An invalid address was detected.
//
#define SE_INVALID_ADDRESS               0xC1000609L

//
// MessageId: SE_TEST_SUCCEDED
//
// MessageText:
//
//  The specified test successfully completed.
//
#define SE_TEST_SUCCEDED                 0xC1000701L

//
// MessageId: SE_TEST_FAILED
//
// MessageText:
//
//  The specified test failed.
//
#define SE_TEST_FAILED                   0xC0FF0702L

//
// MessageId: SE_NO_SUCH_TEST
//
// MessageText:
//
//  The specified test does not exist.
//
#define SE_NO_SUCH_TEST                  0xC0FF0703L

//
// MessageId: SE_ADDRESS_OUTSIDE_RANGE
//
// MessageText:
//
//  An invalid address was detected.
//
#define SE_ADDRESS_OUTSIDE_RANGE         0xC1000704L

//
// MessageId: SE_SIZE_OUTSIDE_RANGE
//
// MessageText:
//
//  An invalid size was detected.
//
#define SE_SIZE_OUTSIDE_RANGE            0xC1000705L

//
// MessageId: SE_SIGNATURE_CHECK_FAILED
//
// MessageText:
//
//  An invalid signature was detected.
//
#define SE_SIGNATURE_CHECK_FAILED        0xC1000706L

//
// MessageId: SE_PID_NOT_ACTIVE
//
// MessageText:
//
//  The specified PID is not active.
//
#define SE_PID_NOT_ACTIVE                0xC1000707L

//
// MessageId: SE_FAILED_DOWNLOAD_MODE
//
// MessageText:
//
//  The adapter could not enter download mode.
//
#define SE_FAILED_DOWNLOAD_MODE          0xC0FF1000L

//
// MessageId: SE_CHECKSUM_ERROR
//
// MessageText:
//
//  A checksum error was detected.
//
#define SE_CHECKSUM_ERROR                0xC0FF1001L

//
// MessageId: SE_SIGNATURE_ERROR
//
// MessageText:
//
//  An invalid signature was detected.
//
#define SE_SIGNATURE_ERROR               0xC0FF1002L

//
// MessageId: SE_PROGRAM_FLASH_ERROR
//
// MessageText:
//
//  An error was detected while attempting to update the firmware.
//
#define SE_PROGRAM_FLASH_ERROR           0xC0FF1003L

//
// MessageId: SE_VERSION_ERROR
//
// MessageText:
//
//  An invalid version number was detected.
//
#define SE_VERSION_ERROR                 0xC0FF1004L

//
// MessageId: SE_CODE_SIZE_ERROR
//
// MessageText:
//
//  The code size is invalid.
//
#define SE_CODE_SIZE_ERROR               0xC0FF1005L

