#if !defined(_SHELL_API_H_)
#define _SHELL_API_H_

#include <pal/palcommon.h>
#include <MI.h>

typedef void * HANDLE;

typedef struct _WSMAN_DATA_TEXT
{
    MI_Uint32 bufferLength;
    _In_reads_(bufferLength) const MI_Char * buffer;
}WSMAN_DATA_TEXT;

typedef struct _WSMAN_DATA_BINARY
{
    MI_Uint32 dataLength;
    _In_reads_(dataLength) MI_Uint8 *data;
}WSMAN_DATA_BINARY;

enum WSManDataType
{
    WSMAN_DATA_NONE                = 0,
    WSMAN_DATA_TYPE_TEXT           = 1,
    WSMAN_DATA_TYPE_BINARY         = 2,
    WSMAN_DATA_TYPE_DWORD          = 4
};
typedef enum WSManDataType WSManDataType;

typedef struct _WSMAN_DATA
{
    WSManDataType type;
    union
    {
        WSMAN_DATA_TEXT text;
        WSMAN_DATA_BINARY binaryData;
        MI_Uint32 number;
    };
} WSMAN_DATA;

typedef struct _WSMAN_OPTION
{
    const MI_Char * name;
    const MI_Char * value;
    BOOL mustComply;
} WSMAN_OPTION;

typedef struct _WSMAN_OPTION_SET
{
    MI_Uint32 optionsCount;
    _In_reads_opt_(optionsCount) WSMAN_OPTION *options;
    BOOL optionsMustUnderstand;
} WSMAN_OPTION_SET;

typedef struct _WSMAN_KEY
{
    const MI_Char * key;
    const MI_Char * value;
} WSMAN_KEY;

typedef struct _WSMAN_SELECTOR_SET
{
    MI_Uint32 numberKeys; // Number of keys (selectors)
    _In_reads_opt_(numberKeys) WSMAN_KEY *keys;  // Array of key names and values
} WSMAN_SELECTOR_SET;


typedef struct _WSMAN_FRAGMENT
{
    _In_ const MI_Char * path;               // fragment path - WS-Transfer
    _In_opt_ const MI_Char * dialect;        // dialect for Fragment path

} WSMAN_FRAGMENT;

typedef struct _WSMAN_FILTER
{
    _In_ const MI_Char * filter;              // filter enumeration/subscription - allows ad-hoc queries using quey languages like SQL
    _In_opt_ const MI_Char * dialect;         // dialect for filter predicate

} WSMAN_FILTER;

typedef struct _WSMAN_OPERATION_INFO
{
    _In_opt_ WSMAN_FRAGMENT fragment;              // optional element to support Fragment transfer or
    _In_opt_ WSMAN_FILTER filter;                  // optional Filter WS-Enumerate/WS-Eventing
    _In_opt_ WSMAN_SELECTOR_SET selectorSet;
    _In_opt_ WSMAN_OPTION_SET optionSet;
    _In_opt_ void *reserved;
             MI_Uint32 version;
} WSMAN_OPERATION_INFO;

typedef struct _WSMAN_CERTIFICATE_DETAILS
{
    const MI_Char * subject;         // Certificate subject in distinguished form
                            // Ex: "CN=xyz.com, OU=xyz management, O=Microsoft, L=Redmond, S=Washington, C=US"
    const MI_Char * issuerName;      // Certificate issuer in distinguished form
                            // Ex: "CN=Microsoft Secure Server Authority, DC=redmond, DC=corp, DC=microsoft, DC=com"
    const MI_Char * issuerThumbprint;// Thumbprint of Certificate issuer
    const MI_Char * subjectName;     // Certificate Subject Alternative Name (SAN) if available or subject Common Name (CN)
                            // Ex: "xyz.com"
} WSMAN_CERTIFICATE_DETAILS;

typedef struct _WSMAN_SENDER_DETAILS
{
    const MI_Char * senderName;                  // Username of the sender
    const MI_Char * authenticationMechanism;      //
    WSMAN_CERTIFICATE_DETAILS *certificateDetails; // valid only if the authentication is client certificates
    HANDLE clientToken;
    const MI_Char * httpURL;
} WSMAN_SENDER_DETAILS;

