SOURCES	+= ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.cpp
HEADERS	+= ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleFunctionality.h
# an example of a functionality 		  
FORMS	+= ../../Functionalities/QmitkSimpleExample/QmitkSimpleExampleControls.ui \
			../../AppModules/QmitkNavigators/QmitkSliderNavigator.ui
TEMPLATE	=app
INCLUDEPATH	+= ../../AppModules/QmitkApplicationBase ../../Functionalities/QmitkSimpleExample ../../AppModules/QmitkNavigators
LANGUAGE	= C++

