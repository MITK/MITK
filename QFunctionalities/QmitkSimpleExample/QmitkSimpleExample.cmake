INCLUDE_DIRECTORIES(../../Framework/QmitkApplicationBase ../../AppModule/QmitkSimpleExample)

# an example of a functionality

SET( APPMOD_H ${APPMOD_H} ../../AppModule/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h )
SET( APPMOD_CPP ${APPMOD_CPP} ../../AppModule/QmitkSimpleExample/QmitkSimpleExampleFunctionality.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ../../AppModule/QmitkSimpleExample/QmitkSimpleExampleControls.ui)
