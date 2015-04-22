#Begin SpectroCam hardware
OPTION(MITK_USE_BiophotonicsHardware_SpectroCam "Enable support for SpectroCam camera" OFF)

IF(MITK_USE_BiophotonicsHardware_SpectroCam)
  #Find SpectroCam SDK
  FIND_LIBRARY(MITK_SpectroCam_LIB SpectroCam DOC "SpectroCam access library." HINTS "C:\\Program Files (x86)\\Ocean Thin Films\\SpectroCam SDK\\Stage\\64-Bit")
  FIND_PATH(MITK_SpectroCam_INCLUDE_DIR ISpectroCam.h DOC  "Include directory of SpectroCam camera."  HINTS "C:\\Program Files (x86)\\Ocean Thin Films\\SpectroCam SDK\\Include")

  #Find Jai SDK
  FIND_LIBRARY(MITK_JAI_LIB Jai_Factory DOC "SpectroCam access library." HINTS "C:\\Program Files\\JAI\\SDK\\library\\CPP\\lib\\Win64_x64" "C:\\Programme\\JAI\\SDK\\library\\CPP\\lib\\Win64_x64")
  FIND_PATH(MITK_JAI_INCLUDE_DIR Jai_Factory.h DOC  "Include directory of SpectroCam camera."  HINTS "C:\\Program Files\\JAI\\SDK\\library\\CPP\\include" "C:\\Programme\\JAI\\SDK\\library\\CPP\\include")

  SET(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${MITK_SpectroCam_LIB} ${MITK_JAI_LIB})
  SET(INCLUDE_DIRS_INTERNAL ${INCLUDE_DIRS_INTERNAL} ${MITK_SpectroCam_INCLUDE_DIR} ${MITK_JAI_INCLUDE_DIR})


MITK_CREATE_MODULE(
  INCLUDE_DIRS ${MITK_BIN_DIR}
     PUBLIC ${INCLUDE_DIRS_INTERNAL}
  ADDITIONAL_LIBS ${ADDITIONAL_LIBS}
  DEPENDS MitkCore MitkOpenCVVideoSupport
  PACKAGE_DEPENDS OpenCV ITK|ITKIOImageBase
#     PUBLIC ${ADDITIONAL_LIBS}
)


Message("SpectroCam Module generated")

ENDIF(MITK_USE_BiophotonicsHardware_SpectroCam)
#End SpectroCam Hardware
