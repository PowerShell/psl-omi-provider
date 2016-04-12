/* @migen@ */
/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#include <ctype.h>
#include <MI.h>
#include "Shell.h"
#include "Command.h"

/*
**==============================================================================
**
** Schema Declaration
**
**==============================================================================
*/

extern MI_SchemaDecl schemaDecl;

/*
**==============================================================================
**
** Qualifier declarations
**
**==============================================================================
*/

/*
**==============================================================================
**
** Stream
**
**==============================================================================
*/

/* property Stream.commandId */
static MI_CONST MI_PropertyDecl Stream_commandId_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00636409, /* code */
    MI_T("commandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Stream, commandId), /* offset */
    MI_T("Stream"), /* origin */
    MI_T("Stream"), /* propagator */
    NULL,
};

/* property Stream.streamName */
static MI_CONST MI_PropertyDecl Stream_streamName_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0073650A, /* code */
    MI_T("streamName"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Stream, streamName), /* offset */
    MI_T("Stream"), /* origin */
    MI_T("Stream"), /* propagator */
    NULL,
};

/* property Stream.data */
static MI_CONST MI_PropertyDecl Stream_data_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00646104, /* code */
    MI_T("data"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Stream, data), /* offset */
    MI_T("Stream"), /* origin */
    MI_T("Stream"), /* propagator */
    NULL,
};

/* property Stream.dataLength */
static MI_CONST MI_PropertyDecl Stream_dataLength_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0064680A, /* code */
    MI_T("dataLength"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Stream, dataLength), /* offset */
    MI_T("Stream"), /* origin */
    MI_T("Stream"), /* propagator */
    NULL,
};

