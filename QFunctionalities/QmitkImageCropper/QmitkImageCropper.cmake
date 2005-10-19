INCLUDE_DIRECTORIES( ../../Functionalities/QmitkImageCropper
                     ../../AppModules/QmitkApplicationBase
                   )
SET( APPMOD_H ${APPMOD_H} ../../Functionalities/QmitkImageCropper/QmitkImageCropper.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ../../Functionalities/QmitkImageCropper/QmitkImageCropper.h )
SET( APPMOD_CPP ${APPMOD_CPP} ../../Functionalities/QmitkImageCropper/QmitkImageCropper.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ../../Functionalities/QmitkImageCropper/QmitkImageCropperControls.ui)
