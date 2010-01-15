@PLUGIN_COPYRIGHT@

#ifndef _@DLL_DEFINE@_DLL_H_
#define _@DLL_DEFINE@_DLL_H_


//
// The following block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the @NormalizedPluginID@_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// @NormalizedPluginID@_EXPORTS functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && !defined(@PROJECT_STATIC_VAR@)
  #if defined(@NormalizedPluginID@_EXPORTS)
    #define @DLL_DEFINE@ __declspec(dllexport)
  #else
    #define @DLL_DEFINE@ __declspec(dllimport)
  #endif
#endif


#if !defined(@DLL_DEFINE@)
  #define @DLL_DEFINE@
#endif

#endif /*_@DLL_DEFINE@_DLL_H_*/
