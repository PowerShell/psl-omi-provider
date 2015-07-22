/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _Stream_h
#define _Stream_h

#include <MI.h>

/*
**==============================================================================
**
** Stream [Stream]
**
** Keys:
**
**==============================================================================
*/

typedef struct _Stream
{
    MI_Instance __instance;
    /* Stream properties */
    MI_ConstStringField commandId;
    MI_ConstStringField streamName;
    MI_ConstStringField data;
    MI_ConstUint32Field dataLength;
    MI_ConstBooleanField endOfStream;
}
Stream;

typedef struct _Stream_Ref
{
    Stream* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Stream_Ref;

typedef struct _Stream_ConstRef
{
    MI_CONST Stream* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Stream_ConstRef;

typedef struct _Stream_Array
{
    struct _Stream** data;
    MI_Uint32 size;
}
Stream_Array;

typedef struct _Stream_ConstArray
{
    struct _Stream MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
Stream_ConstArray;

typedef struct _Stream_ArrayRef
{
    Stream_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Stream_ArrayRef;

typedef struct _Stream_ConstArrayRef
{
    Stream_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
Stream_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl Stream_rtti;

MI_INLINE MI_Result MI_CALL Stream_Construct(
    Stream* self,
    MI_Context* context)
{
    return MI_ConstructInstance(context, &Stream_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL Stream_Clone(
    const Stream* self,
    Stream** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL Stream_IsA(
    const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &Stream_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL Stream_Destruct(Stream* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Stream_Delete(Stream* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL Stream_Post(
    const Stream* self,
    MI_Context* context)
{
    return MI_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL Stream_Set_commandId(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Stream_SetPtr_commandId(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Stream_Clear_commandId(
    Stream* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL Stream_Set_streamName(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Stream_SetPtr_streamName(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Stream_Clear_streamName(
    Stream* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL Stream_Set_data(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL Stream_SetPtr_data(
    Stream* self,
    const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL Stream_Clear_data(
    Stream* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL Stream_Set_dataLength(
    Stream* self,
    MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->dataLength)->value = x;
    ((MI_Uint32Field*)&self->dataLength)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Stream_Clear_dataLength(
    Stream* self)
{
    memset((void*)&self->dataLength, 0, sizeof(self->dataLength));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Stream_Set_endOfStream(
    Stream* self,
    MI_Boolean x)
{
    ((MI_BooleanField*)&self->endOfStream)->value = x;
    ((MI_BooleanField*)&self->endOfStream)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL Stream_Clear_endOfStream(
    Stream* self)
{
    memset((void*)&self->endOfStream, 0, sizeof(self->endOfStream));
    return MI_RESULT_OK;
}


#endif /* _Stream_h */
