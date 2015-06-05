/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _CommandState_h
#define _CommandState_h

#include <MI.h>

/*
**==============================================================================
**
** CommandState [CommandState]
**
** Keys:
**
**==============================================================================
*/

typedef struct _CommandState
{
    MI_Instance __instance;
    /* CommandState properties */
    MI_ConstStringField commandId;
    MI_ConstStringField state;
    MI_ConstUint32Field exitCode;
}
CommandState;

typedef struct _CommandState_Ref
{
    CommandState* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CommandState_Ref;

typedef struct _CommandState_ConstRef
{
    MI_CONST CommandState* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CommandState_ConstRef;

typedef struct _CommandState_Array
{
    struct _CommandState** data;
    MI_Uint32 size;
}
CommandState_Array;

typedef struct _CommandState_ConstArray
{
    struct _CommandState MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CommandState_ConstArray;

typedef struct _CommandState_ArrayRef
{
    CommandState_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CommandState_ArrayRef;

typedef struct _CommandState_ConstArrayRef
{
    CommandState_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CommandState_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CommandState_rtti;

MI_INLINE MI_Result MI_CALL CommandState_Construct(
    CommandState* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &CommandState_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CommandState_Clone(
    const CommandState* self,
    CommandState** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CommandState_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CommandState_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CommandState_Destruct(CommandState* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CommandState_Delete(CommandState* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CommandState_Post(
    const CommandState* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CommandState_Set_commandId(
    CommandState* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CommandState_SetPtr_commandId(
    CommandState* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CommandState_Clear_commandId(
    CommandState* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CommandState_Set_state(
    CommandState* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CommandState_SetPtr_state(
    CommandState* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CommandState_Clear_state(
    CommandState* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CommandState_Set_exitCode(
    CommandState* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->exitCode)->value = x;
    ((MI_Uint32Field*)&self->exitCode)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CommandState_Clear_exitCode(
    CommandState* self)
{
    memset((void*)&self->exitCode, 0, sizeof(self->exitCode));
    return MI_RESULT_OK;
}


#endif /* _CommandState_h */
