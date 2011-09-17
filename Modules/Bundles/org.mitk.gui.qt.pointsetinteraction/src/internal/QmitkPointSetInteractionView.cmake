# QmitkPointSetInteractionView

MESSAGE(STATUS "processing QmitkPointSetInteractionView.cmake")
MESSAGE(STATUS "APPMOD_CPP before: ${APPMOD_CPP}")

SET( APPMOD_H ${APPMOD_H} QmitkPointSetInteractionView.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} QmitkPointSetInteractionView.h )
SET( APPMOD_CPP ${APPMOD_CPP} QmitkPointSetInteractionView.cpp )
SET (APPMOD_FORMS ${APPMOD_FORMS} QmitkPointSetInteractionControls.ui)
MESSAGE(STATUS "APPMOD_CPP after: ${APPMOD_CPP}")

