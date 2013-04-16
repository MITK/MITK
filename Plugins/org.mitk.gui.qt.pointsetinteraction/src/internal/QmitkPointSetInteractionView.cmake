# QmitkPointSetInteractionView

message(STATUS "processing QmitkPointSetInteractionView.cmake")
message(STATUS "APPMOD_CPP before: ${APPMOD_CPP}")

set( APPMOD_H ${APPMOD_H} QmitkPointSetInteractionView.h )
set( APPMOD_MOC_H ${APPMOD_MOC_H} QmitkPointSetInteractionView.h )
set( APPMOD_CPP ${APPMOD_CPP} QmitkPointSetInteractionView.cpp )
set(APPMOD_FORMS ${APPMOD_FORMS} QmitkPointSetInteractionControls.ui)
message(STATUS "APPMOD_CPP after: ${APPMOD_CPP}")

