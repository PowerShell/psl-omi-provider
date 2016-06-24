#include <stdlib.h>
#include <pal/strings.h>
#include <base/result.h>
#include <base/logbase.h>
#include <base/log.h>
#include <MI.h>
#include "wsman.h"
#include "BufferManipulation.h"
#include "Shell.h"

#define GOTO_ERROR(message, result) { __LOGE(("%s (result=%u)", message, result)); miResult = result; goto error; }
struct WSMAN_API
{
    MI_Application application;
};

struct WSMAN_SESSION
{
    MI_Session session;
};

struct WSMAN_SHELL
{
    Batch batch;
    WSMAN_SHELL_ASYNC asyncCallback;
    Shell *shellInstance;
    MI_Operation miOperation;
};

MI_EXPORT MI_Uint32 WINAPI WSManInitialize(
    MI_Uint32 flags,
    _Out_ WSMAN_API_HANDLE *apiHandle
    )
{
    MI_Result miResult;

    (*apiHandle) = calloc(1, sizeof(struct WSMAN_API));
    if (*apiHandle == NULL)
        return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    miResult = MI_Application_InitializeV1(0, NULL, NULL, &(*apiHandle)->application);
    if (miResult != MI_RESULT_OK)
    {
        free(*apiHandle);
        *apiHandle = NULL;
    }
    return miResult;
}