/* property Stream.endOfStream */
static MI_CONST MI_PropertyDecl Stream_endOfStream_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00656D0B, /* code */
    MI_T("endOfStream"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_BOOLEAN, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Stream, endOfStream), /* offset */
    MI_T("Stream"), /* origin */
    MI_T("Stream"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST Stream_props[] =
{
    &Stream_commandId_prop,
    &Stream_streamName_prop,
    &Stream_data_prop,
    &Stream_dataLength_prop,
    &Stream_endOfStream_prop,
};

/* class Stream */
MI_CONST MI_ClassDecl Stream_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x00736D06, /* code */
    MI_T("Stream"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Stream_props, /* properties */
    MI_COUNT(Stream_props), /* numProperties */
    sizeof(Stream), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL, /* owningClass */
};

/*
**==============================================================================
**
** DesiredStream
**
**==============================================================================
*/

/* property DesiredStream.commandId */
static MI_CONST MI_PropertyDecl DesiredStream_commandId_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00636409, /* code */
    MI_T("commandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(DesiredStream, commandId), /* offset */
    MI_T("DesiredStream"), /* origin */
    MI_T("DesiredStream"), /* propagator */
    NULL,
};

/* property DesiredStream.streamName */
static MI_CONST MI_PropertyDecl DesiredStream_streamName_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0073650A, /* code */
    MI_T("streamName"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(DesiredStream, streamName), /* offset */
    MI_T("DesiredStream"), /* origin */
    MI_T("DesiredStream"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST DesiredStream_props[] =
{
    &DesiredStream_commandId_prop,
    &DesiredStream_streamName_prop,
};

/* class DesiredStream */
MI_CONST MI_ClassDecl DesiredStream_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x00646D0D, /* code */
    MI_T("DesiredStream"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    DesiredStream_props, /* properties */
    MI_COUNT(DesiredStream_props), /* numProperties */
    sizeof(DesiredStream), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL, /* owningClass */
};

/*
**==============================================================================
**
** CommandState
**
**==============================================================================
*/

/* property CommandState.commandId */
static MI_CONST MI_PropertyDecl CommandState_commandId_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00636409, /* code */
    MI_T("commandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CommandState, commandId), /* offset */
    MI_T("CommandState"), /* origin */
    MI_T("CommandState"), /* propagator */
    NULL,
};

/* property CommandState.state */
static MI_CONST MI_PropertyDecl CommandState_state_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00736505, /* code */
    MI_T("state"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CommandState, state), /* offset */
    MI_T("CommandState"), /* origin */
    MI_T("CommandState"), /* propagator */
    NULL,
};

/* property CommandState.exitCode */
static MI_CONST MI_PropertyDecl CommandState_exitCode_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00656508, /* code */
    MI_T("exitCode"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CommandState, exitCode), /* offset */
    MI_T("CommandState"), /* origin */
    MI_T("CommandState"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CommandState_props[] =
{
    &CommandState_commandId_prop,
    &CommandState_state_prop,
    &CommandState_exitCode_prop,
};

/* class CommandState */
MI_CONST MI_ClassDecl CommandState_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x0063650C, /* code */
    MI_T("CommandState"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    CommandState_props, /* properties */
    MI_COUNT(CommandState_props), /* numProperties */
    sizeof(CommandState), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL, /* owningClass */
};

/*
**==============================================================================
**
** Shell
**
**==============================================================================
*/

/* property Shell.ShellId */
static MI_CONST MI_PropertyDecl Shell_ShellId_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY, /* flags */
    0x00736407, /* code */
    MI_T("ShellId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ShellId), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.Name */
static MI_CONST MI_PropertyDecl Shell_Name_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006E6504, /* code */
    MI_T("Name"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, Name), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ResourceUri */
static MI_CONST MI_PropertyDecl Shell_ResourceUri_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0072690B, /* code */
    MI_T("ResourceUri"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ResourceUri), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.Owner */
static MI_CONST MI_PropertyDecl Shell_Owner_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006F7205, /* code */
    MI_T("Owner"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, Owner), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ClientIP */
static MI_CONST MI_PropertyDecl Shell_ClientIP_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00637008, /* code */
    MI_T("ClientIP"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ClientIP), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ProcessId */
static MI_CONST MI_PropertyDecl Shell_ProcessId_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00706409, /* code */
    MI_T("ProcessId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ProcessId), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.IdleTimeout */
static MI_CONST MI_PropertyDecl Shell_IdleTimeout_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0069740B, /* code */
    MI_T("IdleTimeout"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, IdleTimeout), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.InputStreams */
static MI_CONST MI_PropertyDecl Shell_InputStreams_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0069730C, /* code */
    MI_T("InputStreams"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, InputStreams), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.OutputStreams */
static MI_CONST MI_PropertyDecl Shell_OutputStreams_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006F730D, /* code */
    MI_T("OutputStreams"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, OutputStreams), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.MaxIdleTimeout */
static MI_CONST MI_PropertyDecl Shell_MaxIdleTimeout_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006D740E, /* code */
    MI_T("MaxIdleTimeout"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, MaxIdleTimeout), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.Locale */
static MI_CONST MI_PropertyDecl Shell_Locale_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006C6506, /* code */
    MI_T("Locale"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, Locale), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.DataLocale */
static MI_CONST MI_PropertyDecl Shell_DataLocale_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0064650A, /* code */
    MI_T("DataLocale"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, DataLocale), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.CompressionMode */
static MI_CONST MI_PropertyDecl Shell_CompressionMode_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0063650F, /* code */
    MI_T("CompressionMode"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, CompressionMode), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ProfileLoaded */
static MI_CONST MI_PropertyDecl Shell_ProfileLoaded_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0070640D, /* code */
    MI_T("ProfileLoaded"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ProfileLoaded), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.Encoding */
static MI_CONST MI_PropertyDecl Shell_Encoding_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00656708, /* code */
    MI_T("Encoding"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, Encoding), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.BufferMode */
static MI_CONST MI_PropertyDecl Shell_BufferMode_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0062650A, /* code */
    MI_T("BufferMode"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, BufferMode), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.State */
static MI_CONST MI_PropertyDecl Shell_State_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00736505, /* code */
    MI_T("State"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, State), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ShellRunTime */
static MI_CONST MI_PropertyDecl Shell_ShellRunTime_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0073650C, /* code */
    MI_T("ShellRunTime"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ShellRunTime), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.ShellInactivity */
static MI_CONST MI_PropertyDecl Shell_ShellInactivity_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0073790F, /* code */
    MI_T("ShellInactivity"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, ShellInactivity), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

/* property Shell.CreationXml */
static MI_CONST MI_PropertyDecl Shell_CreationXml_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00636C0B, /* code */
    MI_T("CreationXml"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell, CreationXml), /* offset */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST Shell_props[] =
{
    &Shell_ShellId_prop,
    &Shell_Name_prop,
    &Shell_ResourceUri_prop,
    &Shell_Owner_prop,
    &Shell_ClientIP_prop,
    &Shell_ProcessId_prop,
    &Shell_IdleTimeout_prop,
    &Shell_InputStreams_prop,
    &Shell_OutputStreams_prop,
    &Shell_MaxIdleTimeout_prop,
    &Shell_Locale_prop,
    &Shell_DataLocale_prop,
    &Shell_CompressionMode_prop,
    &Shell_ProfileLoaded_prop,
    &Shell_Encoding_prop,
    &Shell_BufferMode_prop,
    &Shell_State_prop,
    &Shell_ShellRunTime_prop,
    &Shell_ShellInactivity_prop,
    &Shell_CreationXml_prop,
};

/* parameter Shell.Command(): command */
static MI_CONST MI_ParameterDecl Shell_Command_command_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00636407, /* code */
    MI_T("command"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Command, command), /* offset */
};

/* parameter Shell.Command(): arguments */
static MI_CONST MI_ParameterDecl Shell_Command_arguments_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00617309, /* code */
    MI_T("arguments"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRINGA, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Command, arguments), /* offset */
};

/* parameter Shell.Command(): CommandId */
static MI_CONST MI_ParameterDecl Shell_Command_CommandId_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x00636409, /* code */
    MI_T("CommandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Command, CommandId), /* offset */
};

/* parameter Shell.Command(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Command_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Command, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Command_params[] =
{
    &Shell_Command_MIReturn_param,
    &Shell_Command_command_param,
    &Shell_Command_arguments_param,
    &Shell_Command_CommandId_param,
};

/* method Shell.Command() */
MI_CONST MI_MethodDecl Shell_Command_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00636407, /* code */
    MI_T("Command"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Command_params, /* parameters */
    MI_COUNT(Shell_Command_params), /* numParameters */
    sizeof(Shell_Command), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Command, /* method */
};

static MI_CONST MI_Char* Shell_Send_streamData_EmbeddedInstance_qual_value = MI_T("Stream");

static MI_CONST MI_Qualifier Shell_Send_streamData_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    0,
    &Shell_Send_streamData_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST Shell_Send_streamData_quals[] =
{
    &Shell_Send_streamData_EmbeddedInstance_qual,
};

/* parameter Shell.Send(): streamData */
static MI_CONST MI_ParameterDecl Shell_Send_streamData_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x0073610A, /* code */
    MI_T("streamData"), /* name */
    Shell_Send_streamData_quals, /* qualifiers */
    MI_COUNT(Shell_Send_streamData_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    MI_T("Stream"), /* className */
    0, /* subscript */
    offsetof(Shell_Send, streamData), /* offset */
};

/* parameter Shell.Send(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Send_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Send, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Send_params[] =
{
    &Shell_Send_MIReturn_param,
    &Shell_Send_streamData_param,
};

/* method Shell.Send() */
MI_CONST MI_MethodDecl Shell_Send_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00736404, /* code */
    MI_T("Send"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Send_params, /* parameters */
    MI_COUNT(Shell_Send_params), /* numParameters */
    sizeof(Shell_Send), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Send, /* method */
};

static MI_CONST MI_Char* Shell_Receive_DesiredStream_EmbeddedInstance_qual_value = MI_T("DesiredStream");

static MI_CONST MI_Qualifier Shell_Receive_DesiredStream_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    0,
    &Shell_Receive_DesiredStream_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST Shell_Receive_DesiredStream_quals[] =
{
    &Shell_Receive_DesiredStream_EmbeddedInstance_qual,
};

/* parameter Shell.Receive(): DesiredStream */
static MI_CONST MI_ParameterDecl Shell_Receive_DesiredStream_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00646D0D, /* code */
    MI_T("DesiredStream"), /* name */
    Shell_Receive_DesiredStream_quals, /* qualifiers */
    MI_COUNT(Shell_Receive_DesiredStream_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    MI_T("DesiredStream"), /* className */
    0, /* subscript */
    offsetof(Shell_Receive, DesiredStream), /* offset */
};

static MI_CONST MI_Char* Shell_Receive_Stream_EmbeddedInstance_qual_value = MI_T("Stream");

static MI_CONST MI_Qualifier Shell_Receive_Stream_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    0,
    &Shell_Receive_Stream_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST Shell_Receive_Stream_quals[] =
{
    &Shell_Receive_Stream_EmbeddedInstance_qual,
};

/* parameter Shell.Receive(): Stream */
static MI_CONST MI_ParameterDecl Shell_Receive_Stream_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x00736D06, /* code */
    MI_T("Stream"), /* name */
    Shell_Receive_Stream_quals, /* qualifiers */
    MI_COUNT(Shell_Receive_Stream_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    MI_T("Stream"), /* className */
    0, /* subscript */
    offsetof(Shell_Receive, Stream), /* offset */
};

static MI_CONST MI_Char* Shell_Receive_CommandState_EmbeddedInstance_qual_value = MI_T("CommandState");

static MI_CONST MI_Qualifier Shell_Receive_CommandState_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    0,
    &Shell_Receive_CommandState_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST Shell_Receive_CommandState_quals[] =
{
    &Shell_Receive_CommandState_EmbeddedInstance_qual,
};

/* parameter Shell.Receive(): CommandState */
static MI_CONST MI_ParameterDecl Shell_Receive_CommandState_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x0063650C, /* code */
    MI_T("CommandState"), /* name */
    Shell_Receive_CommandState_quals, /* qualifiers */
    MI_COUNT(Shell_Receive_CommandState_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    MI_T("CommandState"), /* className */
    0, /* subscript */
    offsetof(Shell_Receive, CommandState), /* offset */
};

/* parameter Shell.Receive(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Receive_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Receive, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Receive_params[] =
{
    &Shell_Receive_MIReturn_param,
    &Shell_Receive_DesiredStream_param,
    &Shell_Receive_Stream_param,
    &Shell_Receive_CommandState_param,
};

/* method Shell.Receive() */
MI_CONST MI_MethodDecl Shell_Receive_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00726507, /* code */
    MI_T("Receive"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Receive_params, /* parameters */
    MI_COUNT(Shell_Receive_params), /* numParameters */
    sizeof(Shell_Receive), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Receive, /* method */
};

/* parameter Shell.Signal(): commandId */
static MI_CONST MI_ParameterDecl Shell_Signal_commandId_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00636409, /* code */
    MI_T("commandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Signal, commandId), /* offset */
};

/* parameter Shell.Signal(): code */
static MI_CONST MI_ParameterDecl Shell_Signal_code_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00636504, /* code */
    MI_T("code"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Signal, code), /* offset */
};

/* parameter Shell.Signal(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Signal_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Signal, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Signal_params[] =
{
    &Shell_Signal_MIReturn_param,
    &Shell_Signal_commandId_param,
    &Shell_Signal_code_param,
};

/* method Shell.Signal() */
MI_CONST MI_MethodDecl Shell_Signal_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00736C06, /* code */
    MI_T("Signal"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Signal_params, /* parameters */
    MI_COUNT(Shell_Signal_params), /* numParameters */
    sizeof(Shell_Signal), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Signal, /* method */
};

/* parameter Shell.Disconnect(): IdleTimeOut */
static MI_CONST MI_ParameterDecl Shell_Disconnect_IdleTimeOut_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x0069740B, /* code */
    MI_T("IdleTimeOut"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Disconnect, IdleTimeOut), /* offset */
};

/* parameter Shell.Disconnect(): BufferMode */
static MI_CONST MI_ParameterDecl Shell_Disconnect_BufferMode_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x0062650A, /* code */
    MI_T("BufferMode"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Disconnect, BufferMode), /* offset */
};

