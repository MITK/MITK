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


#ifndef MITKVTKSTENCILRENDERWINDOW_H_HEADER_INCLUDED
#define MITKVTKSTENCILRENDERWINDOW_H_HEADER_INCLUDED

#include "mitkVtkRenderWindow.h"

namespace mitk
{
  //##Documentation
  //##@brief Superclass of VtkRenderWindow classes configured for the 
  //## use with the OpenGL stencil buffer. 
  //##
  //## @note VTK has to be initialized according to the operating system,
  //## developers have to derive this class and care about the initialisation 
  //## of the OpenGL stencil buffer.
  //## By default stencil buffer is not supported by vtk nor by qt.
  //##@ingroup Rendering  
  class VtkStencilRenderWindow : public VtkRenderWindow
  {
  public:
    vtkTypeMacro(VtkStencilRenderWindow,VtkRenderWindow);
    itkTypeMacro(VtkStencilRenderWindow,VtkRenderWindow);

    //##Documentation
    //##@brief derived from vtkRenderWindow to only swap buffer if we are about to finish the rendering
    virtual void CopyResultFrame();

    itkSetMacro(FinishRendering, bool);

  protected:
    VtkStencilRenderWindow();
    virtual ~VtkStencilRenderWindow(){};

    //flag to sign if the renderprocess is about to finish and the buffers shall be swapped
    bool m_FinishRendering;
  };

}

#endif /* MITKVTKSTENCILRENDERWINDOW_H_HEADER_INCLUDED */

