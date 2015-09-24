/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _EnvironmentVariable_h
#define _EnvironmentVariable_h

#include <MI.h>

/*
**==============================================================================
**
** EnvironmentVariable [EnvironmentVariable]
**
** Keys:
**
**==============================================================================
*/

typedef struct _EnvironmentVariable
{
    MI_Instance __instance;
    /* EnvironmentVariable properties */
    MI_ConstStringField Name;
    MI_ConstStringField Value;
}
EnvironmentVariable;

typedef struct _EnvironmentVariable_Ref
{
    EnvironmentVariable* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
EnvironmentVariable_Ref;

typedef struct _EnvironmentVariable_ConstRef
{
    MI_CONST EnvironmentVariable* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
EnvironmentVariable_ConstRef;

typedef struct _EnvironmentVariable_Array
{
    struct _EnvironmentVariable** data;
    MI_Uint32 size;
}
EnvironmentVariable_Array;

typedef struct _EnvironmentVariable_ConstArray
{
    struct _EnvironmentVariable MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
EnvironmentVariable_ConstArray;

typedef struct _EnvironmentVariable_ArrayRef
{
    EnvironmentVariable_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
EnvironmentVariable_ArrayRef;

typedef struct _EnvironmentVariable_ConstArrayRef
{
    EnvironmentVariable_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
EnvironmentVariable_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl EnvironmentVariable_rtti;

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Construct(
    EnvironmentVariable* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &EnvironmentVariable_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Clone(
    const EnvironmentVariable* self,
    EnvironmentVariable** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL EnvironmentVariable_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &EnvironmentVariable_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Destruct(EnvironmentVariable* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Delete(EnvironmentVariable* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Post(
    const EnvironmentVariable* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Set_Name(
    EnvironmentVariable* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_SetPtr_Name(
    EnvironmentVariable* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Clear_Name(
    EnvironmentVariable* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Set_Value(
    EnvironmentVariable* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_SetPtr_Value(
    EnvironmentVariable* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL EnvironmentVariable_Clear_Value(
    EnvironmentVariable* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}


#endif /* _EnvironmentVariable_h */
