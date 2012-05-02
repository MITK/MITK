# Called by ITK.cmake (ExternalProject_Add) as a patch for ITK to work witk GDCM 2.0.18

# First patch
# updates library dependencies
# read whole file CMakeLists.txt
file(STRINGS CMakeLists.txt sourceCode NEWLINE_CONSUME)

# substitute dependency to gdcmMSFF by dependencies for more libraries
string(REGEX REPLACE "gdcmMSFF" "gdcmMSFF gdcmDICT gdcmCommon gdcmDSED" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} CMakeLists.txt @ONLY)

# second patch
# read whole file
file(STRINGS Code/Common/itkLandmarkBasedTransformInitializer.h sourceCode2 NEWLINE_CONSUME)

# backported fix from ITK4
string(REGEX REPLACE "typedef typename ParametersType::ValueType +ParameterValueType;" "typedef typename TransformType::ScalarType                      ParameterValueType;" sourceCode2 ${sourceCode2})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode2})
configure_file(${TEMPLATE_FILE} Code/Common/itkLandmarkBasedTransformInitializer.h @ONLY)

# third patch
# read whole file
file(STRINGS Code/Common/itkImageSource.h sourceCode3 NEWLINE_CONSUME)

# remove ITK_NO_RETURN since the method sometimes returns which makes clang based builds crash
string(REGEX REPLACE "ITK_NO_RETURN" "" sourceCode3 ${sourceCode3})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode3})
configure_file(${TEMPLATE_FILE} Code/Common/itkImageSource.h @ONLY)

