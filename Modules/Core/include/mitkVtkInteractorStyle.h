/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkInteractorStyle_h
#define mitkVtkInteractorStyle_h

#include "MitkCoreExports.h"
#include <vtkInteractorStyleUser.h>

namespace mitk
{
  /**
  * \brief Implements the handling of events that are missing for MITK interaction.
  *
  * This class inherits from vtkInteractorStyleUser, a class that handles
  * VTK-Events and invokes callbacks by means of an observer pattern.
  *
  * Most event-types needed for typical MITK interaction have already
  * been implemented in vtkInteractorStyleUser (Mouse-Buttons + Keyboard).
  * However, wheel-events and widgetModified-events (whatever these are)
  * have not been handled so far. This is the purpose of this class.
  */

  class MITKCORE_EXPORT VtkInteractorStyle : public vtkInteractorStyleUser
  {
  public:
    // default VTK c'tor
    static VtkInteractorStyle* New();
    vtkTypeMacro(VtkInteractorStyle, vtkInteractorStyleUser);

    /**
    * \brief Called when scrolling forwards with the mouse-wheel.
    */
    void OnMouseWheelForward() override;

    /**
    * \brief Called when scrolling backwards with the mouse-wheel.
    */
    void OnMouseWheelBackward() override;

  protected:
    VtkInteractorStyle();
    ~VtkInteractorStyle() override;

    VtkInteractorStyle(const VtkInteractorStyle&) = delete;
    void operator=(const VtkInteractorStyle&) = delete;
  };
}

#endif
