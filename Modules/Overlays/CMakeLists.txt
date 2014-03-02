# unused variable found in qt src related to bug-16780 on osx using the compiler: Apple LLVM version 4.2 (clang-425.0.28) (based on LLVM 3.2svn) Target: x86_64-apple-darwin12.4.0

# In file included from /Users/Cordes/developing/src/MITK-Master/Modules/Overlays/QmitkScalarBar.cpp:20:
# In file included from /Users/Cordes/dartclients/toolkits/qt-4.8.4-Build/include/QtGui/QPaintEvent:1:
# In file included from /Users/Cordes/dartclients/toolkits/qt-4.8.4-Build/include/QtGui/qevent.h:52:
# /Users/Cordes/dartclients/toolkits/qt-4.8.4-Build/include/QtGui/qmime.h:119:10: error: private field 'type' is not used [-Werror,-Wunused-private-field]
#     char type;
#           ^
# 1 error generated.

# start fix for the module Overlays

if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unused-private-field" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

# end fix

MITK_CREATE_MODULE(DEPENDS MitkQtWidgets
                   WARNINGS_AS_ERRORS
)
