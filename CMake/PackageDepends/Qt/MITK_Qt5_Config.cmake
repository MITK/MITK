macro(use_qt5_module module)

  find_package(${module} REQUIRED)

  list(APPEND ALL_INCLUDE_DIRECTORIES ${${module}_INCLUDE_DIRS})
  list(APPEND ALL_LIBRARIES ${${module}_LIBRARIES})

endmacro()

if (NOT WIN32)
  # Qt demands this.. (at least in my build. error message is:
  # ../qt-5.1/include/QtCore/qglobal.h:975:4: error: #error "You must build your code with position independent code if Qt was built with -reduce-relocations. " "Compile your code with -fPIC or -fPIE."
  #
  add_definitions(-fPIC)
endif()
