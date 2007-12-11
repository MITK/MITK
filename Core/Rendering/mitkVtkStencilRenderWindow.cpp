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

#include "mitkVtkStencilRenderWindow.h"


mitk::VtkStencilRenderWindow::VtkStencilRenderWindow()
: mitk::VtkRenderWindow(), m_FinishRendering(true)
{
}

//derived to only call swap buffer if we are about to finish the render process
//done to be able to do augmented reality. First render vtk Objects, then the mitk Objects and then swap buffers.
void mitk::VtkStencilRenderWindow::CopyResultFrame()
{
  //if we are not about to finish rendering, then we don't swap buffers!
  if (!m_FinishRendering)
    return;

  if (this->ResultFrame)
    {
    int *size;
    size = this->GetSize();
    this->SetPixelData(0,0,size[0]-1,size[1]-1,this->ResultFrame,!this->DoubleBuffer);
    }

  this->Frame();
}

