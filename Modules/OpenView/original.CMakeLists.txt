qt5_wrap_cpp( ovCore_moc_files
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h )

set( ovCore_srcs
  vtkEventQtSlotConnect.cxx
  QVTKInteractor.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  vtkQtConnection.cxx
  ${ovCore_moc_files} )

set( ovCore_libs
  vtkRenderingFreeTypeOpenGL
  vtkRenderingVolumeOpenGL
  vtkInteractionStyle
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Quick_LIBRARIES} )

add_library(ovCore SHARED ${ovCore_srcs})
target_link_libraries(ovCore ${ovCore_libs})

# Generate export header.
generate_export_header("ovCore" EXPORT_FILE_NAME "ovCoreModule.h")

if(APPLE)
  install(TARGETS ovCore LIBRARY DESTINATION "OpenView.app/Contents/Libraries")
  install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id  @executable_path/../Libraries/libovCore.dylib ${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/libovCore.dylib)")

  # An odd requirement: since CMake decides to do some install_name_tool
  # stuff automatically, it already changed the full path of libovCore.dylib
  # to simply the library file with no path. So here we need to change
  # that change to instead be relative to @executable_path. If CMake did
  # "help" us like that, we would expect to use the following line:
  #
  #   set(install_name_changes "${install_name_changes} -change ${CMAKE_CURRENT_BINARY_DIR}/libovCore.dylib @executable_path/../Libraries/libovCore.dylib" PARENT_SCOPE)
  set(install_name_changes "${install_name_changes} -change libovCore.dylib @executable_path/../Libraries/libovCore.dylib" PARENT_SCOPE)

  list(APPEND libs_and_execs "${CMAKE_INSTALL_PREFIX}/OpenView.app/Contents/Libraries/libovCore.dylib")
  set(libs_and_execs ${libs_and_execs} PARENT_SCOPE)
else()
  install(TARGETS ovCore
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib")
endif()
