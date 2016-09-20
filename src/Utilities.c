/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include <stdlib.h>
#include <pal/strings.h>
#include <base/logbase.h>
#include <base/log.h>
#include <base/conf.h>
#include <base/paths.h>
#include <MI.h>

MI_Result _GetLogOptionsFromConfigFile(const MI_Char *logfileName)
{
    char path[PAL_MAX_PATH_SIZE];
    MI_Char finalPath[PAL_MAX_PATH_SIZE];
    Conf* conf;

    /* Form the configuration file path */
    Strlcpy(path, OMI_GetPath(ID_CONFIGFILE), sizeof(path));

    /* Open the configuration file */
    conf = Conf_Open(path);
    if (!conf)
    {
        trace_MIFailedToOpenConfigFile(scs(path));
        return MI_RESULT_FAILED;
    }

    /* For each key=value pair in configuration file */
    for (;;)
    {
        const char* key;
        const char* value;
        int r = Conf_Read(conf, &key, &value);

        if (r == -1)
        {
            trace_MIFailedToReadConfigValue(path, scs(Conf_Error(conf)));
            goto error;
        }

        if (r == 1)
            break;

        if (strcmp(key, "loglevel") == 0)
        {
            if (Log_SetLevelFromString(value) != 0)
            {
                trace_MIConfig_InvalidValue(scs(path), Conf_Line(conf), scs(key), scs(value));
                goto error;
            }
        }
    }

    CreateLogFileNameWithPrefix(logfileName, finalPath);
    if (Log_Open(finalPath) != MI_RESULT_OK)
        goto error;

    /* Close configuration file */
    Conf_Close(conf);

    return MI_RESULT_OK;

error:
    Conf_Close(conf);
    return MI_RESULT_INVALID_PARAMETER;
}

