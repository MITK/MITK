INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkFunctionalityTemplate)

# FunctionalityTemplate

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkFunctionalityTemplate/QmitkFunctionalityTemplate.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkFunctionalityTemplate/QmitkFunctionalityTemplate.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkFunctionalityTemplate/QmitkFunctionalityTemplate.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkFunctionalityTemplate/QmitkFunctionalityTemplateControls.ui)
