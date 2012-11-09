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

#ifndef MITK_SPACENAVIGATORVTKCAMERACONTROLLER_H_
#define MITK_SPACENAVIGATORVTKCAMERACONTROLLER_H_

#include <mitkCameraController.h>
#include <mitkSpaceNavigatorExports.h>


class vtkRenderWindow;

namespace mitk {

  /**
  * The Space Navigator camera controller listens to events from a Space Navigator. <br>
  * After receiving such events, it moves the camera of the 3D window accordingly.
  *
  * @brief vtk-based camera controller
  * @ingroup NavigationControl
  */
  class mitkSpaceNavigator_EXPORT SpaceNavigatorVtkCameraController : public CameraController
  {

  public:

    //SmartPointer Macros
    mitkClassMacro(SpaceNavigatorVtkCameraController, CameraController);
    itkNewMacro(Self);

    virtual bool OnSpaceNavigatorEvent(mitk::Action* a, const mitk::StateEvent* e);
    virtual bool OnSpaceNavigatorKeyDown(mitk::Action* a, const mitk::StateEvent* e);

  protected:

    /**
    * Default Constructor
    */
    SpaceNavigatorVtkCameraController();

    /**
    * Default Destructor
    */
    virtual ~SpaceNavigatorVtkCameraController();

    bool m_ClippingRangeIsSet;

  }; // end class SpaceNavigatorVtkCameraController
} // namespace mitk

#endif /* MITK_SPACENAVIGATORVTKCAMERACONTROLLER_H_ */
