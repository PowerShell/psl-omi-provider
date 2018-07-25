/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include <iconv.h>
#include <stdlib.h>
#include <pal/strings.h>
#include <pal/atomic.h>
#include <base/result.h>
#include <base/logbase.h>
#include <base/log.h>
#include <base/helpers.h>
#include <MI.h>
#include "wsman.h"
#include "BufferManipulation.h"
#include "Shell.h"
#include "Command.h"
#include "DesiredStream.h"
#include "Utilities.h"

/* Disable the provider APIs so we can use the provider RTTI */
void MI_CALL Shell_Load(Shell_Self** self, MI_Module_Self* selfModule, MI_Context* context) {}
void MI_CALL Shell_Unload(Shell_Self* self, MI_Context* context) {}
void MI_CALL Shell_EnumerateInstances(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_PropertySet* propertySet, MI_Boolean keysOnly, const MI_Filter* filter) {}
void MI_CALL Shell_GetInstance(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Shell* instanceName, const MI_PropertySet* propertySet){}
void MI_CALL Shell_CreateInstance(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Shell* newInstance){}
void MI_CALL Shell_ModifyInstance(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Shell* modifiedInstance, const MI_PropertySet* propertySet) {}
void MI_CALL Shell_DeleteInstance(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Shell* instanceName){}
void MI_CALL Shell_Invoke_Command(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Command* in){}
void MI_CALL Shell_Invoke_Send(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Send* in){}
void MI_CALL Shell_Invoke_Receive(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Receive* in){}
void MI_CALL Shell_Invoke_Signal(Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Signal* in){}
void MI_CALL Shell_Invoke_Disconnect( Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Disconnect* in){}
void MI_CALL Shell_Invoke_Reconnect( Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Reconnect* in){}
void MI_CALL Shell_Invoke_Connect( Shell_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_Char* methodName, const Shell* instanceName, const Shell_Connect* in){}
void MI_CALL Command_Load( Command_Self** self, MI_Module_Self* selfModule, MI_Context* context){}
void MI_CALL Command_Unload( Command_Self* self, MI_Context* context){}
void MI_CALL Command_EnumerateInstances( Command_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const MI_PropertySet* propertySet, MI_Boolean keysOnly, const MI_Filter* filter){}
void MI_CALL Command_GetInstance( Command_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Command* instanceName, const MI_PropertySet* propertySet){}
void MI_CALL Command_CreateInstance( Command_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Command* newInstance){}
void MI_CALL Command_ModifyInstance( Command_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Command* modifiedInstance, const MI_PropertySet* propertySet){}
void MI_CALL Command_DeleteInstance( Command_Self* self, MI_Context* context, const MI_Char* nameSpace, const MI_Char* className, const Command* instanceName){}


/* Note: Change logging level in omiserver.conf */
#define SHELL_LOGGING_FILE "shellclient"


#define GOTO_ERROR(message, result) { miResult = result; errorMessage=message; __LOGE(("%s (result=%u)", errorMessage, miResult)); goto error; }

static void LogFunctionStart(const char *function)
{
    __LOGD(("%s: START", function));
}
static void LogFunctionEnd(const char *function, MI_Result miResult)
{
    __LOGD(("%s: END, miResult=%u (%s)", function, miResult, Result_ToString(miResult)));
}

struct WSMAN_API
{
    MI_Application application;
};

struct WSMAN_SESSION
{
    WSMAN_API_HANDLE api;
    Batch *batch;
    char *hostname;
    MI_DestinationOptions destinationOptions;
    MI_Char *redirectLocation;
};

struct WSMAN_SHELL
{
    WSMAN_SESSION_HANDLE session;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    MI_OperationCallbacks callbacks;
    Shell *shellInstance;
    MI_Session miSession;
    MI_Operation miCreateShellOperation;
    MI_Operation miDeleteShellOperation;
    MI_OperationOptions operationOptions;
    MI_Boolean didCreate;
};

struct WSMAN_COMMAND
{
    WSMAN_SHELL_HANDLE shell;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    MI_OperationCallbacks callbacks;
    MI_Operation miOperation;
    MI_OperationOptions miOptions;
    MI_Instance *commandProperties;
    MI_Instance *commandClose;
    char *commandId;
};

typedef enum
{
        WSMAN_OPERATION_SEND = 1,
        WSMAN_OPERATION_RECEIVE = 2,
        WSMAN_OPERATION_SIGNAL = 3
} WSMAN_OPERATION_TYPE;
struct WSMAN_OPERATION
{
    WSMAN_OPERATION_TYPE type;
    WSMAN_SHELL_HANDLE shell;
    WSMAN_COMMAND_HANDLE command;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    MI_OperationCallbacks callbacks;
    MI_Operation miOperation;
    MI_OperationOptions miOptions;
    MI_Instance *operationProperties;
};


MI_EXPORT MI_Uint32 WINAPI WSManInitialize(
    MI_Uint32 flags,
    _Out_ WSMAN_API_HANDLE *apiHandle
    )
{
    MI_Result miResult;

    _GetLogOptionsFromConfigFile(SHELL_LOGGING_FILE);

    LogFunctionStart("WSManInitialize");

    (*apiHandle) = calloc(1, sizeof(struct WSMAN_API));
    if (*apiHandle == NULL)
        return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    miResult = MI_Application_InitializeV1(0, NULL, NULL, &(*apiHandle)->application);
    if (miResult != MI_RESULT_OK)
    {
        free(*apiHandle);
        *apiHandle = NULL;
    }
    LogFunctionEnd("WSManInitialize", miResult);
    return miResult;
}


MI_EXPORT MI_Uint32 WINAPI WSManDeinitialize(
    _Inout_opt_ WSMAN_API_HANDLE apiHandle,
    MI_Uint32 flags
    )
{
    LogFunctionStart("WSManDeinitialize");
    if (apiHandle)
    {
        MI_Application_Close(&apiHandle->application);
        free(apiHandle);
    }
    LogFunctionEnd("WSManDeinitialize", MI_RESULT_OK);

    Log_Close();
    return MI_RESULT_OK;
}

MI_EXPORT MI_Uint32 WINAPI WSManGetErrorMessage(
    _In_ WSMAN_API_HANDLE apiHandle,
    MI_Uint32 flags,            // reserved for future use; must be 0
    _In_opt_ const MI_Char16* languageCode,    // the RFC 3066 language code; it can be NULL
    MI_Uint32 errorCode,        // error code for the requested error message
    MI_Uint32 messageLength,    // message length, including NULL terminator
    _Out_writes_to_opt_(messageLength, *messageLengthUsed) MI_Char16*  message,
    _Out_ MI_Uint32* messageLengthUsed  // effective message length, including NULL terminator
    )
{
    const char *resultString = Result_ToString(errorCode);
    size_t resultStringLenth = Tcslen(resultString)+1; /* Includes null terminator */
    MI_Result miResult = MI_RESULT_OK;
    size_t tmpMessageLen;
    char *errorMessage = NULL;
    size_t iconv_return;
    iconv_t iconvData;

    //MI_Char16 *convertedString;
    //Utf8ToUtf16Le(batch, resultString, &convertedString);
    __LOGD(("%s: START, errorCode=%u, messageLength=%u", "WSManGetErrorMessage", errorCode, messageLength));
    if ((messageLength == 0) || (message == NULL))
    {
        *messageLengthUsed = (MI_Uint32) resultStringLenth;
        return ERROR_INSUFFICIENT_BUFFER;
    }

    iconvData = iconv_open("UTF-16LE", "UTF-8" );
    if (iconvData == (iconv_t)-1)
    {
        GOTO_ERROR("Failed to convert stream", MI_RESULT_FAILED);
    }


    tmpMessageLen = messageLength*2; /* Convert to bytes */

    iconv_return = iconv(iconvData,
            (char**)&resultString, &resultStringLenth,
            (char**)&message, &tmpMessageLen);
    if (iconv_return == (size_t) -1)
    {
        iconv_close(iconvData);
        GOTO_ERROR("Failed to convert stream", MI_RESULT_FAILED);
    }

    iconv_close(iconvData);


    *messageLengthUsed = messageLength - (tmpMessageLen / 2);

error:
    LogFunctionEnd("WSManGetErrorMessage", miResult);
    return miResult;
}

