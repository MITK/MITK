INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkMeasurement)

# Measurement

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkMeasurement/QmitkMeasurement.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkMeasurement/QmitkMeasurement.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkMeasurement/QmitkMeasurement.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkMeasurement/QmitkMeasurementControls.ui)
