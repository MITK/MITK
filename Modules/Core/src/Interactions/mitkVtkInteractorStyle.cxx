/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkInteractorStyle.h"

#include <vtkCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(mitkVtkInteractorStyle);

mitkVtkInteractorStyle::mitkVtkInteractorStyle() : vtkInteractorStyleUser()
{
}

mitkVtkInteractorStyle::~mitkVtkInteractorStyle()
{
}

void mitkVtkInteractorStyle::OnMouseWheelForward()
{
  if (this->HasObserver(vtkCommand::MouseWheelForwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
  }
}

void mitkVtkInteractorStyle::OnMouseWheelBackward()
{
  if (this->HasObserver(vtkCommand::MouseWheelBackwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
  }
}
