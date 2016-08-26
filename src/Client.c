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
#include <MI.h>
#include "wsman.h"
#include "BufferManipulation.h"
#include "Shell.h"
#include "Command.h"
#include "DesiredStream.h"

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



#define SHELL_ENABLE_LOGGING 1
#define SHELL_LOGGING_FILE "shell"
#define SHELL_LOGGING_LEVEL OMI_DEBUG


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
};

struct WSMAN_SHELL
{
    WSMAN_SESSION_HANDLE session;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    Shell *shellInstance;
    MI_Session miSession;
    MI_Operation miCreateShellOperation;
};

struct WSMAN_COMMAND
{
    WSMAN_SHELL_HANDLE shell;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
};

struct WSMAN_OPERATION
{
    WSMAN_SHELL_HANDLE shell;
    WSMAN_COMMAND_HANDLE command;
    Batch *batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    MI_OperationCallbacks callbacks;
    MI_Operation miOperation;
    MI_OperationOptions miOptions;
    MI_Instance *receiveProperties;
};

MI_EXPORT MI_Uint32 WINAPI WSManInitialize(
    MI_Uint32 flags,
    _Out_ WSMAN_API_HANDLE *apiHandle
    )
{
    MI_Result miResult;

#ifdef SHELL_ENABLE_LOGGING
    MI_Char finalPath[PAL_MAX_PATH_SIZE];

    CreateLogFileNameWithPrefix(SHELL_LOGGING_FILE, finalPath);
    Log_Open(finalPath);
    Log_SetLevel(SHELL_LOGGING_LEVEL);
#endif

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

#ifdef SHELL_ENABLE_LOGGING
    Log_Close();
#endif
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
        return 122; /* Windows error code ERROR_INSUFFICIENT_BUFFER */
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

    if (_connection && !Utf16LeToUtf8(batch, _connection, &connection))
    {
        GOTO_ERROR("Failed to convert connection name", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*session)->hostname = connection;

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
    httpUrl = strchr(connection, '/');
    if (httpUrl == NULL)
        httpUrl = "/wsman/";
    else
    {
        char *tmp = Batch_Tcsdup(batch, httpUrl);
        if (tmp == NULL)
        {
            GOTO_ERROR("Failed to convert connection name", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        *httpUrl = '\0';
        httpUrl = tmp;
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

    miResult = MI_Application_NewDestinationOptions(&apiHandle->application, &(*session)->destinationOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Destination options creation failed", miResult);
    }

    miResult = MI_DestinationOptions_AddDestinationCredentials(&(*session)->destinationOptions, &userCredentials);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to add credentials to destination options", miResult);
    }
    miResult = MI_DestinationOptions_SetHttpUrlPrefix(&(*session)->destinationOptions, httpUrl);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to add http prefix to destination options", miResult);
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
            if (data->type != WSMAN_DATA_TYPE_DWORD)
                GOTO_ERROR("SSL option should be DWORD", MI_RESULT_INVALID_PARAMETER);
            if (data->number == 0)
            {
                miResult = MI_DestinationOptions_SetTransport(&session->destinationOptions, MI_DESTINATIONOPTIONS_TRANSPORT_HTTP);
            }
            else
            {
                miResult = MI_DestinationOptions_SetTransport(&session->destinationOptions, MI_DESTINATIONOPTIONS_TRANPSORT_HTTPS);
            }
            break;

        case WSMAN_OPTION_UI_LANGUAGE:
        {
            /* String, convert utf16->utf8, locale */
            MI_Char *tmpStr;
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
            /* dword, operation timeout when not the others */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_TIMEOUTMS_CREATE_SHELL:
            /* dword */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_TIMEOUTMS_CLOSE_SHELL:
            /* dword */
            miResult = MI_RESULT_OK;
            break;
        case WSMAN_OPTION_TIMEOUTMS_SIGNAL_SHELL:
            /* dword */
            miResult = MI_RESULT_OK;
            break;

        default:
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
            break;

        case WSMAN_OPTION_MAX_RETRY_TIME:
            *value = 60; /* TODO: Proper value? */
            break;

        default:
            miResult = MI_RESULT_NOT_SUPPORTED;
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
    LogFunctionStart("WSManGetSessionOptionAsString");
    LogFunctionEnd("WSManGetSessionOptionAsString", MI_RESULT_NOT_SUPPORTED);
    /* TODO */
    return MI_RESULT_NOT_SUPPORTED;
}
MI_EXPORT MI_Uint32 WINAPI WSManCloseOperation(
    _Inout_opt_ WSMAN_OPERATION_HANDLE operationHandle,
    MI_Uint32 flags)
{
    LogFunctionStart("WSManCloseOperation");
    LogFunctionEnd("WSManCloseOperation", MI_RESULT_NOT_SUPPORTED);
    return MI_RESULT_NOT_SUPPORTED;
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
                (Shell_Set_ResourceUri(shell->shellInstance, value.string) == MI_RESULT_OK))
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

            iconv_return = iconv(iconvData,
                    (char**) &streamSet->streamIDs[count], &thisStringLen,
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
    MI_OperationOptions operationOptions = MI_OPERATIONOPTIONS_NULL;

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

    miResult = MI_Application_NewOperationOptions(&session->api->application, MI_TRUE, &operationOptions);
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
        if (MI_OperationOptions_SetResourceUri(&operationOptions, tmpStr) != MI_RESULT_OK)
        {
            GOTO_ERROR("Failed to set resource URI in options", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        __LOGD(("Resource URI = %s", tmpStr));
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

    miResult = ExtractOptions(options, batch, &operationOptions);
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
        MI_OperationCallbacks callbacks = MI_OPERATIONCALLBACKS_NULL;
        callbacks.instanceResult = CreateShellComplete;
        callbacks.callbackContext = shell;

        miResult = MI_Application_NewSession(&session->api->application, NULL, session->hostname, &session->destinationOptions, NULL, NULL, &shell->miSession);
        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR("MI_Application_NewSession failed", miResult);
        }

        MI_Session_CreateInstance(&shell->miSession,
                0, /* flags */
                &operationOptions, /*options*/
                NULL, /* namespace */
                &shell->shellInstance->__instance,
                &callbacks, &shell->miCreateShellOperation);
    }


    MI_OperationOptions_Delete(&operationOptions);

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

    if (operationOptions.ft)
    {
        MI_OperationOptions_Delete(&operationOptions);
    }

    *_shell = NULL;
    LogFunctionEnd("WSManCreateShellEx", miResult);
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
    LogFunctionStart("WSManRunShellCommandEx");
    {
        WSMAN_ERROR error = { 0 };
        error.code = MI_RESULT_NOT_SUPPORTED;
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    LogFunctionEnd("WSManRunShellCommandEx", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManSignalShell(
    _In_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,         // if NULL, the Signal will be sent to the shell
    MI_Uint32 flags,
    _In_ const MI_Char16* code,             // signal code
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *signalOperation)  // should be closed using WSManCloseOperation
{
    LogFunctionStart("WSManSignalShell");
    {
        WSMAN_ERROR error = { 0 };
        error.code = MI_RESULT_NOT_SUPPORTED;
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
     }

    LogFunctionEnd("WSManSignalShell", MI_RESULT_NOT_SUPPORTED);
}

MI_Result DecodeReceiveStream(WSMAN_OPERATION_HANDLE operation, const char *streamData, const char *streamId)
{
    DecodeBuffer decodeBuffer, decodedBuffer;
    Batch *batch;
    WSMAN_RESPONSE_DATA responseData;
    WSMAN_ERROR error = {0};

    memset (&responseData, 0, sizeof(responseData));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

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

    /* TODO!! Need to decode packet in a custom way to retrieve this from xml attribute for each stream data item */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (!Utf8ToUtf16Le(batch, "stdout", (MI_Char16**) &responseData.receiveData.streamId))
    {
        error.code = MI_RESULT_FAILED;
        Utf8ToUtf16Le(operation->batch, "Receive failed to convert stream name", (MI_Char16**) &error.errorDetail);
        goto error;
    }

    /* TODO!! Support compression */
    responseData.receiveData.exitCode = 0;
    responseData.receiveData.streamData.type = WSMAN_DATA_TYPE_TEXT;
    responseData.receiveData.streamData.text.buffer = (MI_Char16*) decodedBuffer.buffer;
    responseData.receiveData.streamData.text.bufferLength = decodedBuffer.bufferUsed;

    operation->asyncCallback.completionFunction(
            operation->asyncCallback.operationContext,
            0,
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
    WSMAN_OPERATION_HANDLE operation = ( WSMAN_OPERATION_HANDLE ) callbackContext;
    WSMAN_ERROR error = {0};
    __LOGD(("%s: START, errorCode=%u", "ReceiveShellComplete", resultCode));
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
        goto error;
    }
    else if (instance)
    {
        MI_Value value;
        MI_Uint32 type;

        __LOGD(("Got an instance"));
        if (__MI_Instance_GetElement(instance, "Stream", &value, &type, NULL, NULL) == MI_RESULT_OK)
        {
            __LOGD(("Got a stream, type = %u", type));

            if (type & MI_STRING)
            {
                MI_Result miResult;

                if (type & MI_ARRAY)
                {
                    int i;
                    MI_StringA *strArray = (MI_StringA*) &value;

                    __LOGD(("It is an string array"));

                    for (i = 0; i != strArray->size; i++)
                    {
                        __LOGD(("Entry %u = %s", i, strArray->data[i]));
                        miResult = DecodeReceiveStream(operation, strArray->data[i], "stdout");
                    }

                }
                else
                {
                    __LOGD(("Entry %u = %s", 0, value.string));
                    miResult = DecodeReceiveStream(operation, value.string, "stdout");
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
            /* TODO!! */
        }
    }

    MI_Operation_Close(&operation->miOperation);

    MI_Session_Invoke(&operation->shell->miSession,
            0, /* flags */
            &operation->miOptions, /*options*/
            NULL, /* namespace */
            "Shell",
            "Receive",
            &operation->shell->shellInstance->__instance,
            operation->receiveProperties,
            &(operation->callbacks), &operation->miOperation);

    LogFunctionEnd("ReceiveShellComplete", resultCode);

    return;

error:
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

    (*receiveOperation) = Batch_GetClear(batch, sizeof(WSMAN_OPERATION_HANDLE));
    if (*receiveOperation == NULL)
    {
        GOTO_ERROR("out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    (*receiveOperation)->shell = shell;
    (*receiveOperation)->command = command;
    (*receiveOperation)->asyncCallback = *async;
    (*receiveOperation)->batch = batch;

    miResult = MI_Application_NewOperationOptions(&shell->session->api->application, MI_TRUE, &(*receiveOperation)->miOptions);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create operation options", miResult);
    }

    //miResult = Instance_New((MI_Instance**) &receiveProperties, (MI_ClassDecl*) &Shell_Receive_rtti, batch);
    //miResult = Instance_NewDynamic(&receiveProperties, "Receive", MI_FLAG_CLASS, batch);
    miResult = MI_Application_NewInstance(&shell->session->api->application, "Receive", NULL, &(*receiveOperation)->receiveProperties);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate receive properties instance", miResult);
    }

    //miResult = Instance_New((MI_Instance**) &desiredStream, (MI_ClassDecl*) &DesiredStream_rtti, batch);
    //miResult = Instance_NewDynamic(&desiredStream, "DesiredStream", MI_FLAG_CLASS, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to allocate desired stream instance", miResult);
    }

    miResult = ExtractStreamSet(desiredStreamSet, batch, &streamSetString);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to convert receive desiredStreamSet", miResult);
    }

    value.string = streamSetString;
    //miResult = MI_Instance_AddElement(desiredStream, "streamName", &value, MI_STRING, MI_FLAG_BORROW);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("out of memory", miResult);
    }

    /* TODO: Set commandID */

    //Shell_Receive_SetPtr_DesiredStream(receiveProperties, desiredStream);
    //value.instance = desiredStream;
    //miResult = MI_Instance_AddElement(receiveProperties, "DesiredStream", &value, MI_INSTANCE, MI_FLAG_BORROW);
    //value.string = streamSetString;
    value.string = "stdout";
    miResult = MI_Instance_AddElement((*receiveOperation)->receiveProperties, "DesiredStream", &value, MI_STRING, 0);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("out of memory", miResult);
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

    MI_OperationOptions_SetString(&(*receiveOperation)->miOptions, "__MI_OPERATIONOPTIONS_ACTION", "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/Receive", 0);
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
                (*receiveOperation)->receiveProperties,
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
    if ((*receiveOperation)->receiveProperties)
    {
        MI_Instance_Delete((*receiveOperation)->receiveProperties);
    }
    Batch_Delete(batch);
    LogFunctionEnd("WSManReceiveShellOutput", miResult);
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
    LogFunctionStart("WSManSendShellInput");
    {
        WSMAN_ERROR error = { 0 };
        error.code = MI_RESULT_NOT_SUPPORTED;
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                shell,
                NULL,
                NULL,
                NULL);
    }
    LogFunctionEnd("WSManSendShellInput", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManCloseCommand(
    _Inout_opt_ WSMAN_COMMAND_HANDLE commandHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    LogFunctionStart("WSManCloseCommand");
    {
        WSMAN_ERROR error = { 0 };
        error.code = MI_RESULT_NOT_SUPPORTED;
        async->completionFunction(
                async->operationContext,
                WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
                &error,
                commandHandle->shell,
                NULL,
                NULL,
                NULL);
    }
    LogFunctionEnd("WSManCloseCommand", MI_RESULT_NOT_SUPPORTED);
}

MI_EXPORT void WINAPI WSManCloseShell(
    _Inout_opt_ WSMAN_SHELL_HANDLE shellHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    WSMAN_ERROR error = {0};

    MI_Session_Close(&shellHandle->miSession, NULL, NULL);

    LogFunctionStart("WSManCloseShell");

    async->completionFunction(
            shellHandle->asyncCallback.operationContext,
            0,
            &error,
            shellHandle,
            NULL,
            NULL,
            NULL);
    Batch_Delete(shellHandle->batch);
    LogFunctionEnd("WSManCloseShell", MI_RESULT_OK);
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
