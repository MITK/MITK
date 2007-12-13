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


#ifndef MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "mitkCameraController.h"
#include "vtkRenderWindowInteractor.h"

class vtkRenderWindow;

namespace mitk {

//##Documentation
//## @brief vtk-based camera controller
//## @ingroup NavigationControl
class VtkInteractorCameraController : public CameraController
{
public:
  mitkClassMacro(VtkInteractorCameraController, CameraController);
  itkNewMacro(Self);
  
  //##Documentation
  //## @brief Returns the vtkRenderWindowInteractor used internally by this CameraController
  vtkRenderWindowInteractor* GetVtkInteractor();

  virtual void Resize(int w, int h);
  virtual void MousePressEvent(mitk::MouseEvent*);
  virtual void MouseReleaseEvent(mitk::MouseEvent*);
  virtual void MouseMoveEvent(mitk::MouseEvent*);
  virtual void KeyPressEvent(mitk::KeyEvent*);

  //##Documentation
  //## @brief Set the BaseRenderer to be controlled by this vtk-based camera controller
  virtual void SetRenderer(const mitk::BaseRenderer* renderer);

protected:
  /**
  * @brief Default Constructor
  **/
  VtkInteractorCameraController(const char* type = NULL);

  /**
  * @brief Default Destructor
  **/
  virtual ~VtkInteractorCameraController();

//  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  vtkRenderWindowInteractor* m_VtkInteractor;
};

} // namespace mitk

#endif /* MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
