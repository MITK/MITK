INCLUDE_DIRECTORIES(../../Framework/QmitkApplicationBase ../../Functionalities/QmitkSimpleExample)

# an example of a functionality

SET( APPMOD_H ${APPMOD_H} ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_CPP ${APPMOD_CPP} ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleControls.ui)