typedef struct _WSMAN_PLUGIN_REQUEST
{
    WSMAN_SENDER_DETAILS *senderDetails;
    const MI_Char * locale;
    const MI_Char * resourceUri;
    WSMAN_OPERATION_INFO *operationInfo;
    volatile BOOL shutdownNotification;
    HANDLE shutdownNotificationHandle;
    const MI_Char * dataLocale;
} WSMAN_PLUGIN_REQUEST;



/* ===================================================================
 *  * Following APIs are callbacks from the _PLUGIN_
 * APIs back into the infrastructure to report
 * results and contexts, or to query extra
 * information about the shell
 * ===================================================================
 */
MI_Uint32 MI_CALL WSManPluginReportContext(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * context
    );
#define WSMAN_SHELL_NS PAL_T("http://schemas.microsoft.com/wbem/wsman/1/windows/shell")
#define WSMAN_COMMAND_STATE_DONE WSMAN_SHELL_NS PAL_T("/CommandState/Done")
#define WSMAN_COMMAND_STATE_PENDING WSMAN_SHELL_NS PAL_T("/CommandState/Pending")
#define WSMAN_COMMAND_STATE_RUNNING WSMAN_SHELL_NS PAL_T("/CommandState/Running")

#define WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA 1
#define WSMAN_FLAG_RECEIVE_FLUSH               2
#define WSMAN_FLAG_RECEIVE_RESULT_DATA_BOUNDARY 4

MI_Uint32 MI_CALL WSManPluginReceiveResult(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_opt_ const MI_Char * stream,
    _In_opt_ WSMAN_DATA *streamResult,
    _In_opt_ const MI_Char * commandState,
    _In_ MI_Uint32 exitCode
    );

MI_Uint32 MI_CALL WSManPluginOperationComplete(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ MI_Uint32 errorCode,
    _In_opt_ const MI_Char * extendedInformation
    );

#define WSMAN_PLUGIN_PARAMS_MAX_ENVELOPE_SIZE 1
#define WSMAN_PLUGIN_PARAMS_TIMEOUT 2
#define WSMAN_PLUGIN_PARAMS_REMAINING_RESULT_SIZE 3
#define WSMAN_PLUGIN_PARAMS_LARGEST_RESULT_SIZE 4
#define WSMAN_PLUGIN_PARAMS_GET_REQUESTED_LOCALE 5 /* Returns WSMAN_DATA_TEXT */
#define WSMAN_PLUGIN_PARAMS_GET_REQUESTED_DATA_LOCALE 6 /* Returns WSMAN_DATA_TEXT */


MI_Uint32 MI_CALL WSManPluginGetOperationParameters (
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _Out_ WSMAN_DATA *data
    );

MI_Uint32 MI_CALL WSManPluginGetConfiguration (
  _In_   void * pluginContext,
  _In_   MI_Uint32 flags,
  _Out_  WSMAN_DATA *data
);

MI_Uint32 MI_CALL WSManPluginReportCompletion(
  _In_      void * pluginContext,
  _In_      MI_Uint32 flags
);

MI_Uint32 MI_CALL WSManPluginFreeRequestDetails(_In_ WSMAN_PLUGIN_REQUEST *requestDetails);

#define WSMAN_PLUGIN_STARTUP_REQUEST_RECEIVED     0x0
#define WSMAN_PLUGIN_STARTUP_AUTORESTARTED_REBOOT 0x1
#define WSMAN_PLUGIN_STARTUP_AUTORESTARTED_CRASH  0x2

#define WSMAN_PLUGIN_SHUTDOWN_SYSTEM  1
#define WSMAN_PLUGIN_SHUTDOWN_SERVICE 2
#define WSMAN_PLUGIN_SHUTDOWN_IISHOST 3
#define WSMAN_PLUGIN_SHUTDOWN_IDLETIMEOUT_ELAPSED 4

