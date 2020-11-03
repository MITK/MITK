/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
* \brief Implements the handling of events that are missing for MITK interaction.
*
* This class inherits from vtkInteractorStyleUser, a class that handles
* VTK-Events and invokes callbacks by means of an observer pattern.
*
* Most event-types needed for typical MITK interaction have already
* been implemented in vtkInteractorStyleUser (Mouse-Buttons + Keyboard).
* However, wheel-events and widgetModifed-events (whatever these are)
* have not been handled so far. This is the purpose of this class.
*/

#ifndef __mitkVtkInteractorStyle_h
#define __mitkVtkInteractorStyle_h

#include "MitkCoreExports.h"
#include <vtkInteractorStyleUser.h>

class MITKCORE_EXPORT mitkVtkInteractorStyle : public vtkInteractorStyleUser
{
public:
  // default VTK c'tor
  static mitkVtkInteractorStyle *New();
  vtkTypeMacro(mitkVtkInteractorStyle, vtkInteractorStyleUser);

  /**
  * \brief Called when scrolling forwards with the mouse-wheel.
  */
  void OnMouseWheelForward() override;

  /**
  * \brief Called when scrolling backwards with the mouse-wheel.
  */
  void OnMouseWheelBackward() override;

protected:
  mitkVtkInteractorStyle();
  ~mitkVtkInteractorStyle() override;

private:
  mitkVtkInteractorStyle(const mitkVtkInteractorStyle &); // Not implemented.
  void operator=(const mitkVtkInteractorStyle &);         // Not implemented.
};

#endif
