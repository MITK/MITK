file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkBaseDataIOFactory.cpp
  mitkFileSeriesReader.cpp
  mitkImageWriter.cpp
  mitkImageWriterFactory.cpp
  mitkItkImageFileIOFactory.cpp
  mitkItkImageFileReader.cpp
  mitkItkPictureWrite.cpp
  mitkPointSetIOFactory.cpp
  mitkPointSetReader.cpp
  mitkPointSetWriter.cpp
  mitkPointSetWriterFactory.cpp
  mitkRawImageFileReader.cpp
  mitkSTLFileIOFactory.cpp
  mitkSTLFileReader.cpp
  mitkSurfaceVtkWriter.cpp
  mitkSurfaceVtkWriterFactory.cpp
  mitkVtiFileIOFactory.cpp
  mitkVtiFileReader.cpp
  mitkVtkImageIOFactory.cpp
  mitkVtkImageReader.cpp
  mitkVtkSurfaceIOFactory.cpp
  mitkVtkSurfaceReader.cpp
  vtkPointSetXMLParser.cpp
)
