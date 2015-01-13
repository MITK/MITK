if(TARGET itksys)
  set(ITK_TARGETS_IMPORTED 1)
endif()

find_package(ITK COMPONENTS ${ITK_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED)

if(NOT DEFINED ITK_NO_IO_FACTORY_REGISTER_MANAGER)
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1)
endif()
set(NO_DIRECTORY_SCOPED_ITK_COMPILE_DEFINITION 1)
include(${ITK_USE_FILE})

list(APPEND ALL_LIBRARIES ${ITK_LIBRARIES})
list(APPEND ALL_INCLUDE_DIRECTORIES ${ITK_INCLUDE_DIRS})
if(NOT ITK_NO_IO_FACTORY_REGISTER_MANAGER)
  # We manually add the define which will be of target scope. MITK
  # patches ITK_USE_FILE to remove the directory scoped compile
  # definition since it would be propagated to other targets in the
  # same directory scope but these targets might want to *not*
  # use the ITK factory manager stuff.
  list(APPEND ALL_COMPILE_DEFINITIONS ITK_IO_FACTORY_REGISTER_MANAGER)
endif()

# ITK_USE_FILE does not reset ITK_MODULES_REQUESTED which leads to buggy
# behaviour if it is included more than once in the same scope
unset(ITK_MODULES_REQUESTED)
