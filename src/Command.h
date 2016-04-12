/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _Command_h
#define _Command_h

#include <MI.h>

/*
**==============================================================================
**
** Command [Command]
**
** Keys:
**    ShellId
**    CommandId
**
**==============================================================================
*/

typedef struct _Command
{
    MI_Instance __instance;
    /* Command properties */
    /*KEY*/ MI_ConstStringField ShellId;
    /*KEY*/ MI_ConstStringField CommandId;
}
Command;

typedef struct _Command_Ref
{
    Command* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Command_Ref;

typedef struct _Command_ConstRef
{
    MI_CONST Command* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Command_ConstRef;

typedef struct _Command_Array
{
    struct _Command** data;
    MI_Uint32 size;
}
Command_Array;

typedef struct _Command_ConstArray
{
    struct _Command MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
Command_ConstArray;

typedef struct _Command_ArrayRef
{
    Command_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Command_ArrayRef;

typedef struct _Command_ConstArrayRef
{
    Command_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Command_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl Command_rtti;

MI_INLINE MI_Result MI_CALL Command_Construct(
    Command* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &Command_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Command_Clone(
    const Command* self,
    Command** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL Command_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &Command_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL Command_Destruct(Command* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Command_Delete(Command* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Command_Post(
    const Command* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Command_Set_ShellId(
    Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Command_SetPtr_ShellId(
    Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Command_Clear_ShellId(
    Command* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL Command_Set_CommandId(
    Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Command_SetPtr_CommandId(
    Command* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Command_Clear_CommandId(
    Command* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

/*
**==============================================================================
**
** Command provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _Command_Self Command_Self;

MI_EXTERN_C void MI_CALL Command_Load(
    Command_Self** self,
    MI_Module_Self* selfModule,
    MI_Context* context);

MI_EXTERN_C void MI_CALL Command_Unload(
    Command_Self* self,
    MI_Context* context);

MI_EXTERN_C void MI_CALL Command_EnumerateInstances(
    Command_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const MI_PropertySet* propertySet,
    MI_Boolean keysOnly,
    const MI_Filter* filter);

MI_EXTERN_C void MI_CALL Command_GetInstance(
    Command_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Command* instanceName,
    const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL Command_CreateInstance(
    Command_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Command* newInstance);

MI_EXTERN_C void MI_CALL Command_ModifyInstance(
    Command_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Command* modifiedInstance,
    const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL Command_DeleteInstance(
    Command_Self* self,
    MI_Context* context,
    const MI_Char* nameSpace,
    const MI_Char* className,
    const Command* instanceName);


#endif /* _Command_h */
