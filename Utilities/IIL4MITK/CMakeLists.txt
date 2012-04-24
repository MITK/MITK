find_package(VTK)
include(${VTK_USE_FILE})
# mitkGL.h is in Core/Rendering :(
find_package(OpenGL)
if(NOT OPENGL_FOUND)
  message("GL is required for MITK rendering")
endif(NOT OPENGL_FOUND )
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(../ipPic)

add_library(IIL4MITK ${MITK_WIN32_FORCE_STATIC} image.cpp  item.cpp  picimage.cpp  texture.cpp)
target_link_libraries(IIL4MITK ${OPENGL_LIBRARIES} ${IPPIC_LIBRARIES})
MITK_CREATE_MODULE_CONF(IIL4MITK
  DEPENDS mitkIpPic
)

