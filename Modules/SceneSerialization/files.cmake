file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkGeometryDataSerializer.cpp
  mitkImageSerializer.cpp
  mitkPointSetSerializer.cpp
  mitkPropertyListDeserializer.cpp
  mitkPropertyListDeserializerV1.cpp
  mitkSceneIO.cpp
  mitkSceneReader.cpp
  mitkSceneReaderV1.cpp
  mitkSurfaceSerializer.cpp
)

