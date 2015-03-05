# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1

set(path "Modules/ThirdParty/GDCM/src/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
# more robust search for replacement
string(REPLACE "gdcmMSFF" "gdcmMSFF gdcmDSED gdcmCommon" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

