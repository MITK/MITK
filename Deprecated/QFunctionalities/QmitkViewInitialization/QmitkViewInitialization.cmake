INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkViewInitialization)

# ViewInitialization

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkViewInitialization/QmitkViewInitialization.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkViewInitialization/QmitkViewInitialization.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkViewInitialization/QmitkViewInitialization.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkViewInitialization/QmitkViewInitializationControls.ui)
