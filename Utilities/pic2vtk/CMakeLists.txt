find_package(VTK)
include(${VTK_USE_FILE})

include_directories(.. ../ipPic)
add_library(pic2vtk ${MITK_WIN32_FORCE_STATIC} pic2vtk.cpp)
target_link_libraries(pic2vtk mitkIpPic vtkIO)

MITK_CREATE_MODULE_CONF(pic2vtk
  DEPENDS mitkIpPic
)

