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

#include "mitkVtkStencilRenderWindow.h"


mitk::VtkStencilRenderWindow::VtkStencilRenderWindow()
: mitk::VtkRenderWindow(), m_FinishRendering(true)
{
}

//derived to be able to clear the stencil buffer before rendering
void mitk::VtkStencilRenderWindow::Render()
{
  if(m_MitkRenderer)
  {
    this->Start();//enables the OpenGL Context and calls MakeCurrent

    //check how much bits the stencil buffer supports
    GLint stencilBits = -1;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    if (stencilBits < 1)
      itkWarningMacro("No Stencil Buffer available! Please initialize VTK and your GUI (QT) to support stencil buffer in OpenGL.");

    //setting clearing parameter for stencil test if needed
    glClearStencil(1); 
    glStencilMask(1);
    glClear(GL_STENCIL_BUFFER_BIT); //costs performance

    //Let the renderer search through the datatree and get its mappers. 
    //Then the mapper render the geometries
    m_MitkRenderer->Render();
  }
  else
    itkExceptionMacro("MitkRenderer not set.");
}


//derived to only call swap buffer if we are about to finish the render process
//done to be able to use stencil buffer. First render vtk Objects, then the mitk Objects and then swap buffers.
void mitk::VtkStencilRenderWindow::CopyResultFrame()
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

