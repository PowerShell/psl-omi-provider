/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _DesiredStream_h
#define _DesiredStream_h

#include <MI.h>

/*
**==============================================================================
**
** DesiredStream [DesiredStream]
**
** Keys:
**
**==============================================================================
*/

typedef struct _DesiredStream
{
    MI_Instance __instance;
    /* DesiredStream properties */
    MI_ConstStringField commandId;
    MI_ConstStringField streamName;
}
DesiredStream;

typedef struct _DesiredStream_Ref
{
    DesiredStream* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
DesiredStream_Ref;

typedef struct _DesiredStream_ConstRef
{
    MI_CONST DesiredStream* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
DesiredStream_ConstRef;

typedef struct _DesiredStream_Array
{
    struct _DesiredStream** data;
    MI_Uint32 size;
}
DesiredStream_Array;

typedef struct _DesiredStream_ConstArray
{
    struct _DesiredStream MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
DesiredStream_ConstArray;

typedef struct _DesiredStream_ArrayRef
{
    DesiredStream_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
DesiredStream_ArrayRef;

typedef struct _DesiredStream_ConstArrayRef
{
    DesiredStream_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
DesiredStream_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl DesiredStream_rtti;

MI_INLINE MI_Result MI_CALL DesiredStream_Construct(
    DesiredStream* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &DesiredStream_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Clone(
    const DesiredStream* self,
    DesiredStream** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL DesiredStream_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &DesiredStream_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL DesiredStream_Destruct(DesiredStream* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Delete(DesiredStream* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Post(
    const DesiredStream* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Set_commandId(
    DesiredStream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL DesiredStream_SetPtr_commandId(
    DesiredStream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Clear_commandId(
    DesiredStream* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Set_streamName(
    DesiredStream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL DesiredStream_SetPtr_streamName(
    DesiredStream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL DesiredStream_Clear_streamName(
    DesiredStream* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}


#endif /* _DesiredStream_h */
