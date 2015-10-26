#include <iconv.h>
#include <MI.h>
#include "Shell.h"
#include "ShellAPI.h"
#include "BufferManipulation.h"
#include "coreclrutil.h"
#include <pal/strings.h>
#include <pal/format.h>
#include <pal/lock.h>
#include <base/batch.h>
#include <base/instance.h>

/* TODO:
    * No provider (de-)initialization through WSManPluginStartup and WSManPluginShutdown
    * No re-connect
*/


/* Number of characters reserved for command ID and shell ID -- max number of digits for hex 64-bit number with null terminator */
#define ID_LENGTH 17

#define GOTO_ERROR(result) { miResult = result; goto error; }
#define GOTO_ERROR_EX(result, label) { miResult = result; goto label; }

typedef struct _StreamSet
{
    MI_Uint32 streamNamesCount;
    MI_Char16 **streamNames;
} StreamSet;
/* Index for CommonData arrays as well as the type of the CommonData */
typedef enum
{
    CommonData_Type_Shell = 0,
    CommonData_Type_Command = 1,
    CommonData_Type_Send = 2,
    CommonData_Type_Receive = 3,
    CommonData_Type_Signal = 4
} CommonData_Type;

typedef struct _CommonData CommonData;
typedef struct _ShellData ShellData;
typedef struct _CommandData CommandData;
typedef struct _SendData SendData;
typedef struct _ReceiveData ReceiveData;
typedef struct _SignalData SignalData;


struct _CommonData
{
    /* MUST BE FIRST ITEM IN STRUCTURE  as pluginRequest gets cast to CommonData*/
    WSMAN_PLUGIN_REQUEST pluginRequest;
    WSMAN_SENDER_DETAILS senderDetails;
    WSMAN_OPERATION_INFO operationInfo;

    /* Pointer to the owning operation data, either commandData, shellData or NULL if this is the shell */
    CommonData *parentData;

    /* Pointer to sibling operation data, beit other shells, direct children list for shell or direct children list for command */
    CommonData *siblingData;

    /* Allows us to identify if we are a request for a Shell, Command, Send, Receive or Signal request */
    CommonData_Type requestType;

    /* Associated MI_Context for the WSMAN plugin request */
    MI_Context *miRequestContext;

    /* MI_Instance that was passed in for creating instance, or the parameter object passed in to the operation method */
    MI_Instance *miOperationInstance;

    /* Batch allocator for this request data object and all memory we allocate inside this object */
    Batch *batch;

    WSManPluginShutdownCallback shutdownCallback;
    void *shutdownContext;
} ;

struct _ShellData
{
    CommonData common;

    /* This shells ID */
    MI_Char *shellId;

    /* pointer to list of all active child requests, including command, send, receive and signals. We only support 1 active command */
    CommonData *childNext;

    StreamSet inputStreams;
    StreamSet outputStreams;

    WSMAN_SHELL_STARTUP_INFO wsmanStartupInfo;
    WSMAN_DATA extraInfo;

    /* Is the inbound/outbound streams compressed? */
    MI_Boolean isCompressed;

    /* MI provider self pointer that is returned from MI provider load which holds all shell state. When our shell
     * goes away we will need to remove outself from the list 
    */
    Shell_Self *shell;

    void * pluginShellContext;
};

struct _CommandData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to CommandData */
    CommonData common;

    /* This commands ID. There is only one command per shell,
    * but there may be more than one shell in a process.
    */
    MI_Char *commandId;

    /* pointer to list of all active child requests, including send, receive and signals. We only support 1 active command */
    CommonData *childNext;

    WSMAN_COMMAND_ARG_SET wsmanArgSet;

    /* WSMAN shell Plug-in context is the context reported from either the shell or command depending on which type it is */
    void * pluginCommandContext;
};

struct _SendData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to SendData */
    CommonData common;

    WSMAN_DATA inboundData;

};

struct _ReceiveData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to ReceiveData */
    CommonData common;

    StreamSet outputStreams;
    WSMAN_STREAM_ID_SET wsmanOutputStreams;
};

struct _SignalData
{
    /* MUST BE FIRST ITEM IN STRUCTURE as pointer to CommonData gets cast to SignalData */
    CommonData common;

};

/* The master shell object that the provider passes back as context for all provider
 * operations. Currently it only needs to point to the list of shells.
 */
struct _Shell_Self
{
    ShellData *shellList;

    void *pluginContext;
} ;


/* Based on the shell ID, find the existing ShellData object */
ShellData * FindShellFromSelf(struct _Shell_Self *shell, const MI_Char *shellId)
{
    ShellData *shellData = shell->shellList;

    if (shellId == NULL)
        return NULL;

    while (shellData && (Tcscmp(shellId, shellData->shellId) != 0))
    {
        shellData = (ShellData*)shellData->common.siblingData;
    }

    return shellData;
}

/* Shell_Load is called after the provider has been loaded to return
 * the provider schema to the engine. It also allocates and returns our own
 * context object that is passed to all operations that holds the current
 * state of all our shells.
 */
void MI_CALL Shell_Load(Shell_Self** self, MI_Module_Self* selfModule,
        MI_Context* context)
{
    *self = calloc(1, sizeof(Shell_Self));
    if (*self == NULL)
    {
    	MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    else
    {
        MI_Uint32 result = WSManPluginStartup(0, NULL, NULL, &(*self)->pluginContext);

        MI_Context_PostResult(context, result);
    }
}

/* Shell_Unload is called after all operations have completed, or they should
 * be completed. This function is called right before the provider is unloaded.
 * We clean up our shell context object at this point as no more operations will
 * be using it.
 */
void MI_CALL Shell_Unload(Shell_Self* self, MI_Context* context)
{
    /* TODO: Do we have any shells still active? */
    
    WSManPluginShutdown(self->pluginContext, 0, 0);
    /* NOTE: Expectation is that WSManPluginReportCompletion should be called, but it is not looking like that is always happening */

    free(self);
    MI_Context_PostResult(context, MI_RESULT_OK);
}

/* Shell_EnumerateInstances should return all active shell instances to the client.
 * It is not currently supported.
 */
void MI_CALL Shell_EnumerateInstances(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_PropertySet* propertySet, MI_Boolean keysOnly,
        const MI_Filter* filter)
{
	/* Enumerate through the list of shells and post the results back */
	ShellData *shellData = self->shellList;
	MI_Result miResult = MI_RESULT_OK;

	while (shellData)
	{
		miResult = MI_Context_PostInstance(context, shellData->common.miOperationInstance);
		if (miResult != MI_RESULT_OK)
		{
			printf("Shell_EnumerateInstances failed to post instance\n");
			break;
		}

        shellData = (ShellData*) shellData->common.siblingData;
	}
    MI_Context_PostResult(context, miResult);
}

/* Shell_GetInstance should return information about a shell.
 * It is not currently supported.
 */
void MI_CALL Shell_GetInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName, const MI_PropertySet* propertySet)
{
    MI_Result miResult = MI_RESULT_NOT_FOUND;
    ShellData *shellData = FindShellFromSelf(self, instanceName->Name.value);

    if (shellData)
    {
        miResult = MI_Context_PostInstance(context, shellData->common.miOperationInstance);
		if (miResult != MI_RESULT_OK)
		{
			printf("Shell_GetInstances failed to post instance\n");
		}
    }
    MI_Context_PostResult(context, miResult);
}



