/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _Shell_h
#define _Shell_h

#include <MI.h>
#include "Stream.h"
#include "DesiredStream.h"
#include "CommandState.h"

/*
**==============================================================================
**
** Shell [Shell]
**
** Keys:
**    ShellId
**
**==============================================================================
*/

typedef struct _Shell
{
    MI_Instance __instance;
    /* Shell properties */
    /*KEY*/ MI_ConstStringField ShellId;
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

typedef struct _Shell_Ref
{
    Shell* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Shell_Ref;

typedef struct _Shell_ConstRef
{
    MI_CONST Shell* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Shell_ConstRef;

typedef struct _Shell_Array
{
    struct _Shell** data;
    MI_Uint32 size;
}
Shell_Array;

typedef struct _Shell_ConstArray
{
    struct _Shell MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
Shell_ConstArray;

typedef struct _Shell_ArrayRef
{
    Shell_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Shell_ArrayRef;

typedef struct _Shell_ConstArrayRef
{
    Shell_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Shell_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl Shell_rtti;

MI_INLINE MI_Result MI_CALL Shell_Construct(
    Shell* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &Shell_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Clone(
    const Shell* self,
    Shell** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL Shell_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &Shell_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL Shell_Destruct(Shell* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Delete(Shell* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Post(
    const Shell* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ShellId(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_ShellId(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ShellId(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Set_Name(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Name(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_Name(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ResourceUri(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_ResourceUri(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ResourceUri(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Set_Owner(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Owner(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_Owner(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ClientIP(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_ClientIP(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ClientIP(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ProcessId(
    Shell* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->ProcessId)->value = x;
    ((MI_Uint32Field*)&self->ProcessId)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ProcessId(
    Shell* self)
{
    memset((void*)&self->ProcessId, 0, sizeof(self->ProcessId));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Set_IdleTimeout(
    Shell* self,
    MI_Datetime x)
{
    ((MI_DatetimeField*)&self->IdleTimeout)->value = x;
    ((MI_DatetimeField*)&self->IdleTimeout)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_IdleTimeout(
    Shell* self)
{
    memset((void*)&self->IdleTimeout, 0, sizeof(self->IdleTimeout));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Set_InputStreams(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        7,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_InputStreams(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        7,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_InputStreams(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        7);
}

MI_INLINE MI_Result MI_CALL Shell_Set_OutputStreams(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_OutputStreams(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_OutputStreams(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL Shell_Set_MaxIdleTimeout(
    Shell* self,
    MI_Datetime x)
{
    ((MI_DatetimeField*)&self->MaxIdleTimeout)->value = x;
    ((MI_DatetimeField*)&self->MaxIdleTimeout)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_MaxIdleTimeout(
    Shell* self)
{
    memset((void*)&self->MaxIdleTimeout, 0, sizeof(self->MaxIdleTimeout));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Set_Locale(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Locale(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_Locale(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        10);
}

MI_INLINE MI_Result MI_CALL Shell_Set_DataLocale(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_DataLocale(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_DataLocale(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        11);
}

MI_INLINE MI_Result MI_CALL Shell_Set_CompressionMode(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        12,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_CompressionMode(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        12,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_CompressionMode(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        12);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ProfileLoaded(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        13,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_ProfileLoaded(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        13,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ProfileLoaded(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        13);
}

MI_INLINE MI_Result MI_CALL Shell_Set_Encoding(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        14,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Encoding(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        14,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_Encoding(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        14);
}

MI_INLINE MI_Result MI_CALL Shell_Set_BufferMode(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_BufferMode(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_BufferMode(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        15);
}

MI_INLINE MI_Result MI_CALL Shell_Set_State(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        16,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_State(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        16,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_State(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        16);
}

MI_INLINE MI_Result MI_CALL Shell_Set_ShellRunTime(
    Shell* self,
    MI_Datetime x)
{
    ((MI_DatetimeField*)&self->ShellRunTime)->value = x;
    ((MI_DatetimeField*)&self->ShellRunTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ShellRunTime(
    Shell* self)
{
    memset((void*)&self->ShellRunTime, 0, sizeof(self->ShellRunTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Set_ShellInactivity(
    Shell* self,
    MI_Datetime x)
{
    ((MI_DatetimeField*)&self->ShellInactivity)->value = x;
    ((MI_DatetimeField*)&self->ShellInactivity)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_ShellInactivity(
    Shell* self)
{
    memset((void*)&self->ShellInactivity, 0, sizeof(self->ShellInactivity));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Set_CreationXml(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        19,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_CreationXml(
    Shell* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        19,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_CreationXml(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        19);
}

/*
**==============================================================================
**
** Shell.Command()
**
**==============================================================================
*/

typedef struct _Shell_Command
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
MI_ConstStringField command;
MI_ConstStringAField arguments;
    /*OUT*/ MI_ConstStringField CommandId;
}
Shell_Command;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Command_rtti;

MI_INLINE MI_Result MI_CALL Shell_Command_Construct(
    Shell_Command* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Command_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Clone(
    const Shell_Command* self,
    Shell_Command** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Destruct(
    Shell_Command* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Delete(
    Shell_Command* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Post(
    const Shell_Command* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Set_MIReturn(
    Shell_Command* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Command_Clear_MIReturn(
    Shell_Command* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Command_Set_command(
    Shell_Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Command_SetPtr_command(
    Shell_Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Clear_command(
    Shell_Command* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Set_arguments(
    Shell_Command* self,
    const MI_Char** data,
    MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&arr,
        MI_STRINGA,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Command_SetPtr_arguments(
    Shell_Command* self,
    const MI_Char** data,
    MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&arr,
        MI_STRINGA,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Clear_arguments(
    Shell_Command* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Set_CommandId(
    Shell_Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Command_SetPtr_CommandId(
    Shell_Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Command_Clear_CommandId(
    Shell_Command* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

/*
**==============================================================================
**
** Shell.Send()
**
**==============================================================================
*/

typedef struct _Shell_Send
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
Stream_ConstRef streamData;
}
Shell_Send;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Send_rtti;

MI_INLINE MI_Result MI_CALL Shell_Send_Construct(
    Shell_Send* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Send_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Clone(
    const Shell_Send* self,
    Shell_Send** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Destruct(
    Shell_Send* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Delete(
    Shell_Send* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Post(
    const Shell_Send* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Set_MIReturn(
    Shell_Send* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Send_Clear_MIReturn(
    Shell_Send* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Send_Set_streamData(
    Shell_Send* self,
    const Stream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Send_SetPtr_streamData(
    Shell_Send* self,
    const Stream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Send_Clear_streamData(
    Shell_Send* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

/*
**==============================================================================
**
** Shell.Receive()
**
**==============================================================================
*/

typedef struct _Shell_Receive
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
DesiredStream_ConstRef DesiredStream;
    /*OUT*/ Stream_ConstRef Stream;
    /*OUT*/ CommandState_ConstRef CommandState;
}
Shell_Receive;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Receive_rtti;

MI_INLINE MI_Result MI_CALL Shell_Receive_Construct(
    Shell_Receive* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Receive_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clone(
    const Shell_Receive* self,
    Shell_Receive** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Destruct(
    Shell_Receive* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Delete(
    Shell_Receive* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Post(
    const Shell_Receive* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_MIReturn(
    Shell_Receive* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_MIReturn(
    Shell_Receive* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_DesiredStream(
    Shell_Receive* self,
    const DesiredStream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_SetPtr_DesiredStream(
    Shell_Receive* self,
    const DesiredStream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_DesiredStream(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_Stream(
    Shell_Receive* self,
    const Stream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_SetPtr_Stream(
    Shell_Receive* self,
    const Stream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_Stream(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_CommandState(
    Shell_Receive* self,
    const CommandState* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_SetPtr_CommandState(
    Shell_Receive* self,
    const CommandState* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_CommandState(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

/*
**==============================================================================
**
** Shell.Signal()
**
**==============================================================================
*/

typedef struct _Shell_Signal
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
MI_ConstStringField commandId;
MI_ConstStringField code;
}
Shell_Signal;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Signal_rtti;

MI_INLINE MI_Result MI_CALL Shell_Signal_Construct(
    Shell_Signal* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Signal_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Clone(
    const Shell_Signal* self,
    Shell_Signal** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Destruct(
    Shell_Signal* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Delete(
    Shell_Signal* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Post(
    const Shell_Signal* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Set_MIReturn(
    Shell_Signal* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Clear_MIReturn(
    Shell_Signal* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Set_commandId(
    Shell_Signal* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_SetPtr_commandId(
    Shell_Signal* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Clear_commandId(
    Shell_Signal* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Set_code(
    Shell_Signal* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_SetPtr_code(
    Shell_Signal* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Signal_Clear_code(
    Shell_Signal* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

/*
**==============================================================================
**
** Shell.Disconnect()
**
**==============================================================================
*/

typedef struct _Shell_Disconnect
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
MI_ConstDatetimeField IdleTimeOut;
MI_ConstStringField BufferMode;
}
Shell_Disconnect;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Disconnect_rtti;

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Construct(
    Shell_Disconnect* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Disconnect_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Clone(
    const Shell_Disconnect* self,
    Shell_Disconnect** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Destruct(
    Shell_Disconnect* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Delete(
    Shell_Disconnect* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Post(
    const Shell_Disconnect* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Set_MIReturn(
    Shell_Disconnect* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Clear_MIReturn(
    Shell_Disconnect* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Set_IdleTimeOut(
    Shell_Disconnect* self,
    MI_Datetime x)
{
    ((MI_DatetimeField*)&self->IdleTimeOut)->value = x;
    ((MI_DatetimeField*)&self->IdleTimeOut)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Clear_IdleTimeOut(
    Shell_Disconnect* self)
{
    memset((void*)&self->IdleTimeOut, 0, sizeof(self->IdleTimeOut));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Set_BufferMode(
    Shell_Disconnect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_SetPtr_BufferMode(
    Shell_Disconnect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Disconnect_Clear_BufferMode(
    Shell_Disconnect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

/*
**==============================================================================
**
** Shell.Reconnect()
**
**==============================================================================
*/

typedef struct _Shell_Reconnect
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
}
Shell_Reconnect;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Reconnect_rtti;

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Construct(
    Shell_Reconnect* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Reconnect_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Clone(
    const Shell_Reconnect* self,
    Shell_Reconnect** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Destruct(
    Shell_Reconnect* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Delete(
    Shell_Reconnect* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Post(
    const Shell_Reconnect* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Set_MIReturn(
    Shell_Reconnect* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Reconnect_Clear_MIReturn(
    Shell_Reconnect* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** Shell.Connect()
**
**==============================================================================
*/

typedef struct _Shell_Connect
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
MI_ConstStringField BufferMode;
MI_ConstStringField connectXml;
    /*OUT*/ MI_ConstStringField InputStreams;
    /*OUT*/ MI_ConstStringField OutputStreams;
    /*OUT*/ MI_ConstStringField connectResponseXml;
}
Shell_Connect;

MI_EXTERN_C MI_CONST MI_MethodDecl Shell_Connect_rtti;

MI_INLINE MI_Result MI_CALL Shell_Connect_Construct(
    Shell_Connect* self,
    MI_Context* context)
{
    return MI_ConstructParameters(context, &Shell_Connect_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clone(
    const Shell_Connect* self,
    Shell_Connect** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Destruct(
    Shell_Connect* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Delete(
    Shell_Connect* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Post(
    const Shell_Connect* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_MIReturn(
    Shell_Connect* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_MIReturn(
    Shell_Connect* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_BufferMode(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_BufferMode(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_BufferMode(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_connectXml(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_connectXml(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_connectXml(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_InputStreams(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_InputStreams(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_InputStreams(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_OutputStreams(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_OutputStreams(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_OutputStreams(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_connectResponseXml(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_connectResponseXml(
    Shell_Connect* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_connectResponseXml(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

/*
**==============================================================================
**
** Shell provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _Shell_Self Shell_Self;

MI_EXTERN_C void MI_CALL Shell_Load(
    Shell_Self** self,
    MI_Module_Self* selfModule,
    MI_Context* context);

MI_EXTERN_C void MI_CALL Shell_Unload(
    Shell_Self* self,
    MI_Context* context);

MI_EXTERN_C void MI_CALL Shell_EnumerateInstances(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_PropertySet* propertySet,
    MI_Boolean keysOnly,
    const MI_Filter* filter);

MI_EXTERN_C void MI_CALL Shell_GetInstance(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Shell* instanceName,
    const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL Shell_CreateInstance(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Shell* newInstance);

MI_EXTERN_C void MI_CALL Shell_ModifyInstance(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Shell* modifiedInstance,
    const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL Shell_DeleteInstance(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Shell* instanceName);

MI_EXTERN_C void MI_CALL Shell_Invoke_Command(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Command* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Send(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Send* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Receive(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Receive* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Signal(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Signal* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Disconnect(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Disconnect* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Reconnect(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Reconnect* in);

MI_EXTERN_C void MI_CALL Shell_Invoke_Connect(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Connect* in);


#endif /* _Shell_h */
