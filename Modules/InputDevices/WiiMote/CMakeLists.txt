IF(WIN32)

OPTION(MITK_USE_WIIMOTE "Use wiimote" OFF)

IF(MITK_USE_WIIMOTE)

# was removed due to the fact that it did not work properly with WDK 7.1.0 and VC8
# has to be added manually into the IDE - more details on www.mitk.org
# #find include directory of WDK / DDK and add it to include dirs of module
# FIND_PATH(MITK_WII_WDK_INC "Include Dir DDK or WDK (hidsdi.h)")
# #find lib directory of WDK / DDK and add it to additional lib dirs of module
# #FIND_PATH(MITK_WII_WDK_LIB "Library Dir DDK or WDK (hid.lib)")
# FIND_LIBRARY(MITK_WII_WDK_LIB "hid.lib within DDK or WDK")

MITK_CREATE_MODULE(mitkWiiMote
  INCLUDE_DIRS wiiyourself #${MITK_WII_WDK_INC} 
  DEPENDS Mitk mitkOpenCVVideoSupport
  #ADDITIONAL_LIBS "${MITK_WII_WDK_LIB}"
)
ENDIF(MITK_USE_WIIMOTE)

ENDIF(WIN32)