/* parameter Shell.Disconnect(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Disconnect_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Disconnect, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Disconnect_params[] =
{
    &Shell_Disconnect_MIReturn_param,
    &Shell_Disconnect_IdleTimeOut_param,
    &Shell_Disconnect_BufferMode_param,
};

/* method Shell.Disconnect() */
MI_CONST MI_MethodDecl Shell_Disconnect_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x0064740A, /* code */
    MI_T("Disconnect"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Disconnect_params, /* parameters */
    MI_COUNT(Shell_Disconnect_params), /* numParameters */
    sizeof(Shell_Disconnect), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Disconnect, /* method */
};

/* parameter Shell.Reconnect(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Reconnect_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Reconnect, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Reconnect_params[] =
{
    &Shell_Reconnect_MIReturn_param,
};

/* method Shell.Reconnect() */
MI_CONST MI_MethodDecl Shell_Reconnect_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00727409, /* code */
    MI_T("Reconnect"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Reconnect_params, /* parameters */
    MI_COUNT(Shell_Reconnect_params), /* numParameters */
    sizeof(Shell_Reconnect), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Reconnect, /* method */
};

/* parameter Shell.Connect(): BufferMode */
static MI_CONST MI_ParameterDecl Shell_Connect_BufferMode_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x0062650A, /* code */
    MI_T("BufferMode"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, BufferMode), /* offset */
};