/* ExtractStreamSet takes a list of streams that are space delimited and
 * copies the data into the ShellData object for the stream. These stream
 * names are allocated and so will need to be deleted when the shell is deleted.
 * We allocate a single buffer for the array and the string, then insert null terminators
 * into the string where spaces were and fix up the array pointers to these strings.
 */
static MI_Boolean ExtractStreamSet(CommonData *commonData, const MI_Char *streams, StreamSet *streamSet)
{
	MI_Char *cursor = (MI_Char*) streams;
	MI_Uint32 i;

	/* Count how many stream names we have so we can allocate the array */
	while (cursor && *cursor)
	{
        streamSet->streamNamesCount++;
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor && *cursor)
		{
			cursor++;
		}
	}

	/* Allocate the buffer for the array and all strings inside the array */
    streamSet->streamNames = Batch_Get(commonData->batch,
			  (sizeof(MI_Char*) * streamSet->streamNamesCount)); /* array length */
	if (streamSet->streamNames == NULL)
		return MI_FALSE;

	/* Point cursor to start of stream names again */
	cursor = (MI_Char*) streams;

	/* Loop through the streams fixing up the array pointers and replace spaces with null terminator */
	for (i = 0; i != streamSet->streamNamesCount; i++)
	{
		/* remember the start of this stream */
        streams = cursor;

		/* Find the next space or end of string */
		cursor = Tcschr(cursor, MI_T(' '));
		if (cursor)
		{
			/* We found a space to replace with null terminator */
			*cursor = MI_T('\0');


	        /* Skip to start of next stream name */
			cursor++;
		}

		/* Fix up the pointer to the stream name */
		if (!Utf8ToUtf16Le(commonData->batch, streams, &streamSet->streamNames[i]))
		{
			return MI_FALSE;
		}
	}
	return MI_TRUE;
}

MI_Boolean ExtractStartupInfo(ShellData *shellData, const Shell *shellInstance)
{
    memset(&shellData->wsmanStartupInfo, 0, sizeof(shellData->wsmanStartupInfo));
    
    if (shellInstance->Name.exists)
    {
		if (!Utf8ToUtf16Le(shellData->common.batch, shellInstance->Name.value, (MI_Char16**)&shellData->wsmanStartupInfo.name))
		{
			return MI_FALSE;
		}
    }

    if (shellInstance->WorkingDirectory.exists)
    {
        if (!Utf8ToUtf16Le(shellData->common.batch, shellInstance->WorkingDirectory.value, (MI_Char16**)&shellData->wsmanStartupInfo.workingDirectory))
        {
            return MI_FALSE;
        }
    }

    if (shellInstance->InputStreams.exists)
    {
        shellData->wsmanStartupInfo.inputStreamSet = Batch_Get(shellData->common.batch, sizeof(*shellData->wsmanStartupInfo.inputStreamSet));
        if (shellData->wsmanStartupInfo.inputStreamSet == NULL)
            return MI_FALSE;

        shellData->wsmanStartupInfo.inputStreamSet->streamIDs = (const MI_Char16**) shellData->inputStreams.streamNames;
        shellData->wsmanStartupInfo.inputStreamSet->streamIDsCount = shellData->inputStreams.streamNamesCount;
    }

    if (shellInstance->OutputStreams.exists)
    {
        shellData->wsmanStartupInfo.outputStreamSet = Batch_Get(shellData->common.batch, sizeof(*shellData->wsmanStartupInfo.outputStreamSet));
        if (shellData->wsmanStartupInfo.outputStreamSet == NULL)
            return MI_FALSE;

        shellData->wsmanStartupInfo.outputStreamSet->streamIDs = (const MI_Char16**) shellData->outputStreams.streamNames;
        shellData->wsmanStartupInfo.outputStreamSet->streamIDsCount = shellData->outputStreams.streamNamesCount;
    }

    if (shellInstance->Environment.exists)
    {
        MI_Uint32 i;
        shellData->wsmanStartupInfo.variableSet = Batch_Get(shellData->common.batch,
            sizeof(WSMAN_ENVIRONMENT_VARIABLE_SET) +
            (sizeof(WSMAN_ENVIRONMENT_VARIABLE) * shellInstance->Environment.value.size));
        if (shellData->wsmanStartupInfo.variableSet == NULL)
            return MI_FALSE;
        shellData->wsmanStartupInfo.variableSet->vars = (WSMAN_ENVIRONMENT_VARIABLE*) (shellData->wsmanStartupInfo.variableSet + 1);
        shellData->wsmanStartupInfo.variableSet->varsCount = shellInstance->Environment.value.size;
        for (i = 0; i != shellInstance->Environment.value.size; i++)
        {
    		if (!Utf8ToUtf16Le(shellData->common.batch, shellInstance->Environment.value.data[i]->Name.value, (MI_Char16**)&shellData->wsmanStartupInfo.variableSet->vars[i].name) ||
    			!Utf8ToUtf16Le(shellData->common.batch, shellInstance->Environment.value.data[i]->Value.value, (MI_Char16**)&shellData->wsmanStartupInfo.variableSet->vars[i].value))
    		{
    			return MI_FALSE;
    		}
        }
    }

    return MI_TRUE;
}

MI_Boolean ExtractOperationInfo(MI_Context *context, CommonData *commonData)
{
    MI_Uint32 count;

    if (MI_Context_GetCustomOptionCount(context, &count) != MI_RESULT_OK)
        return MI_FALSE;

    commonData->pluginRequest.operationInfo = &commonData->operationInfo;

    /* Allocate enough space for all of them even though we may not need to use them all */
    commonData->operationInfo.optionSet.options = Batch_GetClear(commonData->batch, sizeof(WSMAN_OPTION)*count);
    if (commonData->operationInfo.optionSet.options == NULL)
        return MI_FALSE;

    for (; count; count--)
    {
        MI_Value value;
        const MI_Char *name;
        MI_Type type;

        if (MI_Context_GetCustomOptionAt(context, count - 1, &name, &type, &value) != MI_RESULT_OK)
            return MI_FALSE;

        if (type != MI_STRING)
            continue;

        /* Skip the internal headers */
        if ((Tcsncmp(name, MI_T("WSMAN_"), 6) == 0) || (Tcsncmp(name, MI_T("HTTP_"), 5) == 0))
        {
            continue;
        }

		if (!Utf8ToUtf16Le(commonData->batch, name, (MI_Char16**)&commonData->operationInfo.optionSet.options[commonData->operationInfo.optionSet.optionsCount].name) ||
			!Utf8ToUtf16Le(commonData->batch, value.string, (MI_Char16**)&commonData->operationInfo.optionSet.options[commonData->operationInfo.optionSet.optionsCount].value))
		{
			return MI_FALSE;
		}
        commonData->operationInfo.optionSet.optionsCount++;
    }

    return MI_TRUE;
}

