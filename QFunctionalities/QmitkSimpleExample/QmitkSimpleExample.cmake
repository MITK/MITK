INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkSimpleExample)

# an example of a functionality

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkSimpleExample/QmitkSimpleExampleControls.ui)