/* parameter Shell.Connect(): connectXml */
static MI_CONST MI_ParameterDecl Shell_Connect_connectXml_param =
{
    MI_FLAG_PARAMETER, /* flags */
    0x00636C0A, /* code */
    MI_T("connectXml"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, connectXml), /* offset */
};

/* parameter Shell.Connect(): InputStreams */
static MI_CONST MI_ParameterDecl Shell_Connect_InputStreams_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x0069730C, /* code */
    MI_T("InputStreams"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, InputStreams), /* offset */
};

/* parameter Shell.Connect(): OutputStreams */
static MI_CONST MI_ParameterDecl Shell_Connect_OutputStreams_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006F730D, /* code */
    MI_T("OutputStreams"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, OutputStreams), /* offset */
};

/* parameter Shell.Connect(): connectResponseXml */
static MI_CONST MI_ParameterDecl Shell_Connect_connectResponseXml_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x00636C12, /* code */
    MI_T("connectResponseXml"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, connectResponseXml), /* offset */
};

/* parameter Shell.Connect(): MIReturn */
static MI_CONST MI_ParameterDecl Shell_Connect_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Shell_Connect, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST Shell_Connect_params[] =
{
    &Shell_Connect_MIReturn_param,
    &Shell_Connect_BufferMode_param,
    &Shell_Connect_connectXml_param,
    &Shell_Connect_InputStreams_param,
    &Shell_Connect_OutputStreams_param,
    &Shell_Connect_connectResponseXml_param,
};

