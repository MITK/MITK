FIND_PACKAGE(VTK)

# mitkGL.h is in Core/Rendering :(
INCLUDE_DIRECTORIES(../ipPic ${VTK_INCLUDE_DIRS} ${MITK_SOURCE_DIR}/Core/Rendering)

ADD_LIBRARY(IIL4MITK image.cpp  item.cpp  picimage.cpp  texture.cpp)
TARGET_LINK_LIBRARIES(IIL4MITK ${IPPIC_LIBRARIES})
