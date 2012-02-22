/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-07-22 16:41:18 +0200 (Fr, 17 Aug 2007) $
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

