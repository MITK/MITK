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


#ifndef MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include <MitkCoreExports.h>
#include "mitkCameraController.h"
#include "vtkRenderWindowInteractor.h"

class vtkRenderWindow;

namespace mitk {

//##Documentation
//## @brief vtk-based camera controller
//## @ingroup NavigationControl
class MITKCORE_EXPORT VtkInteractorCameraController : public CameraController
{
public:
  mitkClassMacro(VtkInteractorCameraController, CameraController);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  //##Documentation
  //## @brief Returns the vtkRenderWindowInteractor used internally by this CameraController
  vtkRenderWindowInteractor* GetVtkInteractor();

  virtual void Resize(int w, int h) override;
  virtual void MousePressEvent(mitk::MouseEvent*) override;
  virtual void MouseReleaseEvent(mitk::MouseEvent*) override;
  virtual void MouseMoveEvent(mitk::MouseEvent*) override;
  virtual void KeyPressEvent(mitk::KeyEvent*) override;

  //##Documentation
  //## @brief Set the BaseRenderer to be controlled by this vtk-based camera controller
  virtual void SetRenderer(const mitk::BaseRenderer* renderer) override;

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
