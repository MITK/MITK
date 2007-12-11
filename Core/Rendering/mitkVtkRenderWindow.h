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


#ifndef MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722
#define MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722

#include "mitkBaseRenderer.h"
#include <stdlib.h>
#include <vtkConfigure.h>
#ifdef WIN32
  #include <vtkWin32OpenGLRenderWindow.h>
#else
  #ifdef VTK_USE_COCOA
    #include "vtkCocoaRenderWindow.h"
    #include "vtkCocoaRenderWindowInteractor.h"
  #else
    #ifdef VTK_USE_CARBON
      #include "vtkCarbonRenderWindow.h"
      #include "vtkCarbonRenderWindowInteractor.h"
    #else
      #include <vtkXOpenGLRenderWindow.h>
    #endif
  #endif
#endif

namespace mitk
{

#ifdef WIN32
  #define PARENTCLASS vtkWin32OpenGLRenderWindow
#else
  #ifdef VTK_USE_COCOA
    #define PARENTCLASS vtkCocoaRenderWindow
  #else
    #ifdef VTK_USE_CARBON
      #define PARENTCLASS vtkCarbonRenderWindow
    #else 
      #define PARENTCLASS vtkXOpenGLRenderWindow
    #endif  
  #endif
#endif

class VtkRenderWindow : public PARENTCLASS
{
public:
  // Description:
  // Construct object so that light follows camera motion.
  static VtkRenderWindow *New();

  vtkTypeMacro(VtkRenderWindow,PARENTCLASS);

  itkTypeMacro(VtkRenderWindow,PARENTCLASS);
#undef PARENTCLASS

  itkSetObjectMacro(MitkRenderer, BaseRenderer);
  itkGetObjectMacro(MitkRenderer, BaseRenderer);

  virtual void Render();

  //##Documentation
  //##@brief call the render process from vtk
  virtual void MitkRender(){vtkRenderWindow::Render();};

  virtual void SetSize(int,int); //Fix for vtkSizeBug
  virtual void SetPosition(int,int); //Fix for vtkSizeBug

protected:
  VtkRenderWindow();
  virtual ~VtkRenderWindow();

  itk::WeakPointer<BaseRenderer> m_MitkRenderer;

private:
  VtkRenderWindow(const VtkRenderWindow&);  // Not implemented.
  void operator=(const VtkRenderWindow&);  // Not implemented.
};

}
#endif /* MITKVTKRENDERWINDOW_H_HEADER_INCLUDED_C1C53722 */

