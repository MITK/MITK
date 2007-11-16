#ifndef _MITK_CHILI_MACROS__H_
#define _MITK_CHILI_MACROS__H_


// this macro is used to prevent unused variable warnings
// in case CHILI_PLUGIN_VERSION_CODE is not defined
#ifndef  CHILI_PLUGIN_VERSION_CODE
  #define mitkHideIfNoVersionCode(x) 
#else
  #define mitkHideIfNoVersionCode(x) x
#endif


#endif

