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


#ifndef MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722

#include "mitkBaseRenderer.h"
#include <stdlib.h>
#ifdef WIN32
  #include <vtkWin32OpenGLRenderWindow.h>
#else
  #include <vtkXOpenGLRenderWindow.h>
#endif

namespace mitk
{

#ifdef WIN32
  #define PARENTCLASS vtkWin32OpenGLRenderWindow
#else
  #define PARENTCLASS vtkXOpenGLRenderWindow
#endif

class VtkRenderWindow : public PARENTCLASS
#undef PARENTCLASS
{
public:
  // Description:
  // Construct object so that light follows camera motion.
  static VtkRenderWindow *New();

  vtkTypeMacro(VtkRenderWindow,vtkRenderWindow);

  itkTypeMacro(VtkRenderWindow,vtkRenderWindow);

  itkSetObjectMacro(MitkRenderer, BaseRenderer);
  itkGetObjectMacro(MitkRenderer, BaseRenderer);

  itkSetMacro(FinishRendering, bool);


  virtual void Render();

  //##Documentation
  //##@brief call the render process from vtk
  virtual void MitkRender(){vtkRenderWindow::Render();};

  //##Documentation
  //##@brief derived from vtkRenderWindow to only swap buffer if we are about to finish the rendering
  virtual void CopyResultFrame();

  // Description:
  // Initialize OpenGL for this window with Stencil Buffer enabled
  // virtual void SetupPixelFormat(HDC hDC, DWORD dwFlags, int debug, 
  //                              int bpp=16, int zbpp=16);

protected:
  VtkRenderWindow();
  virtual ~VtkRenderWindow();

  itk::WeakPointer<BaseRenderer> m_MitkRenderer;

  //flag to sign if the renderprocess is about to finish and the buffers shall be swapped
  bool m_FinishRendering;

private:
  VtkRenderWindow(const VtkRenderWindow&);  // Not implemented.
  void operator=(const VtkRenderWindow&);  // Not implemented.
};

}
#endif /* MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722 */

