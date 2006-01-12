INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkImageCropper
                     ${IP_ROOT}/AppModules/QmitkApplicationBase
                   )
SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkImageCropper/QmitkImageCropper.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkImageCropper/QmitkImageCropper.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkImageCropper/QmitkImageCropper.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkImageCropper/QmitkImageCropperControls.ui)
