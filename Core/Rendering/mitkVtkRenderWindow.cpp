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
mitk::VtkRenderWindow::VtkRenderWindow() : m_MitkRenderer(NULL), m_FinishRendering(false)
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
    
    //enable OpenGL Stencil-Test
    glEnable(GL_STENCIL_TEST);
    glClearStencil(1); //reference for deletion
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(1); 

    //Let the renderer search through the datatree and get its mappers. 
    //Then the mapper render the geometries
    m_MitkRenderer->Render();

    //Disable stencil test
    glDisable(GL_STENCIL_TEST);
  }
  else
    itkExceptionMacro("MitkRenderer not set.");
}

//derived to only call swap buffer if we are about to finish the render process
void mitk::VtkRenderWindow::CopyResultFrame()
{
  //if we are not about to finish rendering, then we don't swap buffers!
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


//derived to enable Stencil Buffer in pfd
/*
 * void mitk::VtkRenderWindow::SetupPixelFormat(HDC hDC, DWORD dwFlags, 
                                                  int debug, int bpp, 
                                                  int zbpp)
{
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  //gc size 
    1,                              //gc version 
    dwFlags         ,               //gc support double-buffering 
    PFD_TYPE_RGBA,                  //gc color type 
    bpp,                             //gc prefered color depth 
    0, 0, 0, 0, 0, 0,               //gc color bits (ignored) 
    0,                              //gc no alpha buffer 
    0,                              //gc alpha bits (ignored) 
    0,                              //gc no accumulation buffer 
    0, 0, 0, 0,                     //gc accum bits (ignored) 
    zbpp,                           //gc depth buffer 
    1,                              //gc stencil buffer enabled!!!
    0,                              //gc no auxiliary buffers 
    PFD_MAIN_PLANE,                 //gc main layer 
    0,                              //gc reserved 
    0, 0, 0,                        //gc no layer, visible, damage masks 
  };
  int pixelFormat;
  // Only try to set pixel format if we do not currently have one
  int currentPixelFormat = GetPixelFormat(hDC);
  // if there is a current pixel format, then make sure it
  // supports OpenGL
  if (currentPixelFormat != 0)
    {
      DescribePixelFormat(hDC, currentPixelFormat,sizeof(pfd), &pfd);
      if (!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
        {
          MessageBox(WindowFromDC(hDC), 
                     "Invalid pixel format, no OpenGL support",
                     "Error",
                     MB_ICONERROR | MB_OK);
          exit(1);
        }         
    }
  else
    {
      // hDC has no current PixelFormat, so 
      pixelFormat = ChoosePixelFormat(hDC, &pfd);
      if (pixelFormat == 0)
        {
          MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
                     MB_ICONERROR | MB_OK);
          exit(1);
        }
      DescribePixelFormat(hDC, pixelFormat,sizeof(pfd), &pfd); 
      if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) 
        {
          // int err = GetLastError();
          MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
                     MB_ICONERROR | MB_OK);
          exit(1);
        }
    }
  if (debug && (dwFlags & PFD_STEREO) && !(pfd.dwFlags & PFD_STEREO))
    {
      vtkGenericWarningMacro("No Stereo Available!");
      this->StereoCapableWindow = 0;
    }
}
*/