/* method Shell.Connect() */
MI_CONST MI_MethodDecl Shell_Connect_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00637407, /* code */
    MI_T("Connect"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_Connect_params, /* parameters */
    MI_COUNT(Shell_Connect_params), /* numParameters */
    sizeof(Shell_Connect), /* size */
    MI_UINT32, /* returnType */
    MI_T("Shell"), /* origin */
    MI_T("Shell"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)Shell_Invoke_Connect, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST Shell_meths[] =
{
    &Shell_Command_rtti,
    &Shell_Send_rtti,
    &Shell_Receive_rtti,
    &Shell_Signal_rtti,
    &Shell_Disconnect_rtti,
    &Shell_Reconnect_rtti,
    &Shell_Connect_rtti,
};

static MI_CONST MI_ProviderFT Shell_funcs =
{
  (MI_ProviderFT_Load)Shell_Load,
  (MI_ProviderFT_Unload)Shell_Unload,
  (MI_ProviderFT_GetInstance)Shell_GetInstance,
  (MI_ProviderFT_EnumerateInstances)Shell_EnumerateInstances,
  (MI_ProviderFT_CreateInstance)Shell_CreateInstance,
  (MI_ProviderFT_ModifyInstance)Shell_ModifyInstance,
  (MI_ProviderFT_DeleteInstance)Shell_DeleteInstance,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)NULL,
  (MI_ProviderFT_DisableIndications)NULL,
  (MI_ProviderFT_Subscribe)NULL,
  (MI_ProviderFT_Unsubscribe)NULL,
  (MI_ProviderFT_Invoke)NULL,
};

