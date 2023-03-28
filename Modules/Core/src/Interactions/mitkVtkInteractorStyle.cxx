/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVtkInteractorStyle.h>

#include <vtkCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(mitk::VtkInteractorStyle);

mitk::VtkInteractorStyle::VtkInteractorStyle() : vtkInteractorStyleUser()
{
}

mitk::VtkInteractorStyle::~VtkInteractorStyle()
{
}

void mitk::VtkInteractorStyle::OnMouseWheelForward()
{
  if (this->HasObserver(vtkCommand::MouseWheelForwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
  }
}

void mitk::VtkInteractorStyle::OnMouseWheelBackward()
{
  if (this->HasObserver(vtkCommand::MouseWheelBackwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
  }
}
