/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
mitk::VtkRenderWindow::VtkRenderWindow() : m_MitkRenderer(NULL), m_FinishRendering(true)
{
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

//derived to only call swap buffer if we are about to finish the render process
//done to be able to use stencil buffer. First render vtk Objects, then the mitk Objects and then swap buffers.
void mitk::VtkRenderWindow::CopyResultFrame()
{
  //if we are not about to finish rendering, then we don't swap buffers!
  //due to this, stereo doesn't work anymore!?!
  if (!m_FinishRendering)
    return;

  if (this->ResultFrame)
    {
    int *size;

    // get the size
    size = this->GetSize();
    this->SetPixelData(0,0,size[0]-1,size[1]-1,this->ResultFrame,!this->DoubleBuffer);
    }

  this->Frame();
}
