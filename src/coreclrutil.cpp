/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include "coreclrutil.h"
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <set>
#include <cstdlib>
#include "base/logbase.h"

// The name of the CoreCLR native runtime DLL
#if defined(__APPLE__)
const std::string coreClrDll = "/libcoreclr.dylib";
#else
const std::string coreClrDll = "/libcoreclr.so";
#endif

void* coreclrLib;

// Prototype of the coreclr_initialize function from the libcoreclr.so
typedef int (*InitializeCoreCLRFunction)(
    const char* exePath,
    const char* appDomainFriendlyName,
    int propertyCount,
    const char** propertyKeys,
    const char** propertyValues,
    void** hostHandle,
    unsigned int* domainId);

// Prototype of the coreclr_shutdown function from the libcoreclr.so
typedef int (*ShutdownCoreCLRFunction)(
    void* hostHandle,
    unsigned int domainId);

InitializeCoreCLRFunction initializeCoreCLR;
ShutdownCoreCLRFunction shutdownCoreCLR;
ExecuteAssemblyFunction executeAssembly;
CreateDelegateFunction createDelegate;

//
// Get the absolute path given the environment variable.
//
std::string GetEnvAbsolutePath(const char* env)
{
    char fullpath[PATH_MAX + 1];

    const char* local = std::getenv(env);
    if (!local)
    {
        return std::string("");
    }

    char *ptr = realpath(local, fullpath);
    if (!ptr)
    {
        __LOGE(("Invalid environment variable %s content, switching to default value. ", env));
        return std::string("");
    }
    return std::string(ptr);
}

// Add all *.dll, *.ni.dll, *.exe, and *.ni.exe files from the specified directory to the tpaList string.
// Note: copied from unixcorerun
void AddFilesFromDirectoryToTpaList(const char* directory, std::string& tpaList)
{
    const char * const tpaExtensions[] = {
        ".ni.dll",      // Probe for .ni.dll first so that it's preferred if ni and il coexist in the same dir
        ".dll",
        ".ni.exe",
        ".exe",
    };

    DIR* dir = opendir(directory);
    if (dir == NULL)
    {
        return;
    }

    std::set<std::string> addedAssemblies;

    // Walk the directory for each extension separately so that we first get files with .ni.dll extension,
    // then files with .dll extension, etc.
    for (unsigned int extIndex = 0; extIndex < sizeof(tpaExtensions) / sizeof(tpaExtensions[0]); extIndex++)
    {
        const char* ext = tpaExtensions[extIndex];
        int extLength = strlen(ext);

        struct dirent* entry;

        // For all entries in the directory
        while ((entry = readdir(dir)) != NULL)
        {
            // We are interested in files only
            switch (entry->d_type)
            {
            case DT_REG:
                break;

                // Handle symlinks and file systems that do not support d_type
            case DT_LNK:
            case DT_UNKNOWN:
            {
                std::string fullFilename;

                fullFilename.append(directory);
                fullFilename.append("/");
                fullFilename.append(entry->d_name);

                struct stat sb;
                if (stat(fullFilename.c_str(), &sb) == -1)
                {
                    continue;
                }

                if (!S_ISREG(sb.st_mode))
                {
                    continue;
                }
            }
            break;

            default:
                continue;
            }

            std::string filename(entry->d_name);

            // Check if the extension matches the one we are looking for
            int extPos = filename.length() - extLength;
            if ((extPos <= 0) || (filename.compare(extPos, extLength, ext) != 0))
            {
                continue;
            }

            std::string filenameWithoutExt(filename.substr(0, extPos));

            // Make sure if we have an assembly with multiple extensions present,
            // we insert only one version of it.
            if (addedAssemblies.find(filenameWithoutExt) == addedAssemblies.end())
            {
                addedAssemblies.insert(filenameWithoutExt);

                tpaList.append(directory);
                tpaList.append("/");
                tpaList.append(filename);
                tpaList.append(":");
            }
        }

        // Rewind the directory stream to be able to iterate over it for the next extension
        rewinddir(dir);
    }

    closedir(dir);
}