typedef struct _WSMAN_STREAM_ID_SET
{
    MI_Uint32 streamIDsCount;
    _In_reads_opt_(streamIDsCount) const MI_Char * *streamIDs;

} WSMAN_STREAM_ID_SET;
typedef struct _WSMAN_ENVIRONMENT_VARIABLE
{
    const MI_Char * name;
    const MI_Char * value;

} WSMAN_ENVIRONMENT_VARIABLE;
typedef struct _WSMAN_ENVIRONMENT_VARIABLE_SET
{
    MI_Uint32 varsCount;
    _In_reads_opt_(varsCount) WSMAN_ENVIRONMENT_VARIABLE *vars;

} WSMAN_ENVIRONMENT_VARIABLE_SET;
typedef struct _WSMAN_SHELL_STARTUP_INFO
{
    WSMAN_STREAM_ID_SET *inputStreamSet;
    WSMAN_STREAM_ID_SET *outputStreamSet;
    MI_Uint32 idleTimeoutMs;
    const MI_Char * workingDirectory;
    WSMAN_ENVIRONMENT_VARIABLE_SET *variableSet;    
    const MI_Char * name;
} WSMAN_SHELL_STARTUP_INFO;

typedef struct _WSMAN_COMMAND_ARG_SET
{
    MI_Uint32 argsCount;
    _In_reads_opt_(argsCount) const MI_Char * *args;

} WSMAN_COMMAND_ARG_SET;


/* ===================================================================
 * Following functions are plug-in APIs that the shell infrastructure
 * calls into
 * ===================================================================
 */

MI_Uint32 MI_CALL WSManPluginStartup(
    _In_ MI_Uint32 flags,
    _In_ const MI_Char * applicationIdentification,
    _In_opt_ const MI_Char * extraInfo,
    _Out_ void * *pluginContext
    );

MI_Uint32 MI_CALL WSManPluginShutdown(
    _In_opt_ void * pluginContext,
    _In_ MI_Uint32 flags,
    _In_ MI_Uint32 reason
    );

void MI_CALL WSManPluginShell(
    _In_ void * pluginContext,   //Relates to context returned from WSMAN_PLUGIN_STARTUP
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_opt_ WSMAN_SHELL_STARTUP_INFO *startupInfo,
    _In_opt_ WSMAN_DATA *inboundShellInformation
    );

void MI_CALL WSManPluginCommand(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * shellContext,
    _In_ const MI_Char * commandLine,
    _In_opt_ WSMAN_COMMAND_ARG_SET *arguments
    );

#define WSMAN_FLAG_SEND_NO_MORE_DATA 1

void MI_CALL WSManPluginSend(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * shellContext,
    _In_opt_ void * commandContext,
    _In_ const MI_Char * stream,
    _In_ WSMAN_DATA *inboundData
    );

void MI_CALL WSManPluginReceive(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * shellContext,
    _In_opt_ void * commandContext,
    _In_opt_ WSMAN_STREAM_ID_SET *streamSet
    );

/* Default signal codes where _EXIT comes from the client to confirm the command has completed and is the last request targeting the command */
#define WSMAN_SIGNAL_CODE_TERMINATE WSMAN_SHELL_NS PAL_T("/signal/Terminate") 
#define WSMAN_SIGNAL_CODE_BREAK     WSMAN_SHELL_NS PAL_T("/signal/Break")
#define WSMAN_SIGNAL_CODE_PAUSE     WSMAN_SHELL_NS PAL_T("/signal/Pause")
#define WSMAN_SIGNAL_CODE_RESUME    WSMAN_SHELL_NS PAL_T("/signal/Resume")
#define WSMAN_SIGNAL_CODE_EXIT      WSMAN_SHELL_NS PAL_T("/signal/Exit")

void MI_CALL WSManPluginSignal(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * shellContext,
    _In_opt_ void * commandContext,
    _In_ const MI_Char * code
    );

void MI_CALL WSManPluginConnect(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * shellContext,
    _In_opt_ void * commandContext,
    _In_opt_ WSMAN_DATA *inboundConnectInformation
     );

void MI_CALL WSManPluginReleaseShellContext(
    _In_ void * shellContext
    );

void MI_CALL WSManPluginReleaseCommandContext(
    _In_ void * shellContext,
    _In_ void * commandContext
    );

#endif /* _SHELL_API_H_ */