MI_Boolean ExtractPluginRequest(MI_Context *context, CommonData *commonData)
{
    const MI_Char *value;
    
    if (MI_Context_GetStringOption(context, MI_T("WSMAN_ResourceURI"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->pluginRequest.resourceUri))
    	{
    		return MI_FALSE;
    	}
    }
    if (MI_Context_GetStringOption(context, MI_T("WSMAN_Locale"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->pluginRequest.locale))
    	{
    		return MI_FALSE;
    	}
    }
    if (MI_Context_GetStringOption(context, MI_T("WSMAN_DataLocale"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->pluginRequest.dataLocale))
    	{
    		return MI_FALSE;
    	}
    }

    commonData->pluginRequest.senderDetails = &commonData->senderDetails;

    if (MI_Context_GetStringOption(context, MI_T("HTTP_URL"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->senderDetails.httpURL))
    	{
    		return MI_FALSE;
    	}
    }
    
    if (MI_Context_GetStringOption(context, MI_T("HTTP_USERNAME"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->senderDetails.senderName))
    	{
    		return MI_FALSE;
    	}
    }

    if (MI_Context_GetStringOption(context, MI_T("HTTP_AUTHORIZATION"), &value) == MI_RESULT_OK)
    {
    	if (!Utf8ToUtf16Le(commonData->batch, value, (MI_Char16**)&commonData->senderDetails.authenticationMechanism))
    	{
    		return MI_FALSE;
    	}
    }

    return ExtractOperationInfo(context, commonData);
}

MI_Boolean ExtractExtraInfo(ShellData *shellData, const Shell* newInstance)
{
    WSMAN_DATA *pExtraInfo = NULL;
    
    size_t toBytesTotal;
    size_t toBytesLeft;
    char *toBuffer;
    char *toBufferCurrent;
    MI_Boolean returnValue = MI_FALSE;

    char *fromBuffer;
    size_t fromBytesLeft;

    size_t creationXmlLength = Tcslen(newInstance->CreationXml.value);

    size_t iconv_return;
    iconv_t iconvData;

    iconvData = iconv_open("UTF-16LE", "UTF-8");
    if (iconvData == (iconv_t)-1)
    {
    	goto cleanup;
    }

    toBytesTotal = (sizeof("<CreationXml>") - 1 + creationXmlLength + sizeof("</CreationXml>") ) * 2; /* Assume buffer is twice as big. */
    toBytesLeft = toBytesTotal;

    toBuffer = Batch_Get(shellData->common.batch, toBytesTotal);
    if (toBuffer == NULL)
    	goto cleanup;
    toBufferCurrent = toBuffer;


    fromBytesLeft =  sizeof("<CreationXml>") - 1; /* Remove null terminator from this one */
    fromBuffer = "<CreationXml>";
    iconv_return = iconv(iconvData, &fromBuffer, &fromBytesLeft, &toBufferCurrent, &toBytesLeft);
    if (iconv_return == (size_t) -1)
    {
    	goto cleanup;
    }
    fromBytesLeft = creationXmlLength;
    fromBuffer = (char*)newInstance->CreationXml.value;
    iconv_return = iconv(iconvData, &fromBuffer, &fromBytesLeft, &toBufferCurrent, &toBytesLeft);
    if (iconv_return == (size_t)-1)
    {
    	goto cleanup;
    }

    fromBytesLeft = sizeof("</CreationXml>"); /* We want the null terminator this time */
    fromBuffer = "</CreationXml>";
    iconv_return = iconv(iconvData, &fromBuffer, &fromBytesLeft, &toBufferCurrent, &toBytesLeft);
    if (iconv_return == (size_t)-1)
    {
    	goto cleanup;
    }

    pExtraInfo = &shellData->extraInfo;
    pExtraInfo->type = WSMAN_DATA_TYPE_TEXT;
    pExtraInfo->text.bufferLength = toBytesTotal - (toBytesTotal- toBytesLeft); /* Adjust for any unused buffer */
    pExtraInfo->text.buffer = (const MI_Char16*) toBuffer;

    returnValue = MI_TRUE;

cleanup:
	if (iconvData != (iconv_t)-1)
		iconv_close(iconvData);

    return returnValue;
}

/* Shell_CreateInstance
 * Called by the client to create a shell. The shell is given an ID by us and sent back.
 * The list of streams that a command could have is listed out in the shell instance passed
 * to us. Other stuff can be included but we are ignoring that for the moment.
 */
void MI_CALL Shell_CreateInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* newInstance)
{
    ShellData *shellData = NULL;
    WSMAN_DATA *pExtraInfo = NULL;
    MI_Result miResult;
    MI_Instance *miOperationInstance = NULL;
    Batch *batch;

    /* Allocate our shell data out of a batch so we can allocate most of it from a single page and free it easily */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Create an internal representation of the shell object that can can use to
     * hold state. It is based on this recored data that a client should be able
     * to do an EnumerateInstance call to the provider.
     * We also allocate enough space for the shell ID string.
     */
    shellData = Batch_GetClear(batch, sizeof(*shellData));
    if (shellData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    shellData->common.batch = batch;

    /* Set the shell ID to be the pointer within the bigger buffer */
    shellData->shellId = Batch_Get(batch, sizeof(MI_Char)*ID_LENGTH);
    if (shellData->shellId == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (Stprintf(shellData->shellId, ID_LENGTH, MI_T("%llx"), (MI_Uint64) shellData->shellId) < 0)
    {
        GOTO_ERROR(MI_RESULT_FAILED);
    }

    /* Create an instance of the shell that we can send for the result of this Create as well as a get/enum operation*/
    /* Note: Instance is allocated from the batch so will be deleted when the shell batch is destroyed */
    if (Instance_Clone(&newInstance->__instance, &miOperationInstance, batch) != MI_RESULT_OK)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if ((miResult = Shell_SetPtr_Name((Shell*)miOperationInstance, shellData->shellId)) != MI_RESULT_OK)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Extract the outbound stream names that are space delimited into an
     * actual array of strings
     */
    if (!ExtractStreamSet(&shellData->common, newInstance->InputStreams.value, &shellData->inputStreams))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    if (!ExtractStreamSet(&shellData->common, newInstance->OutputStreams.value, &shellData->outputStreams))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!ExtractStartupInfo(shellData, newInstance))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!ExtractPluginRequest(context, &shellData->common))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (newInstance->CreationXml.value)
    {
        if (!ExtractExtraInfo(shellData, newInstance))
        {
            GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
        }
        pExtraInfo = &shellData->extraInfo;
    }

    {
    	MI_Value value;
    	MI_Type type;
    	MI_Uint32 flags;
    	MI_Uint32 index;

		if ((MI_Instance_GetElement(&newInstance->__instance, MI_T("IsCompressed"), &value, &type, &flags, &index) == 0) &&
				(type == MI_BOOLEAN) &&
				value.boolean)
		{
			shellData->isCompressed = MI_TRUE;
		}
    }

    /* TODO: Fill in shellData->common.pluginRequest */
    shellData->common.parentData = NULL;    /* We are the top-level shell object */
    shellData->common.requestType = CommonData_Type_Shell;
    shellData->common.miRequestContext = context;
    shellData->common.miOperationInstance = miOperationInstance;

    /* Plumb this shell into our list. Failure paths after this need to unplumb it!
    */
    shellData->common.siblingData = (CommonData *)self->shellList;
    self->shellList = shellData;
    shellData->shell = self;

    /* Call out to external plug-in API to continue shell creation.
     * Acceptance of shell is reported through WSManPluginReportContext.
     * If the shell succeeds or fails we will get a call through WSManPluginOperationComplete
     */
    WSManPluginShell(self->pluginContext, &shellData->common.pluginRequest, 0, &shellData->wsmanStartupInfo, pExtraInfo);
    return;

error:

    if (batch)
        Batch_Delete(batch);

    MI_Context_PostResult(context, MI_RESULT_SERVER_LIMITS_EXCEEDED);
}


