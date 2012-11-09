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

#include "mitkVtkInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkCommand.h>


vtkCxxRevisionMacro(mitkVtkInteractorStyle, "$Revision: 1.35 $");
vtkStandardNewMacro(mitkVtkInteractorStyle);

mitkVtkInteractorStyle::mitkVtkInteractorStyle()
: vtkInteractorStyleUser()
{
}

mitkVtkInteractorStyle::~mitkVtkInteractorStyle()
{
}

void mitkVtkInteractorStyle::OnMouseWheelForward()
{
  if (this->HasObserver(vtkCommand::MouseWheelForwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, NULL);
  }
}

void mitkVtkInteractorStyle::OnMouseWheelBackward()
{
  if (this->HasObserver(vtkCommand::MouseWheelBackwardEvent))
  {
    this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, NULL);
  }
}

