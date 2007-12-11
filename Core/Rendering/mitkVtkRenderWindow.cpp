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


#include "mitkVtkRenderWindow.h"
#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

//----------------------------------------------------------------------------
mitk::VtkRenderWindow* mitk::VtkRenderWindow::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("mitk::VtkRenderWindow");
  if(ret)
  {
    return (mitk::VtkRenderWindow*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new mitk::VtkRenderWindow;
}

// Construct object so that light follows camera motion.
mitk::VtkRenderWindow::VtkRenderWindow() : m_MitkRenderer(NULL)
{
  #ifndef VTK_USE_COCOA
    OwnWindow = 0; // do initializition missing in vtk
  #endif
}

mitk::VtkRenderWindow::~VtkRenderWindow() 
{
}

void mitk::VtkRenderWindow::Render()
{
  if(m_MitkRenderer)
  {
    this->Start();//enables the OpenGL Context and calls MakeCurrent

    //Let the renderer search through the datatree and get its mappers. 
    //Then the mapper render the geometries
    m_MitkRenderer->Render();
  }
  else
    itkExceptionMacro("MitkRenderer not set.");
}

void mitk::VtkRenderWindow::SetSize(int x, int y)
{
  //Fix for vtkSizeBug: when Mapped is true, native
  //routines would be used to resize the window -
  //Qt does not like this.
  bool mapped = this->Mapped;
  this->Mapped = 0;
  Superclass::SetSize(x, y);
  this->Mapped = mapped;
}

void mitk::VtkRenderWindow::SetPosition(int x, int y)
{
  //Fix for vtkSizeBug: when Mapped is true, native
  //routines would be used to re-position the window -
  //Qt does not like this.
  bool mapped = this->Mapped;
  this->Mapped = 0;
  Superclass::SetPosition(x, y);
  this->Mapped = mapped;
}
