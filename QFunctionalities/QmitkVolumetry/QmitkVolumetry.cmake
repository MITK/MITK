INCLUDE_DIRECTORIES(${IP_ROOT}/Framework/QmitkApplicationBase ${IP_ROOT}/Functionalities/QmitkVolumetry)

# Volumetry

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkVolumetry/QmitkVolumetry.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkVolumetry/QmitkVolumetry.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkVolumetry/QmitkVolumetry.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkVolumetry/QmitkVolumetryControls.ui)
