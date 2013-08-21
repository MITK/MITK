macro(use_qt5_module module)

  find_package(Qt5${module} REQUIRED)

  list(APPEND ALL_INCLUDE_DIRECTORIES ${Qt5${module}_INCLUDE_DIRS})
  list(APPEND ALL_LIBRARIES ${Qt5${module}_LIBRARIES})

endmacro()

use_qt5_module(Core)
use_qt5_module(Quick)
use_qt5_module(Qml)
use_qt5_module(Widgets)
use_qt5_module(OpenGL)

# Qt demands this.. (at least in my build. error message is:
# ../qt-5.1/include/QtCore/qglobal.h:975:4: error: #error "You must build your code with position independent code if Qt was built with -reduce-relocations. " "Compile your code with -fPIC or -fPIE."
#
add_definitions(-fPIC)