//
// Below is our custom start/stop interface
//
int startCoreCLR(
    const char* appDomainFriendlyName,
    void** hostHandle,
    unsigned int* domainId)
{
    char exePath[PATH_MAX];

    // get path to current executable
    ssize_t len = readlink("/proc/self/exe", exePath, PATH_MAX);
    if (len == -1 || len == sizeof(exePath))
        len = 0;
    exePath[len] = '\0';

    // get the CoreCLR root path
    std::string clrAbsolutePath = GetEnvAbsolutePath("CORE_ROOT");
    if (clrAbsolutePath.empty())
    {
#if defined(__APPLE__)
        clrAbsolutePath = std::string("/usr/local/bin/pwsh");
#else
        clrAbsolutePath = std::string("/usr/bin/pwsh");
#endif
        char realPath[PATH_MAX + 1];
        char *ptr = realpath(clrAbsolutePath.c_str(), realPath);
        if (ptr == NULL)
        {
            clrAbsolutePath = std::string("");
        }
        else
        {
            std::string followedPath(realPath);
            size_t index = followedPath.find_last_of("/\\");
            if (index == std::string::npos)
            {
                clrAbsolutePath = std::string("");
            }
            else
            {
                clrAbsolutePath = followedPath.substr(0, index);
            }
        }
    }

    if (clrAbsolutePath.empty())
    {
#if defined(__APPLE__)
        clrAbsolutePath = std::string("/usr/local/microsoft/powershell");
#else
        clrAbsolutePath = std::string("/opt/microsoft/powershell");
#endif
    }

    // get the CoreCLR shared library path
    std::string coreClrDllPath(clrAbsolutePath);
    coreClrDllPath += coreClrDll;

    // open the shared library
    coreclrLib = dlopen(coreClrDllPath.c_str(), RTLD_NOW|RTLD_LOCAL);
    if (coreclrLib == NULL)
    {
        __LOGE(("dlopen failed to open the CoreCLR library: %s ", dlerror()));
        return -1;
    }

    // query and verify the function pointers
    initializeCoreCLR = (InitializeCoreCLRFunction)dlsym(coreclrLib,"coreclr_initialize");
    shutdownCoreCLR = (ShutdownCoreCLRFunction)dlsym(coreclrLib,"coreclr_shutdown");
    executeAssembly = (ExecuteAssemblyFunction)dlsym(coreclrLib,"coreclr_execute_assembly");
    createDelegate = (CreateDelegateFunction)dlsym(coreclrLib,"coreclr_create_delegate");

    if (initializeCoreCLR == NULL)
    {
        __LOGE(("function coreclr_initialize not found in CoreCLR library"));
        return -1;
    }
    if (executeAssembly == NULL)
    {
        __LOGE(("function coreclr_execute_assembly not found in CoreCLR library"));
        return -1;
    }
    if (shutdownCoreCLR == NULL)
    {
        __LOGE(("function coreclr_shutdown not found in CoreCLR library"));
        return -1;
    }
    if (createDelegate == NULL)
    {
        __LOGE(("function coreclr_create_delegate not found in CoreCLR library"));
        return -1;
    }

    // generate the Trusted Platform Assemblies list
    std::string tpaList;

    // add assemblies in the CoreCLR root path
    AddFilesFromDirectoryToTpaList(clrAbsolutePath.c_str(), tpaList);

    // create list of properties to initialize CoreCLR
    const char* propertyKeys[] = {
        "TRUSTED_PLATFORM_ASSEMBLIES",
        "APP_PATHS",
        "APP_NI_PATHS",
        "NATIVE_DLL_SEARCH_DIRECTORIES",
        "PLATFORM_RESOURCE_ROOTS",
        "AppDomainCompatSwitch",
        "SERVER_GC",
        "APP_CONTEXT_BASE_DIRECTORY"
    };

    // We use the CORE_ROOT for just about everything: trusted
    // platform assemblies, DLLs, native DLLs, resources, and the
    // AppContext.BaseDirectory. Server garbage collection is disabled
    // by default because of dotnet/cli#652
    const char* propertyValues[] = {
        // TRUSTED_PLATFORM_ASSEMBLIES
        tpaList.c_str(),
        // APP_PATHS
        clrAbsolutePath.c_str(),
        // APP_NI_PATHS
        clrAbsolutePath.c_str(),
        // NATIVE_DLL_SEARCH_DIRECTORIES
        clrAbsolutePath.c_str(),
        // PLATFORM_RESOURCE_ROOTS
        clrAbsolutePath.c_str(),
        // AppDomainCompatSwitch
        "UseLatestBehaviorWhenTFMNotSpecified",
        // SERVER_GC
        "0",
        // APP_CONTEXT_BASE_DIRECTORY
        clrAbsolutePath.c_str()
    };

    // initialize CoreCLR
    int status = initializeCoreCLR(
        exePath,
        appDomainFriendlyName,
        sizeof(propertyKeys)/sizeof(propertyKeys[0]),
        propertyKeys,
        propertyValues,
        hostHandle,
        domainId);

    return status;
}

int stopCoreCLR(void* hostHandle, unsigned int domainId)
{
    // shutdown CoreCLR
    int status = shutdownCoreCLR(hostHandle, domainId);
    if (!SUCCEEDED(status))
    {
        __LOGE(("coreclr_shutdown failed - status: %X", status));
    }

    // close the dynamic library
    if (0 != dlclose(coreclrLib))
    {
        __LOGE(("failed to close CoreCLR library"));
    }

    return status;
}
