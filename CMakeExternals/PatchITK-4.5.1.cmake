# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work with external GDCM 2.2.1

set(path "Modules/ThirdParty/GDCM/src/CMakeLists.txt")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "SFF)" "SFF gdcmDSED gdcmDICT gdcmCommon gdcmcharls gdcmopenjpeg gdcmuuid gdcmjpeg12 gdcmjpeg16 gdcmjpeg8 gdcmzlib gdcmIOD gdcmexpat)" contents ${contents})
set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

