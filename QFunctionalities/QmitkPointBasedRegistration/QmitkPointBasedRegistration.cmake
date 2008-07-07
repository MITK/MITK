INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkPointBasedRegistration)

# PointBasedRegistration

SET( APPMOD_H ${APPMOD_H} ../../Functionalities/QmitkPointBasedRegistration/QmitkPointBasedRegistration.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ../../Functionalities/QmitkPointBasedRegistration/QmitkPointBasedRegistration.h )
SET( APPMOD_CPP ${APPMOD_CPP} ../../Functionalities/QmitkPointBasedRegistration/QmitkPointBasedRegistration.cpp
../../Functionalities/QmitkPointBasedRegistration/QmitkPointBasedRegistrationTesting.cpp
../../Functionalities/QmitkPointBasedRegistration/LandmarkWarping.cpp
 )

SET (APPMOD_FORMS ${APPMOD_FORMS} ../../Functionalities/QmitkPointBasedRegistration/QmitkPointBasedRegistrationControls.ui)
