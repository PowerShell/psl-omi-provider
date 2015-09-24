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
#include "EnvironmentVariable.h"
#include "Stream.h"
#include "CommandState.h"

/*
**==============================================================================
**
** Shell [Shell]
**
** Keys:
**    Name
**
**==============================================================================
*/

typedef struct _Shell
{
    MI_Instance __instance;
    /* Shell properties */
    /*KEY*/ MI_ConstStringField Name;
    MI_ConstStringField WorkingDirectory;
    EnvironmentVariable_ConstArrayRef Environment;
    MI_ConstStringField InputStreams;
    MI_ConstStringField OutputStreams;
    MI_ConstBooleanField IsCompressed;
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

MI_INLINE MI_Result MI_CALL Shell_Set_Name(
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

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Name(
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

MI_INLINE MI_Result MI_CALL Shell_Clear_Name(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Set_WorkingDirectory(
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

MI_INLINE MI_Result MI_CALL Shell_SetPtr_WorkingDirectory(
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

MI_INLINE MI_Result MI_CALL Shell_Clear_WorkingDirectory(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Set_Environment(
    Shell* self,
    const EnvironmentVariable * const * data,
    MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&arr,
        MI_INSTANCEA,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_SetPtr_Environment(
    Shell* self,
    const EnvironmentVariable * const * data,
    MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&arr,
        MI_INSTANCEA,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Clear_Environment(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Set_InputStreams(
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

MI_INLINE MI_Result MI_CALL Shell_SetPtr_InputStreams(
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

MI_INLINE MI_Result MI_CALL Shell_Clear_InputStreams(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL Shell_Set_OutputStreams(
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

MI_INLINE MI_Result MI_CALL Shell_SetPtr_OutputStreams(
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

MI_INLINE MI_Result MI_CALL Shell_Clear_OutputStreams(
    Shell* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL Shell_Set_IsCompressed(
    Shell* self,
    MI_Boolean x)
{
    ((MI_BooleanField*)&self->IsCompressed)->value = x;
    ((MI_BooleanField*)&self->IsCompressed)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Shell_Clear_IsCompressed(
    Shell* self)
{
    memset((void*)&self->IsCompressed, 0, sizeof(self->IsCompressed));
    return MI_RESULT_OK;
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
MI_ConstStringField commandId;
MI_ConstStringField streamSet;
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

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_commandId(
    Shell_Receive* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_SetPtr_commandId(
    Shell_Receive* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_commandId(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_streamSet(
    Shell_Receive* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_SetPtr_streamSet(
    Shell_Receive* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_streamSet(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_Stream(
    Shell_Receive* self,
    const Stream* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
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
        3,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_Stream(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Set_CommandState(
    Shell_Receive* self,
    const CommandState* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
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
        4,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Shell_Receive_Clear_CommandState(
    Shell_Receive* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
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
** Shell.Connect()
**
**==============================================================================
*/

typedef struct _Shell_Connect
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
MI_ConstStringField shellId;
MI_ConstStringField commandId;
MI_ConstStringField inboundConnectionInfo;
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

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_shellId(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_shellId(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_shellId(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_commandId(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_commandId(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_commandId(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Shell_Connect_Set_inboundConnectionInfo(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_SetPtr_inboundConnectionInfo(
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

MI_INLINE MI_Result MI_CALL Shell_Connect_Clear_inboundConnectionInfo(
    Shell_Connect* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
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

MI_EXTERN_C void MI_CALL Shell_Invoke_Connect(
    Shell_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_Char* methodName,
    const Shell* instanceName,
    const Shell_Connect* in);


#endif /* _Shell_h */
