INCLUDE_DIRECTORIES(../../Framework/QmitkApplicationBase ../../AppModules/QmitkSimpleExample)

# an example of a functionality

SET( APPMOD_MOC_H ${APPMOD_MOC_H} ../../AppModules/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_CPP ${APPMOD_CPP} ../../AppModules/QmitkSimpleExample/QmitkSimpleExampleFunctionality.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ../../AppModules/QmitkSimpleExample/QmitkSimpleExampleControls.ui)
