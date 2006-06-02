INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkVolumeVisualization)

# VolumeVisualization
# INCLUDE(${IP_ROOT}/AppModules/QmitkTransferFunctionWidget/QmitkTransferFunctionWidget.cmake)

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkVolumeVisualization/QmitkVolumeVisualization.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkVolumeVisualization/QmitkVolumeVisualization.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkVolumeVisualization/QmitkVolumeVisualization.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkVolumeVisualization/QmitkVolumeVisualizationControls.ui)
