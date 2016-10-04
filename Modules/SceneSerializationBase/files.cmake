file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkAnnotationPropertySerializer.cpp
  mitkBaseDataSerializer.cpp
  mitkBasePropertySerializer.cpp
  mitkBoolLookupTablePropertySerializer.cpp
  mitkBoolPropertySerializer.cpp
  mitkClippingPropertySerializer.cpp
  mitkColorPropertySerializer.cpp
  mitkDoublePropertySerializer.cpp
  mitkEnumerationPropertySerializer.cpp
  mitkEnumerationSubclassesSerializer.cpp
  mitkFloatLookupTablePropertySerializer.cpp
  mitkFloatPropertySerializer.cpp
  mitkGroupTagPropertySerializer.cpp
  mitkIntLookupTablePropertySerializer.cpp
  mitkIntPropertySerializer.cpp
  mitkLevelWindowPropertySerializer.cpp
  mitkLookupTablePropertySerializer.cpp
  mitkPoint3dPropertySerializer.cpp
  mitkPoint3iPropertySerializer.cpp
  mitkPoint4dPropertySerializer.cpp
  mitkPropertyListSerializer.cpp
  mitkStringLookupTablePropertySerializer.cpp
  mitkStringPropertySerializer.cpp
  mitkTemporoSpatialStringPropertySerializer.cpp
  mitkTransferFunctionPropertySerializer.cpp
  mitkUIntPropertySerializer.cpp
  mitkUShortPropertySerializer.cpp
  mitkVector3DPropertySerializer.cpp
  mitkVectorPropertySerializer.cpp
)

