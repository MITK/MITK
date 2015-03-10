/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722


#include <MitkCoreExports.h>
#include "mitkBaseController.h"
#include "mitkDisplayPositionEvent.h"

namespace mitk {

class KeyEvent;

//##Documentation
//## @brief controls the camera used by the associated BaseRenderer
//##
//## Subclass of BaseController. Controls the camera used by the associated
//## BaseRenderer.
//## @ingroup NavigationControl
class MITKCORE_EXPORT CameraController : public BaseController
{
public:
    enum StandardView { ANTERIOR,POSTERIOR,SINISTER,DEXTER,CRANIAL,CAUDAL };

    mitkClassMacro(CameraController, BaseController);
    mitkNewMacro1Param(Self, const char*);

    itkSetMacro(Renderer,BaseRenderer*);
    itkSetMacro(ZoomFactor,double);
    itkGetConstMacro(Renderer,const BaseRenderer*);
    itkGetConstMacro(ZoomFactor,double);

    //##Documentation
    //## @brief Implemented in sub-classes.
    virtual void Resize(int w, int h);

    //##Documentation
    //## @brief Implemented in sub-classes.
    virtual void MousePressEvent(mitk::MouseEvent*);
    //##Documentation
    //## @brief Implemented in sub-classes.
    virtual void MouseReleaseEvent(mitk::MouseEvent*);
    //##Documentation
    //## @brief Implemented in sub-classes.
    virtual void MouseMoveEvent(mitk::MouseEvent*);
    //##Documentation
    //## @brief Implemented in sub-classes.
    virtual void KeyPressEvent(mitk::KeyEvent*);

   virtual void SetViewToAnterior();
   virtual void SetViewToPosterior();
   virtual void SetViewToSinister();
   virtual void SetViewToDexter();
   virtual void SetViewToCranial();
   virtual void SetViewToCaudal();
   virtual void SetStandardView(StandardView view);

protected:
    /**
    * @brief Default Constructor
    **/
    CameraController(const char * type = NULL);

    /**
    * @brief Default Destructor
    **/
    virtual ~CameraController();
    const BaseRenderer* m_Renderer;

    double m_ZoomFactor; ///< zoom factor used for the standard view camera
};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
