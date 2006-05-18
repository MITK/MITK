INCLUDE_DIRECTORIES( ${IP_ROOT}/Functionalities/QmitkIsoSurface)
INCLUDE_DIRECTORIES( ${IP_ROOT}/Components/QmitkTreeNodeImageSelector)

# IsoSurface

SET( APPMOD_H ${APPMOD_H} ${IP_ROOT}/Functionalities/QmitkIsoSurface/QmitkIsoSurface.h )
SET( APPMOD_MOC_H ${APPMOD_MOC_H} ${IP_ROOT}/Functionalities/QmitkIsoSurface/QmitkIsoSurface.h )
SET( APPMOD_CPP ${APPMOD_CPP} ${IP_ROOT}/Functionalities/QmitkIsoSurface/QmitkIsoSurface.cpp )

SET (APPMOD_FORMS ${APPMOD_FORMS} ${IP_ROOT}/Functionalities/QmitkIsoSurface/QmitkIsoSurfaceControls.ui)