/* Don't support modifying a shell instance */
void MI_CALL Shell_ModifyInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* modifiedInstance, const MI_PropertySet* propertySet)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void RecursiveNotifyShutdown(CommonData *commonData)
{
    CommonData *child = NULL;;

    /* If there are children notify them first */
    if (commonData->requestType == CommonData_Type_Shell)
    {
        child = ((ShellData*)commonData)->childNext;
    }
    else if (commonData->requestType == CommonData_Type_Command)
    {
        child = ((CommandData*)commonData)->childNext;
    }

    while (child)
    {
        RecursiveNotifyShutdown(child);
        child = child->siblingData;
    }

    /* Now notify for this object if a shutdown registration is present */
    if (commonData->shutdownCallback)
        commonData->shutdownCallback(commonData->shutdownContext);
}

/* Delete a shell instance. This should not be done by the client until
 * the command is finished and shut down.
 */
void MI_CALL Shell_DeleteInstance(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const Shell* instanceName)
{
    ShellData *shellData;
    MI_Result miResult = MI_RESULT_NOT_FOUND;

    shellData = FindShellFromSelf(self, instanceName->Name.value);

    if (shellData)
    {
        /* Notify shell to shut itself down. We don't delete things
           here because the shell itself will tell us when it is finished
           */
        RecursiveNotifyShutdown((CommonData*)shellData);

        /* TODO: Do we need to wait for the shell shutdown to complete? */

        miResult = MI_RESULT_OK;
    }

    MI_Context_PostResult(context, miResult);
}

MI_Boolean ExtractCommandArgs(CommonData *commonData, Shell_Command* shellInstance, WSMAN_COMMAND_ARG_SET *wsmanArgSet)
{
    if (shellInstance->arguments.exists)
    {
        MI_Uint32 i;

        wsmanArgSet->args = Batch_Get(commonData->batch, shellInstance->arguments.value.size * sizeof(MI_Char*));
        if (wsmanArgSet->args == NULL)
        {
            return MI_FALSE;
        }

        for (i = 0; i != shellInstance->arguments.value.size; i++)
        {
        	if (!Utf8ToUtf16Le(commonData->batch, shellInstance->arguments.value.data[i], (MI_Char16**)&wsmanArgSet->args[i]))
        	{
        		return MI_FALSE;
        	}
        }
        wsmanArgSet->argsCount = shellInstance->arguments.value.size;
    }
    return MI_TRUE;
}

MI_Boolean AddChild(CommonData **childHeadPointer, CommonData *childData)
{
    CommonData *currentChild = *childHeadPointer;

    while (currentChild)
    {
        if (currentChild->requestType == childData->requestType)
        {
            /* Already have one of those */
            return MI_FALSE;
        }

        currentChild = currentChild->siblingData;
    }

    /* Not found this type so we can add it */
    childData->siblingData = *childHeadPointer;
    *childHeadPointer = childData;

    return MI_TRUE;
}

/* Initiate a command on a given shell. The command has parameters and the likes
 * and when created inbound/outbound streams are sent/received via the Send/Receive
 * methods while specifying this command ID. The command is alive until a signal is
 * sent telling us is it officially finished.
 */
void MI_CALL Shell_Invoke_Command(Shell_Self* self, MI_Context* context,
    const MI_Char* nameSpace, const MI_Char* className,
    const MI_Char* methodName, const Shell* instanceName,
    const Shell_Command* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData;
    CommandData *commandData;
    MI_Instance *miOperationInstance = NULL;
    Batch *batch = NULL;
    MI_Char16 *command = NULL;

    shellData = FindShellFromSelf(self, instanceName->Name.value);

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Allocate our shell data out of a batch so we can allocate most of it from a single page and free it easily */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* Create internal command structure.  */
    commandData = Batch_GetClear(batch, sizeof(CommandData));
    if (!commandData)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    commandData->common.batch = batch;

    commandData->commandId = Batch_Get(batch, sizeof(MI_Char)*ID_LENGTH);
    if (!commandData->commandId)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    Stprintf(commandData->commandId, ID_LENGTH, MI_T("%llx"), (MI_Uint64)commandData->commandId);

    /* Create command instance to send back to client */
    miResult = Instance_Clone(&in->__instance, &miOperationInstance, batch);
    if (miResult != MI_RESULT_OK)
    {
    	GOTO_ERROR(miResult);
    }

    Shell_Command_SetPtr_CommandId((Shell_Command*) miOperationInstance, commandData->commandId);
    
    if (!ExtractCommandArgs(&commandData->common, (Shell_Command*)miOperationInstance, &commandData->wsmanArgSet))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!ExtractPluginRequest(context, &shellData->common))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!Utf8ToUtf16Le(batch, ((Shell_Command*)miOperationInstance)->command.value, &command))
    {
    	GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    /* TODO: Fill in shellData->command->common.pluginRequest */
    commandData->common.parentData = (CommonData*)shellData;
    commandData->common.requestType = CommonData_Type_Command;
    commandData->common.miRequestContext = context;
    commandData->common.miOperationInstance = miOperationInstance;

    if (!AddChild(&shellData->childNext, (CommonData*) commandData))
    {
        GOTO_ERROR(MI_RESULT_ALREADY_EXISTS); /* Only one command allowed at a time */
    }

    WSManPluginCommand(
    		&commandData->common.pluginRequest,
			0,
			shellData->pluginShellContext,
            command,
            &commandData->wsmanArgSet);

    /* Success path will send the response back from the callback from this API*/
    return;

error:
    /* Cleanup all the memory and post the error back*/

    if (batch)
        Batch_Delete(batch);

    MI_Context_PostResult(context, miResult);

}

