# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1

set(path "Modules/ThirdParty/GDCM/itk-module-init.cmake")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "en" "  include(\${GDCM_USE_FILE})\nen" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "Modules/ThirdParty/GDCM/src/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "SFF)" "SFF gdcmDSED gdcmCommon)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# http://bugs.mitk.org/show_bug.cgi?id=14954, http://review.source.kitware.com/#/c/11518
set(path "Modules/IO/GDCM/src/itkGDCMImageIO.cxx")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(FIND ${contents} "ImageApplyLookupTable is meant to change the pixel type for PALETTE_COLOR images" position)
if (position EQUAL -1)
  string(REPLACE "itkAssertInDebugAndIgnoreInReleaseMacro( pixeltype_debug == pixeltype );" ";" contents ${contents})
  set(CONTENTS ${contents})
  configure_file(${TEMPLATE_FILE} ${path} @ONLY)
endif()