MI_EXPORT MI_Uint32 WINAPI WSManCreateSession(
    _In_ WSMAN_API_HANDLE apiHandle,
    _In_opt_ const MI_Char16* _connection,         // if NULL, then connection will default to localhost
    MI_Uint32 flags,
    _In_opt_ WSMAN_AUTHENTICATION_CREDENTIALS *serverAuthenticationCredentials,
    _In_opt_ WSMAN_PROXY_INFO *proxyInfo,
    _Out_ WSMAN_SESSION_HANDLE *session
    )
{
    MI_Result miResult;
    char *errorMessage = NULL;
    Batch *batch = NULL;
    char *connection = NULL;
    char *portNumber = NULL;
    char *httpUrl = NULL;
    char *username = NULL;
    char *password = NULL;
    MI_UserCredentials userCredentials;

    LogFunctionStart("WSManCreateSession");
    *session = NULL;

    if (serverAuthenticationCredentials == NULL)
    {
        GOTO_ERROR("No authentication credentials given", MI_RESULT_ACCESS_DENIED);
    }
    if (proxyInfo)
    {
        GOTO_ERROR("Don't support proxy information", MI_RESULT_INVALID_PARAMETER);
    }

    switch (serverAuthenticationCredentials->authenticationMechanism)
    {
        case WSMAN_FLAG_AUTH_BASIC:
            userCredentials.authenticationType = MI_AUTH_TYPE_BASIC;
            break;
        case WSMAN_FLAG_AUTH_NEGOTIATE:
            userCredentials.authenticationType = MI_AUTH_TYPE_NEGO_WITH_CREDS;
            break;
        case WSMAN_FLAG_AUTH_KERBEROS:
            userCredentials.authenticationType = MI_AUTH_TYPE_KERBEROS;
            break;
        default:
            GOTO_ERROR("Unsupported authentication type", MI_RESULT_ACCESS_DENIED);
            break;

    }

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("Out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*session) = Batch_GetClear(batch, sizeof(struct WSMAN_SESSION));
    if (*session == NULL)
    {
        GOTO_ERROR("Out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*session)->batch = batch;

    miResult = MI_Application_NewDestinationOptions(&apiHandle->application, &(*session)->destinationOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Destination options creation failed", miResult);
    }

    /* powershell uses HTTP unless otherwise stated */
    miResult = MI_DestinationOptions_SetTransport(&(*session)->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTP);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to default http transport", miResult);
    }
    /* Packet privacy over HTTP by default unless otherwise stated */
    miResult = MI_DestinationOptions_SetPacketPrivacy(&(*session)->destinationOptions, MI_TRUE);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set packet privacy", miResult);
    }

    if (_connection && !Utf16LeToUtf8(batch, _connection, &connection))
    {
        GOTO_ERROR("Failed to convert connection name", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    else if (_connection == NULL)
    {
        connection = "localhost";
    }

    /* Full format may be:
     *      <transport>://<computerName>:<port><httpUrl>
     * where:
     *      <transport>:// = http:// or https://, if <transport>:// is missing it defaults
     *      :<port> = port to use based on transport specified, if missing uses defaults
     *      <httpUrl> = http URL to post to, if missing defaults to /wsman/
     *
     * For now, assume there is a http url at the end copy if off and remove so we just have the machine name in the connection string
     *
     */
    if (strncmp(connection, "http://", 7) == 0)
    {
        miResult = MI_DestinationOptions_SetTransport(&(*session)->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTP);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set transport to http", miResult);
        }
        /* If http:// connection string is used we need to default to port 80, rather than default wsman http port */
        miResult = MI_DestinationOptions_SetDestinationPort(&(*session)->destinationOptions, 80);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set transport to http", miResult);
        }
        connection += 7;
     }
    else if (strncmp(connection, "https://", 8) == 0)
    {
        miResult = MI_DestinationOptions_SetTransport(&(*session)->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTPS);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set transport to https", miResult);
        }
        /* If https:// connection string is used we need to default to port 443, rather than default wsman https port */
        miResult = MI_DestinationOptions_SetDestinationPort(&(*session)->destinationOptions, 443);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set transport to http", miResult);
        }
        connection += 8;
    }
    else
    {
        /* Assume no prefix and starts with computer name */
    }

    (*session)->hostname = connection;

    portNumber = strchr(connection, ':');
    httpUrl = strchr(connection, '/');

    if (httpUrl == NULL)
        httpUrl = "/wsman/";
    else
    {
        /* Need to copy because we need the initial slash, but need to terminate the hostname at that location. */
        char *tmp = Batch_Tcsdup(batch, httpUrl);
        if (tmp == NULL)
        {
            GOTO_ERROR("Failed to convert connection name", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        *httpUrl = '\0'; /* terminate hostname or port number string properly */
        httpUrl = tmp;
    }
    miResult = MI_DestinationOptions_SetHttpUrlPrefix(&(*session)->destinationOptions, httpUrl);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to add http prefix to destination options", miResult);
    }


    if ((portNumber && !httpUrl) || (portNumber && httpUrl && portNumber < httpUrl))
    {
        MI_Uint32 portNumberValue = 0;
        *portNumber = '\0'; /* null terminate hostname in correct place */
        portNumber ++;  /* move past : */
        if (StrToUint32(portNumber, &portNumberValue) != 0)
        {
            GOTO_ERROR("Failed to parse port number in connection uri", MI_RESULT_INVALID_PARAMETER);
        }
        miResult = MI_DestinationOptions_SetDestinationPort(&(*session)->destinationOptions, portNumberValue);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set transport to http", miResult);
        }
    }

    if (serverAuthenticationCredentials->userAccount.username && !Utf16LeToUtf8(batch, serverAuthenticationCredentials->userAccount.username, &username))
    {
        GOTO_ERROR("Username missing or failed to convert", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (serverAuthenticationCredentials->userAccount.password && !Utf16LeToUtf8(batch, serverAuthenticationCredentials->userAccount.password, &password))
    {
        GOTO_ERROR("password missing or failed to convert", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    userCredentials.credentials.usernamePassword.domain = NULL; /* Assume for now no domain. At some point we may need to split username */
    userCredentials.credentials.usernamePassword.username = username;
    userCredentials.credentials.usernamePassword.password = password;

    if (MI_DestinationOptions_SetMaxEnvelopeSize(&(*session)->destinationOptions, 500))
    {
        GOTO_ERROR("Failed to add credentials to destination options", miResult);
    }

    miResult = MI_DestinationOptions_AddDestinationCredentials(&(*session)->destinationOptions, &userCredentials);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to add credentials to destination options", miResult);
    }
    (*session)->api = apiHandle;

    LogFunctionEnd("WSManCreateSession", miResult);
    return miResult;

error:
    if (batch)
        Batch_Delete(batch);

    *session = NULL;

    LogFunctionEnd("WSManCreateSession", miResult);
    return miResult;
}

MI_EXPORT MI_Uint32 WINAPI WSManCloseSession(
    _Inout_opt_ WSMAN_SESSION_HANDLE session,
    MI_Uint32 flags)
{
    MI_Result miResult = MI_RESULT_OK;

    LogFunctionStart("WSManCloseSession");
    if (session->destinationOptions.ft)
        MI_DestinationOptions_Delete(&session->destinationOptions);

    Batch_Delete(session->batch);

    LogFunctionEnd("WSManCloseSession", miResult);
    return miResult;
}

MI_EXPORT MI_Uint32 WINAPI WSManSetSessionOption(
    _In_ WSMAN_SESSION_HANDLE session,
    WSManSessionOption option,
    _In_ WSMAN_DATA *data)
{
    MI_Result miResult;
    char *errorMessage = NULL;

    LogFunctionStart("WSManSetSessionOption");
    switch (option)
    {
        case WSMAN_OPTION_USE_SSL:
            __LOGD(("WSMAN_OPTION_USE_SSL"));
            if (data->type != WSMAN_DATA_TYPE_DWORD)
                GOTO_ERROR("SSL option should be DWORD", MI_RESULT_INVALID_PARAMETER);
            if (data->number == 0)
            {
                miResult = MI_DestinationOptions_SetTransport(&session->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTP);
            }
            else
            {
                miResult = MI_DestinationOptions_SetTransport(&session->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTPS);
            }
            break;

        case WSMAN_OPTION_UI_LANGUAGE:
        {
            /* String, convert utf16->utf8, locale */
            MI_Char *tmpStr;
            __LOGD(("WSMAN_OPTION_UI_LANGUAGE"));
            if ((data->type != WSMAN_DATA_TYPE_TEXT) ||
                    data->text.buffer == NULL)
            {
                GOTO_ERROR("UI language option is wrong type or NULL", MI_RESULT_INVALID_PARAMETER);
            }
            if (!Utf16LeToUtf8(session->batch, data->text.buffer, &tmpStr))
                GOTO_ERROR("Failed to convert UI language", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            if (MI_DestinationOptions_SetUILocale(&session->destinationOptions, tmpStr) != MI_RESULT_OK)
            {
                GOTO_ERROR("Failed to set UI language option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            miResult = MI_RESULT_OK;
            break;
        }
        case WSMAN_OPTION_LOCALE:
        {
            /* String, convert utf16->utf8, locale */
            MI_Char *tmpStr;
            __LOGD(("WSMAN_OPTION_LOCALE"));
            if ((data->type != WSMAN_DATA_TYPE_TEXT) ||
                    data->text.buffer == NULL)
            {
                GOTO_ERROR("Data locale option is wrong type or NULL", MI_RESULT_INVALID_PARAMETER);
            }
            if (!Utf16LeToUtf8(session->batch, data->text.buffer, &tmpStr))
                GOTO_ERROR("Failed to convert Data locale", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            if (MI_DestinationOptions_SetDataLocale(&session->destinationOptions, tmpStr) != MI_RESULT_OK)
            {
                GOTO_ERROR("Failed to set Data locale option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            miResult = MI_RESULT_OK;
            break;
        }
        case WSMAN_OPTION_DEFAULT_OPERATION_TIMEOUTMS:
        {
            MI_Uint64 microsec = data->number * 1000;
            MI_Datetime datetime;
            __LOGD(("WSMAN_OPTION_DEFAULT_OPERATION_TIMEOUTMS=%u",data->number));
            UsecToDatetime(microsec, &datetime);
            MI_DestinationOptions_SetTimeout(&session->destinationOptions, &datetime.u.interval);
            /* dword, operation timeout when not the others */
            miResult = MI_RESULT_OK;
            break;
        }
        case WSMAN_OPTION_TIMEOUTMS_CREATE_SHELL:
            __LOGD(("WSMAN_OPTION_TIMEOUTMS_CREATE_SHELL=%u (not implemented yet)", data->number));
            /* dword */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_TIMEOUTMS_CLOSE_SHELL:
            __LOGD(("WSMAN_OPTION_TIMEOUTMS_CLOSE_SHELL=%u (not implemented yet)", data->number));
            /* dword */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_TIMEOUTMS_SIGNAL_SHELL:
            __LOGD(("WSMAN_OPTION_TIMEOUTMS_SIGNAL_SHELL=%u (not implemented yet)", data->number));
            /* dword */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_SHELL_MAX_DATA_SIZE_PER_MESSAGE_KB:
            __LOGD(("WSMAN_OPTION_SHELL_MAX_DATA_SIZE_PER_MESSAGE_KB=%u", data->number));
            /* dword */
            if ((data->type != WSMAN_DATA_TYPE_DWORD) ||
               (MI_DestinationOptions_SetMaxEnvelopeSize(&session->destinationOptions, data->number) != MI_RESULT_OK))
            {
                GOTO_ERROR("Failed to add credentials to destination options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_UNENCRYPTED_MESSAGES:
            __LOGD(("WSMAN_OPTION_UNENCRYPTED_MESSAGES=%u", data->number));
            /* dword */
            if (data->type == WSMAN_DATA_TYPE_DWORD)
            {
                if (data->number)
                {
                    if (MI_DestinationOptions_SetPacketPrivacy(&session->destinationOptions, MI_FALSE) != MI_RESULT_OK)
                    {
                        GOTO_ERROR("Failed to turn packet privacy off", MI_RESULT_SERVER_LIMITS_EXCEEDED);
                    }
                }
                else
                {
                    if (MI_DestinationOptions_SetPacketPrivacy(&session->destinationOptions, MI_TRUE) != MI_RESULT_OK)
                    {
                        GOTO_ERROR("Failed to turn packet privacy on", MI_RESULT_SERVER_LIMITS_EXCEEDED);
                    }
                }
            }
            else
            {
                GOTO_ERROR("Failed to set packet privacy, invalid parameter", MI_RESULT_INVALID_PARAMETER);
            }
            miResult = MI_RESULT_OK;
            break;
        default:
            __LOGD(("ignored option %u", option));
            miResult = MI_RESULT_OK;    /* Assume we can ignore it */
    }

error:
    LogFunctionEnd("WSManSetSessionOption", miResult);
    return miResult;
}

MI_EXPORT MI_Uint32 WINAPI WSManGetSessionOptionAsDword(
    _In_ WSMAN_SESSION_HANDLE session, /* NOTE: This may be an application handle */
    WSManSessionOption option,
    _Inout_ MI_Uint32 *value)
{
    MI_Uint32 miResult = MI_RESULT_OK;

    LogFunctionStart("WSManGetSessionOptionAsDword");
    switch (option)
    {
        case WSMAN_OPTION_SHELL_MAX_DATA_SIZE_PER_MESSAGE_KB:
            *value = 500; /* TODO: Proper value? */
            __LOGD(("WSMAN_OPTION_SHELL_MAX_DATA_SIZE_PER_MESSAGE_KB returning 500"));
            break;

        case WSMAN_OPTION_MAX_RETRY_TIME:
            *value = 60; /* TODO: Proper value? */
            __LOGD(("WSMAN_OPTION_SHELL_MAX_DATA_SIZE_PER_MESSAGE_KB returning 60"));
            break;

        default:
            miResult = MI_RESULT_NOT_SUPPORTED;
            __LOGD(("unknown option %u", option));
    }
    LogFunctionEnd("WSManGetSessionOptionAsDword", miResult);
    return miResult;
}

MI_EXPORT MI_Uint32 WINAPI WSManGetSessionOptionAsString(
    _In_ WSMAN_SESSION_HANDLE session,
    WSManSessionOption option,
    MI_Uint32 stringLength,
    _Out_writes_to_opt_(stringLength, *stringLengthUsed) MI_Char16* string,
    _Out_ MI_Uint32* stringLengthUsed)
{
    MI_Uint32 miResult = MI_RESULT_OK;

    LogFunctionStart("WSManGetSessionOptionAsString");
    switch (option)
    {
        case WSMAN_OPTION_REDIRECT_LOCATION:
        {
            if ((string == NULL) && (session->redirectLocation) && (stringLengthUsed))
            {
                *stringLengthUsed = Tcslen(session->redirectLocation) + 1;
                __LOGD(("Redirect location: returning string length of %u", *stringLengthUsed));
                miResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else if (string && stringLengthUsed && (session->redirectLocation) && (stringLength > Tcslen(session->redirectLocation)))
            {
                size_t iconv_return;
                iconv_t iconvData;
                size_t tmpToStringLen;
                size_t tmpFromStringLen;
                char *tmpFromString = session->redirectLocation;
                char *tmpToString = (char *)string;

                iconvData = iconv_open("UTF-16LE", "UTF-8" );
                if (iconvData == (iconv_t)-1)
                {
                    miResult = MI_RESULT_FAILED;
                }
                else
                {
                    tmpFromStringLen = (Tcslen(session->redirectLocation) + 1);
                    tmpToStringLen = stringLength * 2;

                    iconv_return = iconv(iconvData,
                            &tmpFromString, &tmpFromStringLen,
                            &tmpToString, &tmpToStringLen);
                    if (iconv_return == (size_t) -1)
                    {
                        miResult = MI_RESULT_FAILED;
                    }
                    else
                    {
                        *stringLengthUsed = (MI_Uint32) Tcslen(session->redirectLocation) + 1;
                        __LOGE(("Redirect location: returning location: %s (length %u)",session->redirectLocation,  Tcslen(session->redirectLocation) + 1));
                    }

                    iconv_close(iconvData);
                }
            }
            else
            {
                /* invalid parameter */
                miResult = MI_RESULT_INVALID_PARAMETER;
                __LOGE(("Redirect location: Parameters not correct for retrieving string"));
            }
            break;
        }
        default:
            miResult = MI_RESULT_NOT_SUPPORTED;
            __LOGD(("unknown option %u", option));
    }
    LogFunctionEnd("WSManGetSessionOptionAsString", miResult);
    return miResult;
}
MI_EXPORT MI_Uint32 WINAPI WSManCloseOperation(
    _Inout_opt_ WSMAN_OPERATION_HANDLE operationHandle,
    MI_Uint32 flags)
{
    LogFunctionStart("WSManCloseOperation");
    if (operationHandle->type == WSMAN_OPERATION_RECEIVE && operationHandle->miOperation.ft)
    {
        MI_Operation_Cancel(&operationHandle->miOperation, MI_REASON_NONE);
    }

    /* Wait for operation to complete and delete memory */

    LogFunctionEnd("WSManCloseOperation", MI_RESULT_NOT_SUPPORTED);
    return MI_RESULT_OK;
}

/*
typedef struct _Shell
{
    MI_Instance __instance;
    MI_ConstStringField ShellId;
    MI_ConstStringField Name;
    MI_ConstStringField ResourceUri;
    MI_ConstStringField Owner;
    MI_ConstStringField ClientIP;
    MI_ConstUint32Field ProcessId;
    MI_ConstDatetimeField IdleTimeout;
    MI_ConstStringField InputStreams;
    MI_ConstStringField OutputStreams;
    MI_ConstDatetimeField MaxIdleTimeout;
    MI_ConstStringField Locale;
    MI_ConstStringField DataLocale;
    MI_ConstStringField CompressionMode;
    MI_ConstStringField ProfileLoaded;
    MI_ConstStringField Encoding;
    MI_ConstStringField BufferMode;
    MI_ConstStringField State;
    MI_ConstDatetimeField ShellRunTime;
    MI_ConstDatetimeField ShellInactivity;
    MI_ConstStringField CreationXml;
}
Shell;
*/

void MI_CALL CreateShellComplete(
    _In_opt_     MI_Operation *operation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    struct WSMAN_SHELL *shell = (struct WSMAN_SHELL *) callbackContext;
    WSMAN_ERROR error = {0};

    __LOGD(("%s: START, errorCode=%u", "CreateShellComplete", resultCode));

    /* Copy off the resource URI that all future shell operations should use */
    if ((resultCode == MI_RESULT_OK) && instance)
    {
        MI_Value value;
        MI_Type type;

        if ((__MI_Instance_GetElement(instance, "ResourceUri", &value, &type, NULL, NULL) == MI_RESULT_OK) &&
                (type == MI_STRING) &&
                (Shell_Set_ResourceUri(shell->shellInstance, value.string) == MI_RESULT_OK) &&
                (MI_OperationOptions_SetResourceUri(&shell->operationOptions, value.string) == MI_RESULT_OK))
        {
            __LOGD(("Create shell returned resource URI = %s", value.string));
        }
        else
        {
            resultCode = MI_RESULT_FAILED;
        }
        if ((__MI_Instance_GetElement(instance, "ShellId", &value, &type, NULL, NULL) == MI_RESULT_OK) &&
                (type == MI_STRING) &&
                (Shell_Set_ShellId(shell->shellInstance, value.string) == MI_RESULT_OK))
        {
            __LOGD(("Create shell returned Shell ID = %s", value.string));
        }
        else
        {
            resultCode = MI_RESULT_FAILED;
        }
    }
    else if ((resultCode == MI_RESULT_NOT_SUPPORTED) && (errorDetails))
    {
        /* Check to see if this is a redirect error */
        MI_Value value;
        MI_Type type;
        MI_Uint32 flags, index;
        if ((MI_Instance_GetElement(errorDetails, MI_T("ProbableCauseDescription"),  &value, &type, &flags, &index) == MI_RESULT_OK) &&
                (type == MI_STRING) &&
                (value.string))
        {
            #define REDIRECT_LOCATION MI_T("REDIRECT_LOCATION: ")
            if (Tcsncmp(REDIRECT_LOCATION, value.string, MI_COUNT(REDIRECT_LOCATION)-1) == 0)
            {
                shell->session->redirectLocation = Batch_Tcsdup(shell->session->batch, value.string + (MI_COUNT(REDIRECT_LOCATION)-1));
                if (shell->session->redirectLocation == NULL)
                {
                    resultCode = MI_RESULT_SERVER_LIMITS_EXCEEDED;
                    errorString = NULL;
                }
                else
                {
                    resultCode = ERROR_WSMAN_REDIRECT_REQUESTED;
                }
            }
        }
    }

    error.code = resultCode;
    if (errorString)
    {
        Utf8ToUtf16Le(shell->batch, errorString, (MI_Char16**) &error.errorDetail);
    }
    else if (resultCode != MI_RESULT_OK)
    {
        Utf8ToUtf16Le(shell->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
    }

    MI_Operation_Close(&shell->miCreateShellOperation);

    if (resultCode == MI_RESULT_OK)
    {
        shell->didCreate = MI_TRUE;
        shell->asyncCallback.completionFunction(
                shell->asyncCallback.operationContext,
                0,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }
    else
    {
        shell->asyncCallback.completionFunction(
                shell->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
    }
    LogFunctionEnd("CreateShellComplete", resultCode);
}

static MI_Result ExtractStreamSet(WSMAN_STREAM_ID_SET *streamSet, Batch *batch, char **streamSetString)
{
    MI_Result miResult = MI_RESULT_OK;
    char *errorMessage = NULL;
    size_t stringLength = 1;
    MI_Uint32 count;
    char *tmpStr;
    char *cursor;

    *streamSetString = NULL;

    /* Allocate a buffer that is as big as all strings with spaces in-between  */
    for (count = 0; count != streamSet->streamIDsCount; count++)
    {
        stringLength += Utf16LeStrLenBytes(streamSet->streamIDs[count]);
        stringLength ++; /* for space between strings, extra one not a big issue */
    }
    tmpStr = Batch_Get(batch, stringLength);
    if (tmpStr == NULL)
        return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    cursor = tmpStr;

    if (streamSet->streamIDsCount > 0)
    {
        size_t iconv_return;
        iconv_t iconvData;

        iconvData = iconv_open("UTF-8", "UTF-16LE");
        if (iconvData == (iconv_t)-1)
        {
            GOTO_ERROR("Failed to convert stream", MI_RESULT_FAILED);
        }


        for (count = 0; count != streamSet->streamIDsCount; count++)
        {
            size_t thisStringLen = Utf16LeStrLenBytes(streamSet->streamIDs[count]);
            const MI_Char16 *strID = streamSet->streamIDs[count];

            iconv_return = iconv(iconvData,
                    (char**) &strID, &thisStringLen,
                    &cursor, &stringLength);
            if (iconv_return == (size_t) -1)
            {
                iconv_close(iconvData);
                GOTO_ERROR("Failed to convert stream", MI_RESULT_FAILED);
            }

            /* Append space on end */
            *(cursor-1) = ' ';
        }
        iconv_close(iconvData);
    }

    /* Null terminate */
    *(cursor-1) = '\0';

    *streamSetString = tmpStr;
error:
    return miResult;
}

MI_Result ExtractOptions(_In_opt_ WSMAN_OPTION_SET *wsmanOptions, Batch *batch, MI_OperationOptions *miOptions)
{
    MI_Uint32 i;
    MI_Result miResult = MI_RESULT_OK;
    char *errorMessage = NULL;


    if (wsmanOptions == NULL)
        return MI_RESULT_OK;

    for (i = 0; i != wsmanOptions->optionsCount; i++)
    {
        char *name;
        char *value;

        if (!Utf16LeToUtf8(batch, wsmanOptions->options[i].name, &name))
            GOTO_ERROR("Failed to convert option name", MI_RESULT_SERVER_LIMITS_EXCEEDED);

        if (!Utf16LeToUtf8(batch, wsmanOptions->options[i].value, &value))
            GOTO_ERROR("Failed to convert option value", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        __LOGD(("%s: %s=%s", "ExtractOptions", name, value));
        miResult = MI_OperationOptions_SetString(miOptions, name, value, 0);
        if (miResult != MI_RESULT_OK)
            GOTO_ERROR("Failed to set shell option", miResult);
    }
error:
    return miResult;
}

MI_EXPORT void WINAPI WSManCreateShellEx(
    _Inout_ WSMAN_SESSION_HANDLE session,
    MI_Uint32 flags,
    _In_ const MI_Char16* resourceUri,           // shell resource URI
    _In_ const MI_Char16* shellId,
    _In_opt_ WSMAN_SHELL_STARTUP_INFO *startupInfo,
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_opt_ WSMAN_DATA *createXml,                     // open content for create shell
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_SHELL_HANDLE *_shell) // should be closed using WSManCloseShell
{
    Batch *batch = NULL;
    MI_Instance *_shellInstance;
    MI_Result miResult;
    char *errorMessage = NULL;
    struct WSMAN_SHELL *shell = NULL;
    char *tmpStr = NULL;

    LogFunctionStart("WSManCreateShellEx");

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    shell = Batch_GetClear(batch, sizeof(struct WSMAN_SHELL));
    if (shell == NULL)
    {
        GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    shell->batch = batch;
    shell->session = session;

    miResult = MI_Application_NewOperationOptions(&session->api->application, MI_TRUE, &shell->operationOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    /* Stash the async callback information for when we get the wsman response and need
     * to call back into the client
     */
    shell->asyncCallback = *async;

    miResult = Instance_New(&_shellInstance, &Shell_rtti, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create instance", miResult);
    }
    shell->shellInstance = (Shell*) _shellInstance;


    if (shellId)
    {
        if (!Utf16LeToUtf8(batch, shellId, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        if (Shell_Set_ShellId(shell->shellInstance, tmpStr) != MI_RESULT_OK)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        __LOGD(("ShellID = %s", tmpStr));
    }

    if (resourceUri)
    {
        if (!Utf16LeToUtf8(batch, resourceUri, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        if (Shell_Set_ResourceUri(shell->shellInstance, tmpStr) != MI_RESULT_OK)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        if (MI_OperationOptions_SetResourceUri(&shell->operationOptions, tmpStr) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        __LOGD(("Resource URI = %s", tmpStr));
    }

    if (MI_OperationOptions_SetNumber(&shell->operationOptions, "__MI_OPERATIONOPTIONS_ISSHELL", 1, 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set IsShell option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (startupInfo)
    {
        if (startupInfo->inputStreamSet && startupInfo->inputStreamSet->streamIDsCount)
        {
            miResult = ExtractStreamSet(startupInfo->inputStreamSet, batch, &tmpStr);
            if (miResult != MI_RESULT_OK)
            {
                GOTO_ERROR("Extract input stream failed", miResult);
            }
            Shell_Set_InputStreams(shell->shellInstance, tmpStr);
            __LOGD(("Inbound streams = %s", tmpStr));
        }
        if (startupInfo->outputStreamSet && startupInfo->outputStreamSet->streamIDsCount)
        {
            miResult = ExtractStreamSet(startupInfo->outputStreamSet, batch, &tmpStr);
            if (miResult != MI_RESULT_OK)
            {
                GOTO_ERROR("Extract output stream failed", miResult);
            }
            if (Shell_Set_OutputStreams(shell->shellInstance, tmpStr) != MI_RESULT_OK)
            {
                GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            __LOGD(("Output streams = %s", tmpStr));
        }

        if (startupInfo->name)
        {
            if (!Utf16LeToUtf8(batch, startupInfo->name, &tmpStr))
            {
                GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            if (Shell_Set_Name(shell->shellInstance, tmpStr) != MI_RESULT_OK)
            {
                GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
            __LOGD(("Session name = %s", tmpStr));
         }
    }

    miResult = ExtractOptions(options, batch, &shell->operationOptions);
    if (miResult != MI_RESULT_OK)
        GOTO_ERROR("Failed to convert wsman options", miResult);

    if (createXml && (createXml->type == WSMAN_DATA_TYPE_TEXT))
    {
        if (!Utf16LeToUtf8(batch, createXml->text.buffer, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        if (Shell_Set_CreationXml(shell->shellInstance, tmpStr) != MI_RESULT_OK)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        __LOGD(("Creation XML = %s", tmpStr));
    }

    {
        memset(&shell->callbacks, 0, sizeof(shell->callbacks));

        shell->callbacks.instanceResult = CreateShellComplete;
        shell->callbacks.callbackContext = shell;

        miResult = MI_Application_NewSession(&session->api->application, NULL, session->hostname, &session->destinationOptions, NULL, NULL, &shell->miSession);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("MI_Application_NewSession failed", miResult);
        }

        MI_Session_CreateInstance(&shell->miSession,
                0, /* flags */
                &shell->operationOptions, /*options*/
                NULL, /* namespace */
                &shell->shellInstance->__instance,
                &shell->callbacks, &shell->miCreateShellOperation);
    }

    *_shell = shell;
    LogFunctionEnd("WSManCreateShellEx", miResult);
    return;

error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    if (batch)
    {
        Batch_Delete(batch);
    }

    if (shell->operationOptions.ft)
    {
        MI_OperationOptions_Delete(&shell->operationOptions);
    }

    *_shell = NULL;
    LogFunctionEnd("WSManCreateShellEx", miResult);
}

void MI_CALL CommandShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    WSMAN_COMMAND_HANDLE operation = ( WSMAN_COMMAND_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "CommandShellComplete", resultCode));
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorString)
        {
            Utf8ToUtf16Le(operation->batch, errorString, (MI_Char16**) &error.errorDetail);
        }
        else
        {
            Utf8ToUtf16Le(operation->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
        }
    }
    else if (instance)
    {
        MI_Value value;
        MI_Type type;

        if ((__MI_Instance_GetElement(instance, "CommandId", &value, &type, NULL, NULL) == MI_RESULT_OK) &&
                (type == MI_STRING) &&
                (__MI_Instance_SetElement(operation->commandProperties, "CommandId", &value, MI_STRING, 0) == MI_RESULT_OK) &&
                (__MI_Instance_GetElement(operation->commandProperties, "CommandId", &value, &type, NULL, NULL) == MI_RESULT_OK))
        {
            __LOGD(("Command returned Command ID = %s", value.string));

            /* Replace this with a pointer to the command ID from the instance owned by the command object, rather than the one that will
             * go out of scope after this call finishes */
            operation->commandId = value.string;
        }
        else
        {
            resultCode = MI_RESULT_FAILED;
        }
    }

    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation,
                NULL,
                NULL);
    MI_Operation_Close(&operation->miOperation);
//    Batch_Delete(operation->batch);

    LogFunctionEnd("CommandShellComplete", resultCode);
}

/* Adds an Arguments property that is a string array */
MI_Result  ExtractCommandArgs(WSMAN_COMMAND_ARG_SET *args, Batch *batch, MI_Instance *commandProperties)
{
    MI_Value stringArr;
    MI_Uint32 index;

    memset(&stringArr, 0, sizeof(stringArr));

    stringArr.stringa.size = args->argsCount;
    stringArr.stringa.data = Batch_Get(batch, sizeof(MI_Char*)*stringArr.stringa.size);
    if (stringArr.stringa.data == NULL)
        return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    for (index = 0; index != stringArr.stringa.size; index++)
    {
        if (!Utf16LeToUtf8(batch, args->args[index], &stringArr.stringa.data[index]))
        {
            return MI_RESULT_SERVER_LIMITS_EXCEEDED;
        }
        __LOGD(("Command argument %u = %s", index, stringArr.stringa.data[index]));
    }

    return MI_Instance_AddElement(commandProperties, "Arguments", &stringArr, MI_STRINGA, 0);
}

MI_EXPORT void WINAPI WSManRunShellCommandEx(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ const MI_Char16* commandId,
    _In_ const MI_Char16* commandLine,
    _In_opt_ WSMAN_COMMAND_ARG_SET *args,
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_COMMAND_HANDLE *command) // should be closed using WSManCloseCommand
{
    MI_Result miResult;
    char *errorMessage = NULL;
    Batch *batch = NULL;
    MI_Value value;
    char *tmpStr;

    LogFunctionStart("WSManRunShellCommandEx");

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*command) = Batch_GetClear(batch, sizeof(struct WSMAN_COMMAND));
    if (*command == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*command)->shell = shell;
    (*command)->asyncCallback = *async;
    (*command)->batch = batch;

    miResult = MI_Application_NewOperationOptions(&shell->session->api->application, MI_TRUE, &(*command)->miOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    miResult = MI_Application_NewInstance(&shell->session->api->application, "CommandLine", NULL, &(*command)->commandProperties);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    if (commandId)
    {
        if (!Utf16LeToUtf8(batch, commandId, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        value.string = tmpStr;
        if (MI_Instance_AddElement((*command)->commandProperties, "CommandId", &value, MI_STRING, 0) != MI_RESULT_OK)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        (*command)->commandId = tmpStr;
        __LOGD(("command ID = %s", tmpStr));
    }

    if (commandLine)
    {
        if (!Utf16LeToUtf8(batch, commandLine, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        value.string = tmpStr;
        if (MI_Instance_AddElement((*command)->commandProperties, "Command", &value, MI_STRING, 0) != MI_RESULT_OK)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        __LOGD(("command line = %s", tmpStr));
    }

    if (args )
    {
        miResult = ExtractCommandArgs(args, batch, (*command)->commandProperties);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to extract command args", miResult);
        }
    }

    {
        MI_Value value;
        MI_Type type;
        if (__MI_Instance_GetElement(&shell->shellInstance->__instance, "ResourceUri", &value, &type, NULL, NULL) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to get resource URI", MI_RESULT_FAILED);
        }
        if (MI_OperationOptions_SetResourceUri(&(*command)->miOptions, value.string) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
    }
    if (MI_OperationOptions_SetNumber(&(*command)->miOptions, "__MI_OPERATIONOPTIONS_ISSHELL", 1, 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set IsShell option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }


    if (MI_OperationOptions_SetString(&(*command)->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Command", 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set action option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    {

        (*command)->callbacks.instanceResult = CommandShellComplete;
        (*command)->callbacks.callbackContext = *command;

        MI_Session_Invoke(&shell->miSession,
                0, /* flags */
                &(*command)->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Command",
                &shell->shellInstance->__instance,
                (*command)->commandProperties,
                &((*command)->callbacks), &(*command)->miOperation);
    }

    __LOGD(("New command handle = %p", *command));
    LogFunctionEnd("WSManRunShellCommandEx", MI_RESULT_OK);
    return;


error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    if ((*command)->miOptions.ft)
    {
        MI_OperationOptions_Delete(&(*command)->miOptions);
    }
    if ((*command)->commandProperties)
    {
        MI_Instance_Delete((*command)->commandProperties);
    }
    Batch_Delete(batch);
    LogFunctionEnd("WSManRunShellCommandEx", miResult);
}
void MI_CALL SignalShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    WSMAN_OPERATION_HANDLE operation = ( WSMAN_OPERATION_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "SignalShellComplete", resultCode));
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorString)
        {
            Utf8ToUtf16Le(operation->batch, errorString, (MI_Char16**) &error.errorDetail);
        }
        else
        {
            Utf8ToUtf16Le(operation->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
        }
    }
    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation->command,
                operation,
                NULL);
    MI_Operation_Close(&operation->miOperation);
    Batch_Delete(operation->batch);

    LogFunctionEnd("SignalShellComplete", resultCode);
}

MI_EXPORT void WINAPI WSManSignalShell(
    _In_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,         // if NULL, the Signal will be sent to the shell
    MI_Uint32 flags,
    _In_ const MI_Char16* code,             // signal code
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *signalOperation)  // should be closed using WSManCloseOperation
{
    MI_Result miResult;
    char *errorMessage = NULL;
    //MI_Instance *desiredStream = NULL;
    Batch *batch = NULL;
//    char *streamSetString = NULL;
    MI_Value value;

    LogFunctionStart("WSManSignalShell");

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*signalOperation) = Batch_GetClear(batch, sizeof(struct WSMAN_OPERATION));
    if (*signalOperation == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*signalOperation)->type = WSMAN_OPERATION_SIGNAL;
    (*signalOperation)->shell = shell;
    (*signalOperation)->command = command;
    (*signalOperation)->asyncCallback = *async;
    (*signalOperation)->batch = batch;

    miResult = MI_Application_NewOperationOptions(&shell->session->api->application, MI_TRUE, &(*signalOperation)->miOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    miResult = MI_Application_NewInstance(&shell->session->api->application, "Signal", NULL, &(*signalOperation)->operationProperties);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    if (!Utf16LeToUtf8(batch, code, &value.string))
    {
        GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
     miResult = MI_Instance_AddElement((*signalOperation)->operationProperties, "Code", &value, MI_STRING, 0);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("out of memory", miResult);
    }
    __LOGD(("Signal Code = %s", value.string));

    if (command)
    {
        value.string = command->commandId;
        miResult = MI_Instance_AddElement((*signalOperation)->operationProperties, "CommandId", &value, MI_STRING, 0);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("out of memory", miResult);
        }
        __LOGD(("Signal for command %s", command->commandId));
    }

    {
        MI_Value value;
        MI_Type type;
        if (__MI_Instance_GetElement(&shell->shellInstance->__instance, "ResourceUri", &value, &type, NULL, NULL) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to get resource URI", MI_RESULT_FAILED);
        }
        if (MI_OperationOptions_SetResourceUri(&(*signalOperation)->miOptions, value.string) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
    }
    if (MI_OperationOptions_SetNumber(&(*signalOperation)->miOptions, "__MI_OPERATIONOPTIONS_ISSHELL", 1, 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set IsShell option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }


    if (MI_OperationOptions_SetString(&(*signalOperation)->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Signal", 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set action option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    {

        (*signalOperation)->callbacks.instanceResult = SignalShellComplete;
        (*signalOperation)->callbacks.callbackContext = *signalOperation;

        MI_Session_Invoke(&shell->miSession,
                0, /* flags */
                &(*signalOperation)->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Signal",
                &shell->shellInstance->__instance,
                (*signalOperation)->operationProperties,
                &((*signalOperation)->callbacks), &(*signalOperation)->miOperation);
    }

    LogFunctionEnd("WSManSignalShell", MI_RESULT_OK);
    return;


error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    if ((*signalOperation)->miOptions.ft)
    {
        MI_OperationOptions_Delete(&(*signalOperation)->miOptions);
    }
    if ((*signalOperation)->operationProperties)
    {
        MI_Instance_Delete((*signalOperation)->operationProperties);
    }
    LogFunctionEnd("WSManSignalShell", MI_RESULT_NOT_SUPPORTED);
}

MI_Result DecodeReceiveStream(WSMAN_OPERATION_HANDLE operation, const MI_Instance *streamInstance)
{
    DecodeBuffer decodeBuffer, decodedBuffer;
    Batch *batch;
    WSMAN_RESPONSE_DATA responseData;
    WSMAN_ERROR error = {0};
    const char *commandId = NULL;
    const char *streamName = NULL;
    const char *streamData = NULL;
    MI_Boolean streamComplete = MI_FALSE;
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;

    memset (&responseData, 0, sizeof(responseData));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

    if (__MI_Instance_GetElement(streamInstance, "CommandId", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        commandId = value.string;
        __LOGD(("Command ID = %s", commandId));
    }

    if (__MI_Instance_GetElement(streamInstance, "streamName", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        streamName = value.string;
        __LOGD(("Stream Name = %s", streamName));
    }

    if (__MI_Instance_GetElement(streamInstance, "endOfStream", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        streamComplete = value.boolean;
        __LOGD(("End of stream = %d", streamComplete));
    }

    if (__MI_Instance_GetElement(streamInstance, "data", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        streamData = value.string;
        __LOGD(("Data = %s", streamData));
    }

    decodeBuffer.buffer = (char*)streamData;
    decodeBuffer.bufferLength = Tcslen(streamData);
    decodeBuffer.bufferUsed = decodeBuffer.bufferLength;
    if (Base64DecodeBuffer(&decodeBuffer, &decodedBuffer) != MI_RESULT_OK)
    {
        error.code = MI_RESULT_FAILED;
        Utf8ToUtf16Le(operation->batch, "Receive failed to convert stream data", (MI_Char16**) &error.errorDetail);
        goto error;
    }

    /* TODO!! */
    responseData.receiveData.commandState = NULL;

    batch = Batch_New(BATCH_MAX_PAGES);
    if (!Utf8ToUtf16Le(batch, streamName, (MI_Char16**) &responseData.receiveData.streamId))
    {
        error.code = MI_RESULT_FAILED;
        Utf8ToUtf16Le(operation->batch, "Receive failed to convert stream name", (MI_Char16**) &error.errorDetail);
        goto error;
    }

    /* TODO!! Support compression */
    responseData.receiveData.exitCode = 0;
    responseData.receiveData.streamData.type = WSMAN_DATA_TYPE_BINARY;
    responseData.receiveData.streamData.binaryData.data = (MI_Uint8*) decodedBuffer.buffer;
    responseData.receiveData.streamData.binaryData.dataLength = decodedBuffer.bufferUsed;

    flags = 0;
    if (streamComplete)
        flags = WSMAN_FLAG_CALLBACK_END_OF_STREAM;

    operation->asyncCallback.completionFunction(
            operation->asyncCallback.operationContext,
            flags,
            &error,
            operation->shell,
            operation->command,
            operation,
            &responseData);

    free(decodedBuffer.buffer);

    Batch_Delete(batch);
    return MI_RESULT_OK;

error:
    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation->command,
                operation,
                NULL);

    return error.code;
}

MI_Result DecodeReceiveCommandState(WSMAN_OPERATION_HANDLE operation, const MI_Instance *commandStateInstance, MI_Boolean *done)
{
    Batch *batch;
    WSMAN_RESPONSE_DATA responseData = {{0}};
    WSMAN_ERROR error = {0};
    const char *commandId = NULL;
    const char *state = NULL;
    MI_Value value;
    MI_Type type;
    MI_Uint32 flags;

    if (__MI_Instance_GetElement(commandStateInstance, "CommandId", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        commandId = value.string;
        __LOGD(("CommandID = %s", commandId));
    }

    if (__MI_Instance_GetElement(commandStateInstance, "state", &value, &type, &flags, NULL) == MI_RESULT_OK)
    {
        state = value.string;
        __LOGD(("Command state = %s", state));
    }


    batch = Batch_New(BATCH_MAX_PAGES);
    if (!Utf8ToUtf16Le(batch, state, (MI_Char16**) &responseData.receiveData.commandState))
    {
        error.code = MI_RESULT_FAILED;
        Utf8ToUtf16Le(operation->batch, "Receive failed to convert commandState", (MI_Char16**) &error.errorDetail);
        goto error;
    }

    if (strcmp(state, WSMAN_COMMAND_STATE_DONE) == 0)
    {
        flags = WSMAN_FLAG_CALLBACK_END_OF_OPERATION;
        *done = MI_TRUE;
    }

    operation->asyncCallback.completionFunction(
            operation->asyncCallback.operationContext,
            0,
            &error,
            operation->shell,
            operation->command,
            operation,
            &responseData);

    Batch_Delete(batch);
    return MI_RESULT_OK;

error:
    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation->command,
                operation,
                NULL);

    return error.code;
}

void MI_CALL ReceiveShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    MI_Boolean done = MI_FALSE;
    WSMAN_OPERATION_HANDLE operation = ( WSMAN_OPERATION_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    if (operation->command)
    {
        __LOGD(("%s: START, errorCode=%u, shellId=%s, commandId=%s", "ReceiveShellComplete", resultCode, operation->shell->shellInstance->ShellId.value, operation->command->commandId));
    }
    else
    {
        __LOGD(("%s: START, errorCode=%u, shellId=%s, commandId=<null>", "ReceiveShellComplete", resultCode, operation->shell->shellInstance->ShellId.value));
    }
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorDetails)
        {
            MI_Value value;
            MI_Uint32 type;
            /* We need to check if this is a server-side timeout. If so we need to re-send the request */
            //ProbableCause == 111 -- timeout
            if ((MI_Instance_GetElement(errorDetails, "ProbableCause", &value, &type, NULL, NULL) == MI_RESULT_OK) &&
                    (value.uint32 == 111))
            {
                __LOGD(("Timeout from remote machine, re-sending request"));
                goto retry;
            }
        }
        if (errorString)
        {
            __LOGD(("Error string = %s", errorString));
            if (strncmp("ERROR_WSMAN_OPERATION_TIMEDOUT", errorString, strlen("ERROR_WSMAN_OPERATION_TIMEDOUT")) == 0)
            {
                /* We expect this error so lets fall through */
                __LOGD(("Timeout on receive means no data yet so we just send another request"));
                resultCode = 0;
            }
            else
            {
                Utf8ToUtf16Le(operation->batch, errorString, (MI_Char16**) &error.errorDetail);
                goto error;
            }
        }
        else
        {
            Utf8ToUtf16Le(operation->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
            goto error;
        }
    }

    if (instance)
    {
        MI_Value value;
        MI_Uint32 type;

        if (__MI_Instance_GetElement(instance, "CommandId", &value, &type, NULL, NULL) == MI_RESULT_OK)
        {
            __LOGD(("Receive result for CommandId=%s", value.string));
        }
        __LOGD(("Got an instance"));
        if (__MI_Instance_GetElement(instance, "Stream", &value, &type, NULL, NULL) == MI_RESULT_OK)
        {
            __LOGD(("Got a stream, type = %u", type));

            if (type & MI_INSTANCE)
            {
                MI_Result miResult;

                if (type & MI_ARRAY)
                {
                    int i;
                    MI_InstanceA *instArray = (MI_InstanceA*) &value;

                    for (i = 0; i != instArray->size; i++)
                    {
                        __LOGD(("Entry %u = %p", i, instArray->data[i]));
                        miResult = DecodeReceiveStream(operation, instArray->data[i]);
                    }

                }
                else
                {
                    __LOGD(("Entry %u = %p", 0, value.string));
                    miResult = DecodeReceiveStream(operation, value.instance);
                }
                if (miResult != MI_RESULT_OK)
                {
                    error.code = MI_RESULT_FAILED;
                    Utf8ToUtf16Le(operation->batch, "Receive failed to get stream data", (MI_Char16**) &error.errorDetail);
                    goto error;
                }
            }
            else
            {
                __LOGD(("It is an unsupported type"));
                error.code = MI_RESULT_FAILED;
                Utf8ToUtf16Le(operation->batch, "Receive data has unsupported type", (MI_Char16**) &error.errorDetail);
                goto error;
            }
        }
        else if (__MI_Instance_GetElement(instance, "CommandState", &value, &type, NULL, NULL) == MI_RESULT_OK)
        {
            MI_Result miResult = DecodeReceiveCommandState(operation, value.instance, &done);
            if (miResult != MI_RESULT_OK)
            {
                error.code = MI_RESULT_FAILED;
                Utf8ToUtf16Le(operation->batch, "Receive failed to get stream data", (MI_Char16**) &error.errorDetail);
                goto error;
            }
         }
    }

retry:
    MI_Operation_Close(&operation->miOperation);

    /* WHAT ABOUT TIMEOUT ? */
    if (!done /*&& (resultCode == 0)*/)
    {
        __LOGD(("Sending new receive request"));
        MI_Session_Invoke(&operation->shell->miSession,
                0, /* flags */
                &operation->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Receive",
                &operation->shell->shellInstance->__instance,
                operation->operationProperties,
                &(operation->callbacks), &operation->miOperation);
    }
    LogFunctionEnd("ReceiveShellComplete", resultCode);

    return;

error:
    MI_Operation_Close(&operation->miOperation);
    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation->command,
                operation,
                NULL);
    Batch_Delete(operation->batch);
 }

MI_EXPORT void WINAPI WSManReceiveShellOutput(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,
    MI_Uint32 flags,
    _In_opt_ WSMAN_STREAM_ID_SET *desiredStreamSet,  // request output from a particular stream or list of streams
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *receiveOperation) // should be closed using WSManCloseOperation
{
    MI_Result miResult;
    char *errorMessage = NULL;
    //MI_Instance *desiredStream = NULL;
    Batch *batch = NULL;
    char *streamSetString = NULL;
    MI_Value value;

    LogFunctionStart("WSManReceiveShellOutput");

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*receiveOperation) = Batch_GetClear(batch, sizeof(struct WSMAN_OPERATION));
    if (*receiveOperation == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*receiveOperation)->type = WSMAN_OPERATION_RECEIVE;
    (*receiveOperation)->shell = shell;
    (*receiveOperation)->command = command;
    (*receiveOperation)->asyncCallback = *async;
    (*receiveOperation)->batch = batch;

    miResult = MI_Application_NewOperationOptions(&shell->session->api->application, MI_TRUE, &(*receiveOperation)->miOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    miResult = MI_Application_NewInstance(&shell->session->api->application, "Receive", NULL, &(*receiveOperation)->operationProperties);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    miResult = ExtractStreamSet(desiredStreamSet, batch, &streamSetString);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to convert receive desiredStreamSet", miResult);
    }

    value.string = streamSetString;
    miResult = MI_Instance_AddElement((*receiveOperation)->operationProperties, "DesiredStream", &value, MI_STRING, 0);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("out of memory", miResult);
    }
    __LOGD(("Stream set = %s", value.string));

    if (command)
    {
        value.string = command->commandId;
        miResult = MI_Instance_AddElement((*receiveOperation)->operationProperties, "CommandId", &value, MI_STRING, 0);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("out of memory", miResult);
        }
        __LOGD(("Receive for command %s", command->commandId));
    }

    {
        MI_Value value;
        MI_Type type;
        if (__MI_Instance_GetElement(&shell->shellInstance->__instance, "ResourceUri", &value, &type, NULL, NULL) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to get resource URI", MI_RESULT_FAILED);
        }
        if (MI_OperationOptions_SetResourceUri(&(*receiveOperation)->miOptions, value.string) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
    }
    if (MI_OperationOptions_SetNumber(&(*receiveOperation)->miOptions, "__MI_OPERATIONOPTIONS_ISSHELL", 1, 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set IsShell option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (MI_OperationOptions_SetString(&(*receiveOperation)->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Receive", 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set action option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    {

        (*receiveOperation)->callbacks.instanceResult = ReceiveShellComplete;
        (*receiveOperation)->callbacks.callbackContext = *receiveOperation;

        MI_Session_Invoke(&shell->miSession,
                0, /* flags */
                &(*receiveOperation)->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Receive",
                &shell->shellInstance->__instance,
                (*receiveOperation)->operationProperties,
                &((*receiveOperation)->callbacks), &(*receiveOperation)->miOperation);
    }

    LogFunctionEnd("WSManReceiveShellOutput", MI_RESULT_OK);
    return;


error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    if ((*receiveOperation)->miOptions.ft)
    {
        MI_OperationOptions_Delete(&(*receiveOperation)->miOptions);
    }
    if ((*receiveOperation)->operationProperties)
    {
        MI_Instance_Delete((*receiveOperation)->operationProperties);
    }
    Batch_Delete(batch);
    LogFunctionEnd("WSManReceiveShellOutput", miResult);
}

void MI_CALL SendShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    WSMAN_OPERATION_HANDLE operation = ( WSMAN_OPERATION_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "SendShellComplete", resultCode));
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorString)
        {
            Utf8ToUtf16Le(operation->batch, errorString, (MI_Char16**) &error.errorDetail);
        }
        else
        {
            Utf8ToUtf16Le(operation->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
        }
    }
    operation->asyncCallback.completionFunction(
                operation->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                operation->shell,
                operation->command,
                operation,
                NULL);
    MI_Operation_Close(&operation->miOperation);
    Batch_Delete(operation->batch);

    LogFunctionEnd("SendShellComplete", resultCode);
}
MI_EXPORT void WINAPI WSManSendShellInput(
    _In_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,
    MI_Uint32 flags,
    _In_ const MI_Char16* streamId,               // input stream name
    _In_ WSMAN_DATA *streamData,        // data as binary - that can contain text (ANSI/UNICODE),
                                        // binary content or objects or partial or full XML
    BOOL endOfStream,
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *sendOperation) // should be closed using WSManCloseOperation
{
    MI_Result miResult;
    char *errorMessage = NULL;
    MI_Instance *stream = NULL;
    Batch *batch = NULL;
    MI_Value value;

    LogFunctionStart("WSManSendShellInput");

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*sendOperation) = Batch_GetClear(batch, sizeof(struct WSMAN_OPERATION));
    if (sendOperation == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*sendOperation)->type = WSMAN_OPERATION_SEND;
    (*sendOperation)->shell = shell;
    (*sendOperation)->command = command;
    (*sendOperation)->asyncCallback = *async;
    (*sendOperation)->batch = batch;

    miResult = MI_Application_NewOperationOptions(&shell->session->api->application, MI_TRUE, &(*sendOperation)->miOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    miResult = MI_Application_NewInstance(&shell->session->api->application, "Send", NULL, &(*sendOperation)->operationProperties);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate operation properties instance", miResult);
    }

    miResult = MI_Application_NewInstance(&shell->session->api->application, "Stream", NULL, &stream);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate operation properties instance", miResult);
    }

    if (command)
    {
        value.string = command->commandId;
        miResult = MI_Instance_AddElement(stream, "CommandId", &value, MI_STRING, 0);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("out of memory", miResult);
        }
        __LOGD(("Send for command %s", command->commandId));
    }

    if (streamId)
    {
        if (!Utf16LeToUtf8(batch, streamId, &value.string))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        miResult = MI_Instance_AddElement(stream, "streamName", &value, MI_STRING, 0);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("out of memory", miResult);
        }
        __LOGD(("Send stream name = %s", value.string));
    }


    if (streamData && streamData->type == WSMAN_DATA_TYPE_BINARY && streamData->binaryData.data)
    {
        DecodeBuffer decodeBuffer, decodedBuffer;
        memset(&decodeBuffer, 0, sizeof(decodeBuffer));
        memset(&decodedBuffer, 0, sizeof(decodedBuffer));

        decodeBuffer.buffer = (MI_Char*) streamData->binaryData.data;
        decodeBuffer.bufferLength = streamData->binaryData.dataLength;
        decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

        /* NOTE: Base64EncodeBuffer allocates enough space for a NULL terminator */
        miResult = Base64EncodeBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("Base64EncodeBuffer failed", miResult);
        }

        /* Set the null terminator on the end of the buffer as this is supposed to be a string*/
        memset(decodedBuffer.buffer + decodedBuffer.bufferUsed, 0, sizeof(MI_Char));

        value.string = decodedBuffer.buffer;

        miResult = MI_Instance_AddElement(stream, "data", &value, MI_STRING, 0);

        if (miResult != MI_RESULT_OK)
        {
            free(decodedBuffer.buffer);
            GOTO_ERROR("out of memory", miResult);
        }

        __LOGD(("Send stream data = %s", value.string));
        free(decodedBuffer.buffer);
    }

    if (endOfStream)
    {
        value.boolean = MI_TRUE;
        miResult = MI_Instance_AddElement(stream, "endOfStream", &value, MI_BOOLEAN, 0);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("out of memory", miResult);
        }
        __LOGD(("Send stream end-of-stream %s", value.string));
    }

    value.instance = stream;
    miResult = MI_Instance_AddElement((*sendOperation)->operationProperties, "Stream", &value, MI_INSTANCE, MI_FLAG_BORROW);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to add Stream property to parameters", miResult);
    }

    {
        MI_Value value;
        MI_Type type;
        if (__MI_Instance_GetElement(&shell->shellInstance->__instance, "ResourceUri", &value, &type, NULL, NULL) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to get resource URI", MI_RESULT_FAILED);
        }
        if (MI_OperationOptions_SetResourceUri(&(*sendOperation)->miOptions, value.string) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
    }
    if (MI_OperationOptions_SetNumber(&(*sendOperation)->miOptions, "__MI_OPERATIONOPTIONS_ISSHELL", 1, 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set IsShell option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (MI_OperationOptions_SetString(&(*sendOperation)->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Send", 0) != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to set action option", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    {

        (*sendOperation)->callbacks.instanceResult = SendShellComplete;
        (*sendOperation)->callbacks.callbackContext = *sendOperation;

        MI_Session_Invoke(&shell->miSession,
                0, /* flags */
                &(*sendOperation)->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Send",
                &shell->shellInstance->__instance,
                (*sendOperation)->operationProperties,
                &((*sendOperation)->callbacks), &(*sendOperation)->miOperation);
    }

    LogFunctionEnd("WSManSendShellInput", MI_RESULT_OK);
    return;


error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    if ((*sendOperation)->miOptions.ft)
    {
        MI_OperationOptions_Delete(&(*sendOperation)->miOptions);
    }
    if ((*sendOperation)->operationProperties)
    {
        MI_Instance_Delete((*sendOperation)->operationProperties);
    }
    Batch_Delete(batch);
    LogFunctionEnd("WSManSendShellInput", miResult);
}

void MI_CALL CommandCloseShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    WSMAN_COMMAND_HANDLE command = ( WSMAN_COMMAND_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "CommandCloseShellComplete", resultCode));
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorString)
        {
            Utf8ToUtf16Le(command->batch, errorString, (MI_Char16**) &error.errorDetail);
            __LOGD(("Error string = %s", errorString));
        }
        else
        {
            Utf8ToUtf16Le(command->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
        }
    }

    MI_Operation_Close(miOperation);

    command->asyncCallback.completionFunction(
            command->asyncCallback.operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            command->shell,
            command,
            NULL,
            NULL);
}

 MI_EXPORT void WINAPI WSManCloseCommand(
    _Inout_opt_ WSMAN_COMMAND_HANDLE commandHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    MI_Result miResult;
    char *errorMessage = NULL;
    MI_Value value;

    LogFunctionStart("WSManCloseCommand");

    miResult = MI_Application_NewInstance(&commandHandle->shell->session->api->application, "Signal", NULL, &commandHandle->commandClose);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    value.string = commandHandle->commandId;
    miResult = MI_Instance_AddElement(commandHandle->commandClose, "CommandId", &value, MI_STRING, 0);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    value.string = "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/signal/terminate";
    miResult = MI_Instance_AddElement(commandHandle->commandClose, "Code", &value, MI_STRING, 0);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    MI_OperationOptions_SetString(&commandHandle->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Signal", 0);

    commandHandle->asyncCallback = *async;

    {
        commandHandle->callbacks.instanceResult = CommandCloseShellComplete;
        commandHandle->callbacks.callbackContext = commandHandle;

        MI_Session_Invoke(&commandHandle->shell->miSession,
                0, /* flags */
                &commandHandle->miOptions, /*options*/
                NULL, /* namespace */
                "Shell",
                "Signal",
                &commandHandle->shell->shellInstance->__instance,
                commandHandle->commandClose,
                &(commandHandle->callbacks), &commandHandle->miOperation);
    }

    LogFunctionEnd("WSManCloseCommand", MI_RESULT_OK);
    return;


error:
    {
        WSMAN_ERROR error = { 0 };
        error.code = miResult;
        Batch *batch = NULL;
        batch = Batch_New(BATCH_MAX_PAGES);
        if (batch != NULL)
        {
            Utf8ToUtf16Le(batch, errorMessage, (MI_Char16**) &error.errorDetail);
        }

        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                commandHandle->shell,
                commandHandle,
                NULL,
                NULL);
        if (batch)
        {
            Batch_Delete(batch);
        }
     }

    if (commandHandle->commandClose)
    {
        MI_Instance_Delete(commandHandle->commandClose);
    }
    LogFunctionEnd("WSManCloseCommand", miResult);
    return;
}

void MI_CALL CloseSessionComplete(_In_opt_ void *competionContext
)
{
    __LOGD(("%s: END", "CloseSessionComplete"));
}

void MI_CALL CloseShellComplete(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext,
    _In_opt_ const MI_Instance *instance,
             MI_Boolean moreResults,
    _In_     MI_Result resultCode,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    WSMAN_SHELL_HANDLE shell = ( WSMAN_SHELL_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "CloseShellComplete", resultCode));
    error.code = resultCode;
    if (resultCode != 0)
    {
        if (errorString)
        {
            Utf8ToUtf16Le(shell->batch, errorString, (MI_Char16**) &error.errorDetail);
            __LOGD(("Error string = %s", errorString));
        }
        else
        {
            Utf8ToUtf16Le(shell->batch, Result_ToString(resultCode), (MI_Char16**) &error.errorDetail);
        }
    }
    shell->asyncCallback.completionFunction(
                shell->asyncCallback.operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
    MI_Operation_Close(miOperation);
    __LOGD(("%s: START", "CloseSessionComplete"));
    MI_Session_Close(&shell->miSession, NULL, CloseSessionComplete);
    __LOGD(("%s: END, errorCode=%u", "CloseShellComplete", resultCode));
}
MI_EXPORT void WINAPI WSManCloseShell(
    _Inout_opt_ WSMAN_SHELL_HANDLE shellHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    LogFunctionStart("WSManCloseShell");

    shellHandle->asyncCallback = *async;

    if (shellHandle->didCreate)
    {
        shellHandle->callbacks.instanceResult = CloseShellComplete;
        shellHandle->callbacks.callbackContext = shellHandle;

        MI_Session_DeleteInstance(&shellHandle->miSession,
                0, /* flags */
                &shellHandle->operationOptions, /*options*/
                NULL, /* namespace */
                &shellHandle->shellInstance->__instance,
                &shellHandle->callbacks,
                &shellHandle->miDeleteShellOperation);
    }
    else
    {
        WSMAN_ERROR error = {0};
        error.code = MI_RESULT_OK;
        shellHandle->asyncCallback.completionFunction(
                    shellHandle->asyncCallback.operationContext,
                    WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                    &error,
                    shellHandle,
                    NULL,
                    NULL,
                    NULL);
     }

    LogFunctionEnd("WSManCloseShell", MI_RESULT_OK);
    return;
}

MI_EXPORT void WINAPI WSManDisconnectShell(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_DISCONNECT_INFO* disconnectInfo,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    WSMAN_ERROR error = {0};
    LogFunctionStart("WSManDisconnectShell");
    error.code = MI_RESULT_NOT_SUPPORTED;
    async->completionFunction(
            async->operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            shell,
            NULL,
            NULL,
            NULL);
     LogFunctionEnd("WSManDisconnectShell", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManReconnectShell(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    WSMAN_ERROR error = {0};
    LogFunctionStart("WSManReconnectShell");
    error.code = MI_RESULT_NOT_SUPPORTED;
    async->completionFunction(
            async->operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            shell,
            NULL,
            NULL,
            NULL);
     LogFunctionEnd("WSManReconnectShell", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManReconnectShellCommand(
    _Inout_ WSMAN_COMMAND_HANDLE commandHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    WSMAN_ERROR error = {0};
    LogFunctionStart("WSManReconnectShellCommand");
    error.code = MI_RESULT_NOT_SUPPORTED;
    async->completionFunction(
            async->operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            commandHandle->shell ,
            NULL,
            NULL,
            NULL);
     LogFunctionEnd("WSManReconnectShellCommand", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManConnectShell(
    _Inout_ WSMAN_SESSION_HANDLE session,
    MI_Uint32 flags,
    _In_ const MI_Char16* resourceUri,
    _In_ const MI_Char16* shellID,           // shell Identifier
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_opt_ WSMAN_DATA *connectXml,                     // open content for connect shell
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_SHELL_HANDLE *shell) // should be closed using WSManCloseShell
{
    WSMAN_ERROR error = {0};
    *shell = NULL;
    LogFunctionStart("WSManConnectShell");
    error.code = MI_RESULT_NOT_SUPPORTED;
    async->completionFunction(
            async->operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            *shell,
            NULL,
            NULL,
            NULL);
     LogFunctionEnd("WSManConnectShell", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManConnectShellCommand(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ const MI_Char16* commandID,  //command Identifier
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_opt_ WSMAN_DATA *connectXml,                // open content for connect command
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_COMMAND_HANDLE *command) // should be closed using WSManCloseCommand
{
    WSMAN_ERROR error = {0};
    LogFunctionStart("WSManConnectShellCommand");
    error.code = MI_RESULT_NOT_SUPPORTED;
    async->completionFunction(
            async->operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            shell,
            NULL,
            NULL,
            NULL);
     LogFunctionEnd("WSManConnectShellCommand", MI_RESULT_NOT_SUPPORTED);
}