CommandData *FindCommandFromShell(const ShellData *shell, const MI_Char *commandId)
{
    CommonData *child = shell->childNext;

    if (commandId == NULL)
        return NULL;

    while (child)
    {
        if (child->requestType == CommonData_Type_Command)
        {
            CommandData *command = (CommandData*)child;
            if (Tcscmp(commandId, command->commandId) == 0)
            {
                return command;
            }
        }

        child = child->siblingData;
    }

    return NULL;
}

/* Shell_Invoke_Send
 *
 * This CIM method is called when the client is delivering a chunk of data to the shell.
 * It can be delivering it to the shell itself or to a command, depending on if the
 * commandId parameter is present.
 * This test provider will deliver anything that is sent back to the client through
 * a pending Receive methods result.
 */
void MI_CALL Shell_Invoke_Send(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Send* in)
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Uint32 pluginFlags = 0;
    ShellData *shellData = FindShellFromSelf(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    SendData *sendData = NULL;
    Batch *batch = NULL;
    DecodeBuffer decodeBuffer, decodedBuffer;
    MI_Instance *clonedIn = NULL;
    MI_Char16 *streamName;
    
    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
	memset(&decodedBuffer, 0, sizeof(decodedBuffer));

	/* Was the shell ID the one we already know about? */
    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }

    /* Check to make sure the command ID is correct if this send is aimed at the command */
    if (in->streamData.value->commandId.exists)
    {
        commandData = FindCommandFromShell(shellData, in->streamData.value->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
    }

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    sendData = Batch_GetClear(batch, sizeof(SendData));
    if (sendData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    sendData->common.batch = batch;

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    /* We may not actually have any data but we may be completing the data. Make sure
     * we are only processing the inbound stream if we have some data to process.
     */
    if (in->streamData.value->data.exists)
    {
        decodeBuffer.buffer = (MI_Char*)in->streamData.value->data.value;
        decodeBuffer.bufferLength = in->streamData.value->dataLength.value * sizeof(MI_Char);
        decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

        /* Base-64 decode the data from decodeBuffer to decodedBuffer. The result buffer
         * gets allocated in this function and we need to free it.*/
         /* TODO: This does not support unicode MI_Char strings */
        miResult = Base64DecodeBuffer(&decodeBuffer, &decodedBuffer);
        if (miResult != MI_RESULT_OK)
        {
            /* decodeBuffer.buffer does not need deleting */
            GOTO_ERROR(miResult);
        }

        /* decodeBuffer.buffer does not need freeing as it was from method in parameters. */
        /* We switch the decodedBuffer to decodeBuffer for further processing. */
        decodeBuffer = decodedBuffer;
        memset(&decodedBuffer, 0, sizeof(decodedBuffer));

        if (shellData->isCompressed)
        {
            /* Decompress it from decodeBuffer to decodedBuffer. The result buffer
             * gets allocated in this function and we need to free it.
             */
            miResult = DecompressBuffer(&decodeBuffer, &decodedBuffer);
            if (miResult != MI_RESULT_OK)
            {
                free(decodeBuffer.buffer);
                decodeBuffer.buffer = NULL;
                GOTO_ERROR(miResult);
            }

            /* Free the previously allocated buffer and switch the
             * decodedBuffer back to decodeBuffer for further processing.
             */
            free(decodeBuffer.buffer);
            decodeBuffer = decodedBuffer;
        }
    }
    else
    {
        pluginFlags = WSMAN_FLAG_SEND_NO_MORE_DATA;
    }

    if (!ExtractPluginRequest(context, &shellData->common))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!Utf8ToUtf16Le(batch, in->streamData.value->streamName.value, &streamName))
    {
    	GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    {
        sendData->inboundData.type = WSMAN_DATA_TYPE_BINARY;
        sendData->inboundData.binaryData.data = (MI_Uint8*)decodeBuffer.buffer;
        sendData->inboundData.binaryData.dataLength = decodeBuffer.bufferUsed;

        sendData->common.miRequestContext = context;
        sendData->common.miOperationInstance = clonedIn;
        sendData->common.requestType = CommonData_Type_Send;
        
        if (commandData)
        {
            sendData->common.parentData = (CommonData*)commandData;

            if (!AddChild(&commandData->childNext, (CommonData*)sendData))
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }

            WSManPluginSend(
                &sendData->common.pluginRequest,
                pluginFlags,
                shellData->pluginShellContext,
                commandData->pluginCommandContext,
                streamName,
                &sendData->inboundData);
        }
        else
        {
            sendData->common.parentData = (CommonData*) shellData;

            if (!AddChild(&shellData->childNext, (CommonData*)sendData))
            {
                GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
            }

            WSManPluginSend(
                &sendData->common.pluginRequest,
                pluginFlags,
                shellData->pluginShellContext,
                NULL,
                streamName,
                &sendData->inboundData);
        }
    }
    /* Now the plugin has been called the result is sent from the WSManPluginOperationComplete callback */
    return;

error:
    if (decodedBuffer.buffer)
	    free(decodedBuffer.buffer);

	MI_Context_PostResult(context, miResult);

    if (batch)
        Batch_Delete(batch);
}

/* Shell_Invoke_Receive
 * This gets called to queue up a receive of output from the provider when there is enough
 * data to send.
 * In our test provider all we are doing is sending the data back that we received in Send
 * so we are caching the Receive context and wake up any pending Send that is waiting
 * for us.
 */