MI_EXPORT MI_Uint32 WINAPI WSManDeinitialize(
    _Inout_opt_ WSMAN_API_HANDLE apiHandle,
    MI_Uint32 flags
    )
{
    if (apiHandle)
    {
        MI_Application_Close(&apiHandle->application);
        free(apiHandle);
    }
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
    //const char *resultString = Result_ToString(errorCode);
    //Need to convert it to utf16
    return MI_RESULT_NOT_SUPPORTED;
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
    Batch *tmpBatch = NULL;
    char *connection = NULL;

    *session = NULL;

    if ((serverAuthenticationCredentials == NULL) ||
        (serverAuthenticationCredentials->authenticationMechanism != WSMAN_FLAG_AUTH_BASIC))
    {
        GOTO_ERROR("Only support basic authentication", MI_RESULT_ACCESS_DENIED);
    }

    tmpBatch = Batch_New(BATCH_MAX_PAGES);
    if (tmpBatch == NULL)
    {
        GOTO_ERROR("Out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (_connection && !Utf16LeToUtf8(tmpBatch, _connection, &connection))
    {
        GOTO_ERROR("Failed to convert connection name", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    (*session) = calloc(1, sizeof(struct WSMAN_SESSION));
    if (*session == NULL)
    {
        GOTO_ERROR("Out of memory", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    miResult = MI_Application_NewSession(&apiHandle->application,MI_T("MI_REMOTE_WSMAN" ), connection, NULL, NULL, NULL, &(*session)->session);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("MI_Application_NewSession failed", miResult);
    }

    Batch_Delete(tmpBatch);

    return miResult;

error:
    if (tmpBatch)
        Batch_Delete(tmpBatch);

    if (*session)
    {
        free(*session);
        *session = NULL;
    }
    return miResult;
}

MI_Uint32 WINAPI WSManCloseSession(
    _Inout_opt_ WSMAN_SESSION_HANDLE session,
    MI_Uint32 flags)
{
    MI_Result miResult = MI_Session_Close(&session->session, NULL, NULL);
    free(session);
    return miResult;
}

MI_Uint32 WINAPI WSManSetSessionOption(
    _In_ WSMAN_SESSION_HANDLE session,
    WSManSessionOption option,
    _In_ WSMAN_DATA *data)
{
    /* TODO */
    return MI_RESULT_OK;
}

MI_Uint32 WINAPI WSManGetSessionOptionAsDword(
    _In_ WSMAN_SESSION_HANDLE session,
    WSManSessionOption option,
    _Inout_ MI_Uint32 *value)
{
    /* TODO */
    return MI_RESULT_NOT_SUPPORTED;
}

MI_Uint32 WINAPI WSManGetSessionOptionAsString(
    _In_ WSMAN_SESSION_HANDLE session,
    WSManSessionOption option,
    MI_Uint32 stringLength,
    _Out_writes_to_opt_(stringLength, *stringLengthUsed) MI_Char16* string,
    _Out_ MI_Uint32* stringLengthUsed)
{
    /* TODO */
    return MI_RESULT_NOT_SUPPORTED;
}
MI_Uint32 WINAPI WSManCloseOperation(
    _Inout_opt_ WSMAN_OPERATION_HANDLE operationHandle,
    MI_Uint32 flags)
{
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
    WSMAN_ERROR error;
    memset(&error, 0, sizeof(error));
    error.code = resultCode;
    shell->asyncCallback.completionFunction(
            shell->asyncCallback.operationContext,
            WSMAN_FLAG_CALLBACK_END_OF_OPERATION,
            &error,
            shell,
            NULL,
            NULL,
            NULL);
}

MI_Result ExtractStreamSet(WSMAN_STREAM_ID_SET *streamSet, Batch *batch, MI_Array *array)
{
    MI_Result miResult = MI_RESULT_OK;

    if (streamSet->streamIDsCount > 0)
    {
        array->size = streamSet->streamIDsCount;
        array->data = Batch_Get(batch, array->size * sizeof(void*));
        if (array->data == NULL)
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }

        for (array->size = 0; array->size != streamSet->streamIDsCount; streamSet->streamIDsCount++)
        {
            if (!Utf16LeToUtf8(batch, streamSet->streamIDs[array->size], &array->data[array->size]))
            {
                GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
            }
        }
    }
    else
    {
        memset(array, 0, sizeof(*array));
    }
error:
    return miResult;
}

void WINAPI WSManCreateShellEx(
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
    struct WSMAN_SHELL *shell = NULL;
    char *tmpStr = NULL;

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

    miResult = Instance_New(&_shellInstance, &Shell_rtti, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR("Failed to create instance", miResult);
    }
    shell->shellInstance = (Shell*) _shellInstance;

    if (!Utf16LeToUtf8(batch, shellId, &tmpStr))
    {
        GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    Shell_SetPtr_ShellId(shell->shellInstance, tmpStr);

    if (!Utf16LeToUtf8(batch, resourceUri, &tmpStr))
    {
        GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    Shell_SetPtr_ResourceUri(shell->shellInstance, tmpStr);

    if (startupInfo)
    {
        if (startupInfo->inputStreamSet && startupInfo->inputStreamSet->streamIDsCount)
        {
            miResult = ExtractStreamSet(startupInfo->inputStreamSet, batch, &tmpStr);
            if (miResult != MI_RESULT_OK)
            {
                GOTO_ERROR("Extract input stream failed", miResult);
            }
            Shell_SetPtr_InputStreams(shell->shellInstance, tmpStr);
        }
        if (startupInfo->outputStreamSet && startupInfo->outputStreamSet->streamIDsCount)
        {
            miResult = ExtractStreamSet(startupInfo->outputStreamSet, batch, &tmpStr);
            if (miResult != MI_RESULT_OK)
            {
                GOTO_ERROR("Extract output stream failed", miResult);
            }
            Shell_SetPtr_OutputStreams(shell->shellInstance, tmpStr);
        }
     }

    if (createXml && (createXml->type == WSMAN_DATA_TYPE_TEXT))
    {
        if (!Utf16LeToUtf8(batch, resourceUri, &tmpStr))
        {
            GOTO_ERROR("Alloc failed", MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        Shell_SetPtr_CreationXml(shell->shellInstance, tmpStr);
    }

    {
        MI_OperationCallbacks callbacks = MI_OPERATIONCALLBACKS_NULL;
        callbacks.instanceResult = CreateShellComplete;
        callbacks.callbackContext = shell;

        MI_Session_CreateInstance(&session->session,
                0, /* flags */
                NULL, /*options*/
                "root/interop",
                &shell->shellInstance->__instance,
                &callbacks, &shell->miOperation);
    }


    *_shell = shell;
    return;

error:
    {
        WSMAN_ERROR error;
        error.code = miResult;
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

    *_shell = NULL;
}

void WINAPI WSManRunShellCommandEx(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ const MI_Char16* commandId,
    _In_ const MI_Char16* commandLine,
    _In_opt_ WSMAN_COMMAND_ARG_SET *args,
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_COMMAND_HANDLE *command) // should be closed using WSManCloseCommand
{
}

void WINAPI WSManSignalShell(
    _In_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,         // if NULL, the Signal will be sent to the shell
    MI_Uint32 flags,
    _In_ const MI_Char16* code,             // signal code
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *signalOperation)  // should be closed using WSManCloseOperation
{
}

void WINAPI WSManReceiveShellOutput(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    _In_opt_ WSMAN_COMMAND_HANDLE command,
    MI_Uint32 flags,
    _In_opt_ WSMAN_STREAM_ID_SET *desiredStreamSet,  // request output from a particular stream or list of streams
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_OPERATION_HANDLE *receiveOperation) // should be closed using WSManCloseOperation
{
}

void WINAPI WSManSendShellInput(
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
}

void WINAPI WSManCloseCommand(
    _Inout_opt_ WSMAN_COMMAND_HANDLE commandHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
}

void WINAPI WSManCloseShell(
    _Inout_opt_ WSMAN_SHELL_HANDLE shellHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
    Batch_Delete(shellHandle->batch);
    async->completionFunction(lotsOfParams);
}

void WINAPI WSManDisconnectShell(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_DISCONNECT_INFO* disconnectInfo,
    _In_ WSMAN_SHELL_ASYNC *async)
{
}

void WINAPI WSManReconnectShell(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
}

void WINAPI WSManReconnectShellCommand(
    _Inout_ WSMAN_COMMAND_HANDLE commandHandle,
    MI_Uint32 flags,
    _In_ WSMAN_SHELL_ASYNC *async)
{
}

void WINAPI WSManConnectShell(
    _Inout_ WSMAN_SESSION_HANDLE session,
    MI_Uint32 flags,
    _In_ const MI_Char16* resourceUri,
    _In_ const MI_Char16* shellID,           // shell Identifier
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_opt_ WSMAN_DATA *connectXml,                     // open content for connect shell
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_SHELL_HANDLE *shell) // should be closed using WSManCloseShell
{
}

void WINAPI WSManConnectShellCommand(
    _Inout_ WSMAN_SHELL_HANDLE shell,
    MI_Uint32 flags,
    _In_ const MI_Char16* commandID,  //command Identifier
    _In_opt_ WSMAN_OPTION_SET *options,
    _In_opt_ WSMAN_DATA *connectXml,                // open content for connect command
    _In_ WSMAN_SHELL_ASYNC *async,
    _Out_ WSMAN_COMMAND_HANDLE *command) // should be closed using WSManCloseCommand
{
}
