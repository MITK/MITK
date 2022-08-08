foreach(opencv_module ${OpenCV_REQUIRED_COMPONENTS_BY_MODULE})
  if(NOT opencv_module MATCHES "^opencv_")
    set(opencv_module "opencv_${opencv_module}")
  endif()
  list(APPEND _opencv_required_components_by_module ${opencv_module})
endforeach()

find_package(OpenCV COMPONENTS ${_opencv_required_components_by_module} REQUIRED)

foreach(opencv_module ${_opencv_required_components_by_module})
  list(APPEND ALL_LIBRARIES ${opencv_module})
endforeach()