void MI_CALL Shell_Invoke_Receive(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Receive* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData = FindShellFromSelf(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    ReceiveData *receiveData = NULL;
    Batch *batch = NULL;
    MI_Instance *clonedIn = NULL;

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists)
    {
        commandData = FindCommandFromShell(shellData, in->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }

        /* Find an existing receiveData is one exists */
        {
            CommonData *tmp = commandData->childNext;
            while (tmp && (tmp->requestType != CommonData_Type_Receive))
            {
                tmp = tmp->siblingData;
            }
            receiveData = (ReceiveData*) tmp;
        }
    }
    else
    {
        /* Find an existing receiveData is one exists */
        CommonData *tmp = shellData->childNext;
        while (tmp && (tmp->requestType != CommonData_Type_Receive))
        {
            tmp = tmp->siblingData;
        }
        receiveData = (ReceiveData*)tmp;
    }


    if (receiveData)
    {
        /* We already have a Receive queued up with the plug-in so cache the context and wake it up in case it is waiting for it */
        receiveData->common.miRequestContext = context;
        CondLock_Broadcast((ptrdiff_t)&receiveData->common.miRequestContext);
        return;
    }

    /* new one */
    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    receiveData = Batch_GetClear(batch, sizeof(ReceiveData));
    if (receiveData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    receiveData->common.batch = batch;

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    if (!ExtractStreamSet(&receiveData->common, ((Shell_Receive*)clonedIn)->streamSet.value, &receiveData->outputStreams))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (!ExtractPluginRequest(context, &shellData->common))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    receiveData->wsmanOutputStreams.streamIDsCount = receiveData->outputStreams.streamNamesCount;
    receiveData->wsmanOutputStreams.streamIDs = (const MI_Char16**) receiveData->outputStreams.streamNames;

    receiveData->common.miRequestContext = context;
    receiveData->common.miOperationInstance = clonedIn;
    receiveData->common.requestType = CommonData_Type_Receive;

    if (commandData)
    {
        receiveData->common.parentData = (CommonData*)commandData;

        if (!AddChild(&commandData->childNext, (CommonData*)receiveData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginReceive(
            &receiveData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            commandData->pluginCommandContext,
            &receiveData->wsmanOutputStreams);
    }
    else
    {
        receiveData->common.parentData = (CommonData*)shellData;

        if (!AddChild(&shellData->childNext, (CommonData*)receiveData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginReceive(
            &receiveData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            NULL,
            &receiveData->wsmanOutputStreams);
    }

    /* Posting on receive context happens when we get WSManPluginOperationComplete callback to terminate the request or WSManPluginReceiveResult with some data */
    return;

error:

    MI_Context_PostResult(context, miResult);
    
    if (batch)
    {
        Batch_Delete(batch);
    }
}

/* Shell_Invoke_Signal
 * This function is called for a few reasons like hitting Ctrl+C. It is
 * also called to signal the completion of a command from the clients perspective
 * and the command is not considered complete until this fact.
 * In reality we may time out a command if we don't receive any data
 * but for the sake of this provider we don't care.
 */
void MI_CALL Shell_Invoke_Signal(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Signal* in)
{
    MI_Result miResult = MI_RESULT_OK;
    ShellData *shellData = FindShellFromSelf(self, instanceName->Name.value);
    CommandData *commandData = NULL;
    SignalData *signalData = NULL;
    Batch *batch = NULL;
    MI_Instance *clonedIn = NULL;
    MI_Char16 *signalCode = NULL;

    if (!shellData)
    {
        GOTO_ERROR(MI_RESULT_NOT_FOUND);
    }
    if (!in->code.exists)
    {
        GOTO_ERROR(MI_RESULT_NOT_SUPPORTED);
    }
    /* If we have a command ID make sure it is the correct one */
    if (in->commandId.exists)
    {
        commandData = FindCommandFromShell(shellData, in->commandId.value);
        if (commandData == NULL)
        {
            GOTO_ERROR(MI_RESULT_NOT_FOUND);
        }
    }

    batch = Batch_New(BATCH_MAX_PAGES);
    if (batch == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    signalData = Batch_GetClear(batch, sizeof(SignalData));
    if (signalData == NULL)
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }
    signalData->common.batch = batch;

    miResult = Instance_Clone(&in->__instance, &clonedIn, batch);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR(miResult);
    }

    if (!ExtractPluginRequest(context, &shellData->common))
    {
        GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
    }

    if (((Shell_Signal*)clonedIn)->code.value)
    {
		if (!Utf8ToUtf16Le(batch, ((Shell_Signal*)clonedIn)->code.value, &signalCode))
		{
			GOTO_ERROR(MI_RESULT_SERVER_LIMITS_EXCEEDED);
		}
    }
    signalData->common.miRequestContext = context;
    signalData->common.miOperationInstance = clonedIn;
    signalData->common.requestType = CommonData_Type_Signal;

    if (commandData)
    {
        signalData->common.parentData = (CommonData*)commandData;

        if (!AddChild(&commandData->childNext, (CommonData*)signalData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginSignal(
            &signalData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            commandData->pluginCommandContext,
            signalCode);
    }
    else
    {
        signalData->common.parentData = (CommonData*)shellData;

        if (!AddChild(&shellData->childNext, (CommonData*)signalData))
        {
            GOTO_ERROR(MI_RESULT_ALREADY_EXISTS);
        }

        WSManPluginSignal(
            &signalData->common.pluginRequest,
            0,
            shellData->pluginShellContext,
            NULL,
            signalCode);
    }

    /* Posting on signal context happens when we get a WSManPluginOperationComplete callback */
    return;

error:

	MI_Context_PostResult(context, miResult);

    if (batch)
    {
        Batch_Delete(batch);
    }
}

/* Connect allows the client to re-connect to a shell that was started from a different remote machine and continue getting output delivered to a new client */
void MI_CALL Shell_Invoke_Connect(Shell_Self* self, MI_Context* context,
        const MI_Char* nameSpace, const MI_Char* className,
        const MI_Char* methodName, const Shell* instanceName,
        const Shell_Connect* in)
{
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

ShellData *GetShellFromOperation(CommonData *commonData)
{
    if (commonData->parentData == NULL)
        return (ShellData*)commonData;
    
    return GetShellFromOperation(commonData->parentData);
}

CommandData *GetCommandFromOperation(CommonData *commonData)
{
    if (commonData == NULL)
        return NULL;
    if (commonData->requestType == CommonData_Type_Command)
        return (CommandData *)commonData;

    return GetCommandFromOperation(commonData->parentData);
}

/* report a shell or command context from teh winrm plugin. We use this for future calls into the plugin.
 This also means the shell or command has been started successfully so we can post the operation instance
 back to the client to indicate to them that they can now post data to the operation or receive data back from it.
 */
MI_Uint32 MI_CALL WSManPluginReportContext(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ void * context
    )
{
    CommonData *commonData = (CommonData*) requestDetails;
    MI_Result miResult;

    /* Grab the providers context, which may be shell or command, and store it in our object */
    if (commonData->requestType == CommonData_Type_Shell)
    {
        ((ShellData*)commonData)->pluginShellContext = context;
    }
    else if (commonData->requestType == CommonData_Type_Command)
    {
        ((CommandData*)commonData)->pluginCommandContext = context;
    }
    else
    {
        return MI_RESULT_INVALID_PARAMETER;
    }

    /* Post our shell or command object back to the client */
    if (((miResult = MI_Context_PostInstance(commonData->miRequestContext, commonData->miOperationInstance)) != MI_RESULT_OK) ||
        ((miResult = MI_Context_PostResult(commonData->miRequestContext, miResult)) != MI_RESULT_OK))
    {
        /* Something failed. They will call WSManPluginOperationComplete to report it is done/failed though so we will clean up in there */
    }
	return miResult;
}

/* Max recursion is for a child operation of a command which is 2 recursions to hit the shell */
MI_Boolean IsStreamCompressed(CommonData *commonData)
{
    if (commonData->parentData == NULL)
    {
        ShellData *shellData = (ShellData*)commonData;
        return shellData->isCompressed;
    }
    return IsStreamCompressed(commonData->parentData);
}

/*
 * The WSMAN plug-in gets called once and it keeps sending data back to us.
 * At our level, however, we need to potentially wait for the next request
 * before we can send more.
 */
MI_Uint32 MI_CALL WSManPluginReceiveResult(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_opt_ const MI_Char16 * _streamName,
    _In_opt_ WSMAN_DATA *streamResult,
    _In_opt_ const MI_Char16 * _commandState,
    _In_ MI_Uint32 exitCode
    )
{
    MI_Result miResult;
    CommonData *commonData = (CommonData*)requestDetails;
    MI_Context *receiveContext = NULL;
    CommandState commandStateInst;
    Shell_Receive *receive = NULL;
    Stream receiveStream;
    DecodeBuffer decodeBuffer, decodedBuffer;
    MI_Char *streamName = NULL;
    MI_Char *commandState = NULL;
    Batch *tempBatch;

    memset(&decodeBuffer, 0, sizeof(decodeBuffer));
    memset(&decodedBuffer, 0, sizeof(decodedBuffer));

    if (commonData->requestType != CommonData_Type_Receive)
        return MI_RESULT_INVALID_PARAMETER;

    tempBatch = Batch_New(BATCH_MAX_PAGES);
    if (tempBatch == NULL)
    	return MI_RESULT_SERVER_LIMITS_EXCEEDED;

    if (_streamName && !Utf16LeToUtf8(tempBatch, _streamName, &streamName))
    {
    	GOTO_ERROR_EX(MI_RESULT_SERVER_LIMITS_EXCEEDED, errorSkipInstanceDeletes);
    }
    if (_commandState && !Utf16LeToUtf8(tempBatch, _commandState, &commandState))
    {
    	GOTO_ERROR_EX(MI_RESULT_SERVER_LIMITS_EXCEEDED, errorSkipInstanceDeletes);
    }
    /* TODO: Which stream if stream is NULL? */

    /* Wait for a Receive request to come in before we post the result back */
    do
    {
    } while (CondLock_Wait((ptrdiff_t)&commonData->miRequestContext,
                           (ptrdiff_t*)&commonData->miRequestContext,
                           0, 
                           CONDLOCK_DEFAULT_SPINCOUNT) == 0);
    
    /* Copy off the receive context. Another Receive should not happen at the same time but this makes
    * sure we are the only one processing it. This will also help to protect us if a Signal comes in
    * to shut things down as we are now responsible for delivering its results.
    */
    receiveContext = commonData->miRequestContext;
    commonData->miRequestContext = NULL;

    /* Clone the result object for Receive because we will reuse it for each receive response */
    miResult = Instance_Clone(commonData->miOperationInstance, (MI_Instance**)&receive, NULL);
    if (miResult != MI_RESULT_OK)
    {
        GOTO_ERROR_EX(miResult, errorSkipInstanceDeletes);
    }

    miResult =  CommandState_Construct(&commandStateInst, receiveContext);
    if (miResult != MI_RESULT_OK)
    {
        Shell_Receive_Delete(receive);
        GOTO_ERROR_EX(miResult, errorSkipInstanceDeletes);
    }
    miResult = Stream_Construct(&receiveStream, receiveContext);
    if (miResult != MI_RESULT_OK)
    {
        Stream_Destruct(&receiveStream);
        Shell_Receive_Delete(receive);
        GOTO_ERROR_EX(miResult, errorSkipInstanceDeletes);
    }

    /* Set the command ID for the instances that need it */
    if (commonData->parentData->requestType == CommonData_Type_Command)
    {
        CommandData *commandData = (CommandData*)commonData->parentData;
        CommandState_SetPtr_commandId(&commandStateInst, commandData->commandId);
        Stream_SetPtr_commandId(&receiveStream, commandData->commandId);
    }

    if (streamResult)
    {
        decodeBuffer.buffer = (MI_Char*)streamResult->binaryData.data;
        decodeBuffer.bufferLength = streamResult->binaryData.dataLength;
        decodeBuffer.bufferUsed = decodeBuffer.bufferLength;

        if (IsStreamCompressed(commonData))
        {
            /* Re-compress it from decodeBuffer to decodedBuffer. The result buffer
             * gets allocated in this function and we need to free it.
             */
            miResult = CompressBuffer(&decodeBuffer, &decodedBuffer, sizeof(MI_Char));
            if (miResult != MI_RESULT_OK)
            {
                decodeBuffer.buffer = NULL;
                GOTO_ERROR(miResult);
            }

            /* switch the decodedBuffer back to decodeBuffer for further processing.
             */
            decodeBuffer = decodedBuffer;
        }

        /* TODO: This does not support unicode MI_Char strings */
        /* NOTE: Base64EncodeBuffer allocates enough space for a NULL terminator */
        miResult = Base64EncodeBuffer(&decodeBuffer, &decodedBuffer);

        /* Free previously allocated buffer if it was compressed. */
        if (IsStreamCompressed(commonData))
        {
            free(decodeBuffer.buffer);
        }
        decodeBuffer.buffer = NULL;

        if (miResult != MI_RESULT_OK)
        {
            GOTO_ERROR(miResult);
        }

        /* Set the null terminator on the end of the buffer as this is supposed to be a string*/
        memset(decodedBuffer.buffer + decodedBuffer.bufferUsed, 0, sizeof(MI_Char));
        
        /* Add the final string to the stream. This is just a pointer to it and
        * is not getting copied so we need to delete the buffer after we have posted the instance
        * to the receive context.
        */
        Stream_SetPtr_data(&receiveStream, decodedBuffer.buffer);
    }
    
    /* CommandState tells the client if we are done or not with this stream.
    */

    if (commandState &&
        (Tcscasecmp(commandState, WSMAN_COMMAND_STATE_DONE) == 0))
    {
        /* TODO: Mark stream as complete for either shell or command */
        CommandState_SetPtr_state(&commandStateInst, WSMAN_COMMAND_STATE_DONE);
    }
    else if (commandState)
    {
        CommandState_SetPtr_state(&commandStateInst, commandState);
    }
    else
    {
        CommandState_SetPtr_state(&commandStateInst, WSMAN_COMMAND_STATE_RUNNING);
    }

    /* Stream holds the results of the inbound/outbound stream. A result can have more
    * than one stream, either for the same stream or different ones.
    */
    if (flags & WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA)
        Stream_Set_endOfStream(&receiveStream, MI_TRUE);
    else
        Stream_Set_endOfStream(&receiveStream, MI_FALSE);

    if (streamName)
    {
        Stream_SetPtr_streamName(&receiveStream, streamName);
    }

    /* The result of the Receive contains the command results and a set of streams.
    * We only support one stream at a time for now.
    */
    Shell_Receive_Set_MIReturn(receive, MI_RESULT_OK);
    Shell_Receive_SetPtr_CommandState(receive, &commandStateInst);


    /* Add the stream embedded instance to the receive result.  */
    Shell_Receive_SetPtr_Stream(receive, &receiveStream);

    /* Post the result back to the client. We can delete the base-64 encoded buffer after
    * posting it.
    */
    Shell_Receive_Post(receive, receiveContext);

error:
    /* Clean up the various result objects */
    CommandState_Destruct(&commandStateInst);
    Stream_Destruct(&receiveStream);
    Shell_Receive_Delete(receive);

errorSkipInstanceDeletes:
    MI_Context_PostResult(receiveContext, miResult);
    if (tempBatch)
    	Batch_Delete(tempBatch);

    if (decodedBuffer.buffer)
    	free(decodedBuffer.buffer);

    return (MI_Uint32) miResult;

}

/* PowerShell only uses WSMAN_PLUGIN_PARAMS_GET_REQUESTED_LOCALE and WSMAN_PLUGIN_PARAMS_GET_REQUESTED_DATA_LOCALE 
 * which are optional wsman packet headers.
 * Other things that could be retrieved are things like operation timeout. 
 * This data should be retrieved from the operation context through the operation options method.
 */
MI_Uint32 MI_CALL WSManPluginGetOperationParameters (
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _Out_ WSMAN_DATA *data
    )
{
	CommonData *commonData = (CommonData*) requestDetails;
	if ((flags == WSMAN_PLUGIN_PARAMS_GET_REQUESTED_LOCALE) ||
			(flags == WSMAN_PLUGIN_PARAMS_GET_REQUESTED_DATA_LOCALE))
	{
		if (!Utf8ToUtf16Le(commonData->batch, "en-us", (MI_Char16**)&data->text.buffer))
			return MI_RESULT_FAILED;
		data->text.bufferLength = 5;
		return MI_RESULT_OK;
	}
	return (MI_Uint32) MI_RESULT_FAILED;
}

/* Gets information about the host the provider is hosted in */
MI_Uint32 MI_CALL WSManPluginGetConfiguration (
  _In_   void * pluginOperationContext,
  _In_   MI_Uint32 flags,
  _Out_  WSMAN_DATA *data
)
{
    return (MI_Uint32)MI_RESULT_FAILED;
}

MI_Boolean DetachOperationFromParent(CommonData *commonData)
{
    CommonData *parent = commonData->parentData;
    CommonData **parentsChildren;

    if (parent->requestType == CommonData_Type_Command)
        parentsChildren = &((CommandData*)parent)->childNext;
    else
        parentsChildren = &((ShellData*)parent)->childNext;

    while (*parentsChildren)
    {
        if ((*parentsChildren) == commonData)
        {
            /* found it, remove it from the list */
            (*parentsChildren) = commonData->siblingData;
            return MI_TRUE;
        }
        parentsChildren = &(*parentsChildren)->siblingData;
    }

    return MI_FALSE;
}

/*
* Reports the completion of an operation by all operation entry points except for the WSMAN_PLUGIN_STARTUP and WSMAN_PLUGIN_SHUTDOWN methods.
*/
MI_Uint32 MI_CALL WSManPluginOperationComplete(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ MI_Uint32 flags,
    _In_ MI_Uint32 errorCode,
    _In_opt_ const MI_Char16 * extendedInformation
    )
{
    CommonData *commonData = (CommonData*) requestDetails;
    MI_Result miResult;

    /* Question is: which request is this? */
    switch (commonData->requestType)
    {
    case CommonData_Type_Shell:
    {
        /* TODO: Shell has completed. We should get no more calls after this */
        /* Did we already send the response back? If not we need to do something */
        /* TODO: Are there other outstanding operations? */

        ShellData *shellData = (ShellData *)commonData;
        ShellData **pointerToPatch = &shellData->shell->shellList;

        while (*pointerToPatch && (*pointerToPatch != shellData))
        {
            pointerToPatch = (ShellData **)&(*pointerToPatch)->common.siblingData;
        }
        if (*pointerToPatch)
            *pointerToPatch = (ShellData *)shellData->common.siblingData;

        /* Delete the shell object and all the memory owned by it (owned by the batch) */
        Batch_Delete(commonData->batch);
        break;
    }
    case CommonData_Type_Command:
        /* TODO: This command is complete. No more calls for this command should happen */
        /* TODO: Did we already send the response back? If not we need to do something */
        /* TODO: Are there any active child objects? */

        DetachOperationFromParent(commonData);
        Batch_Delete(commonData->batch);
        break;
    case CommonData_Type_Receive:
        /* TODO: This is the termination of the receive. No more will happen for either the command or shell, depending on which is is aimed at */
        /* We may or may not have a pending Receive protocol packet for this depending on if we have already send a command completion message or not */
        if (commonData->miRequestContext)
        {
        	MI_Char16 *commandState;
        	if (!Utf8ToUtf16Le(commonData->batch, WSMAN_COMMAND_STATE_DONE, &commandState))
        	{
        		return MI_RESULT_FAILED;
        	}
            /* We have a pending request that needs to be terminated */
            WSManPluginReceiveResult(requestDetails, WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA, NULL, NULL, commandState, errorCode);
        }

        /* Clean up the Receive data as there is nothing else going to happen */
        DetachOperationFromParent(commonData);
        Batch_Delete(commonData->batch);
        break;

    /* Send/Receive only need to post back the operation instance with the MIReturn code set */
    case CommonData_Type_Signal:
    case CommonData_Type_Send:
    {
        MI_Value miValue;
        MI_Context *miRequestContext = commonData->miRequestContext;
        MI_Instance *miOperationInstance = commonData->miOperationInstance;

        commonData->miRequestContext = NULL;
        commonData->miOperationInstance = NULL;

        /* Methods only have the return code set in the instance so set that and post back. */
        miValue.uint32 = errorCode;
        MI_Instance_SetElement(miOperationInstance,MI_T("MIReturn"),&miValue,MI_UINT32,0);
        
        miResult = MI_Context_PostInstance(miRequestContext, miOperationInstance);
        MI_Context_PostResult(miRequestContext, miResult);

        MI_Instance_Delete(miOperationInstance);

        /* Remove self from parent */
        DetachOperationFromParent(commonData);

        /* Some extra data to clean up from send request */
        if (commonData->requestType == CommonData_Type_Send)
        {
            SendData *sendData = (SendData*)commonData;
            free(sendData->inboundData.binaryData.data);

        }
        /* Delete our batch */
        Batch_Delete(commonData->batch);

        break;
    }
    }

    return MI_RESULT_OK;
}

/* Seems to be called to say the plug-in is actually done */
MI_Uint32 MI_CALL WSManPluginReportCompletion(
  _In_      void * pluginOperationContext,
  _In_      MI_Uint32 flags
  )
{
    return (MI_Uint32)MI_RESULT_OK;
}

/* Free up information inside requestDetails if possible */
MI_Uint32 MI_CALL WSManPluginFreeRequestDetails(_In_ WSMAN_PLUGIN_REQUEST *requestDetails)
{
	return MI_RESULT_OK;
}


void MI_CALL WSManPluginRegisterShutdownCallback(
    _In_ WSMAN_PLUGIN_REQUEST *requestDetails,
    _In_ WSManPluginShutdownCallback shutdownCallback,
    _In_opt_ void *shutdownContext)
{
    CommonData *commonData = (CommonData*)requestDetails;
    commonData->shutdownCallback = shutdownCallback;
    commonData->shutdownContext = shutdownContext;
}
