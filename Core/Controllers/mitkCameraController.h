/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722


#include "mitkCommon.h"
#include "mitkBaseController.h"
#include "mitkEventTypedefs.h"
class mitk::BaseRenderer;

namespace mitk {

//##Documentation
//## @brief controls the camera used by the associated BaseRenderer
//##
//## Subclass of BaseController. Controls the camera used by the associated
//## BaseRenderer.
//## @ingroup NavigationControl
class MITK_CORE_EXPORT CameraController : public BaseController
{
public:
    enum StandardView { ANTERIOR,POSTERIOR,SINISTER,DEXTER,CRANIAL,CAUDAL };

    mitkClassMacro(CameraController, BaseController);
    mitkNewMacro1Param(Self, const char*);
    
    itkSetMacro(Renderer,BaseRenderer*);
    itkGetConstMacro(Renderer,const BaseRenderer*);

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
};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