/* class Shell */
MI_CONST MI_ClassDecl Shell_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x00736C05, /* code */
    MI_T("Shell"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Shell_props, /* properties */
    MI_COUNT(Shell_props), /* numProperties */
    sizeof(Shell), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    Shell_meths, /* methods */
    MI_COUNT(Shell_meths), /* numMethods */
    &schemaDecl, /* schema */
    &Shell_funcs, /* functions */
    NULL, /* owningClass */
};

/*
**==============================================================================
**
** Command
**
**==============================================================================
*/

/* property Command.ShellId */
static MI_CONST MI_PropertyDecl Command_ShellId_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY, /* flags */
    0x00736407, /* code */
    MI_T("ShellId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Command, ShellId), /* offset */
    MI_T("Command"), /* origin */
    MI_T("Command"), /* propagator */
    NULL,
};

/* property Command.CommandId */
static MI_CONST MI_PropertyDecl Command_CommandId_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY, /* flags */
    0x00636409, /* code */
    MI_T("CommandId"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(Command, CommandId), /* offset */
    MI_T("Command"), /* origin */
    MI_T("Command"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST Command_props[] =
{
    &Command_ShellId_prop,
    &Command_CommandId_prop,
};

static MI_CONST MI_ProviderFT Command_funcs =
{
  (MI_ProviderFT_Load)Command_Load,
  (MI_ProviderFT_Unload)Command_Unload,
  (MI_ProviderFT_GetInstance)Command_GetInstance,
  (MI_ProviderFT_EnumerateInstances)Command_EnumerateInstances,
  (MI_ProviderFT_CreateInstance)Command_CreateInstance,
  (MI_ProviderFT_ModifyInstance)Command_ModifyInstance,
  (MI_ProviderFT_DeleteInstance)Command_DeleteInstance,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)NULL,
  (MI_ProviderFT_DisableIndications)NULL,
  (MI_ProviderFT_Subscribe)NULL,
  (MI_ProviderFT_Unsubscribe)NULL,
  (MI_ProviderFT_Invoke)NULL,
};

/* class Command */
MI_CONST MI_ClassDecl Command_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x00636407, /* code */
    MI_T("Command"), /* name */
    NULL, /* qualifiers */
    0, /* numQualifiers */
    Command_props, /* properties */
    MI_COUNT(Command_props), /* numProperties */
    sizeof(Command), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    &Command_funcs, /* functions */
    NULL, /* owningClass */
};

/*
**==============================================================================
**
** __mi_server
**
**==============================================================================
*/

MI_Server* __mi_server;
/*
**==============================================================================
**
** Schema
**
**==============================================================================
*/

static MI_ClassDecl MI_CONST* MI_CONST classes[] =
{
    &Command_rtti,
    &CommandState_rtti,
    &DesiredStream_rtti,
    &Shell_rtti,
    &Stream_rtti,
};

MI_SchemaDecl schemaDecl =
{
    NULL, /* qualifierDecls */
    0, /* numQualifierDecls */
    classes, /* classDecls */
    MI_COUNT(classes), /* classDecls */
};

/*
**==============================================================================
**
** MI_Server Methods
**
**==============================================================================
*/

MI_Result MI_CALL MI_Server_GetVersion(
    MI_Uint32* version){
    return __mi_server->serverFT->GetVersion(version);
}

MI_Result MI_CALL MI_Server_GetSystemName(
    const MI_Char** systemName)
{
    return __mi_server->serverFT->GetSystemName(systemName);
}

