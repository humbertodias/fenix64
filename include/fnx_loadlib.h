#ifndef _FNX_LOADLIB_H
    #define _FNX_LOADLIB_H

    /* WIN32 INCLUDES */
    #ifdef WIN32
        #include <windows.h>
        #include <winbase.h>
    #endif
    #ifdef TARGET_MAC
        #include <dlfcn.h>
        #include <unistd.h>
        #define __stdcall
    #endif
     #ifdef TARGET_linux
        #include <dlfcn.h>
        #include <unistd.h>
        #define __stdcall
    #endif
    #ifdef TARGET_BEOS
        #include <unistd.h>
		#include <dlfcn.h>
    #endif

    typedef void (__stdcall  * dlfunc) (void *(*)(char *), void (*)(char *, char *, int, void *));

    #ifdef WIN32
        #define dlopen(a,b)     LoadLibrary(a)
        #define dlsym(a,b)      (dlfunc)GetProcAddress(a,b)

        static char * dlerror (void)
        {
            static char * buffer = "Could not load library." ;
            return buffer;
        }
    #endif

#endif
