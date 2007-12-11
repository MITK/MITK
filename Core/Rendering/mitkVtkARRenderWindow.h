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


#ifndef MITKVTKARRENDERWINDOW_H_HEADER_INCLUDED
#define MITKVTKARRENDERWINDOW_H_HEADER_INCLUDED

#include "mitkVtkRenderWindow.h"

namespace mitk
{
  //##Documentation
  //##@brief 
  //##@ingroup Rendering  
  //## This RenderWindow is configured to realize an augmented reality render window.
  class VtkARRenderWindow : public VtkRenderWindow
  {
  public:
    vtkTypeMacro(VtkARRenderWindow,VtkRenderWindow);
    itkTypeMacro(VtkARRenderWindow,VtkRenderWindow);

    static VtkARRenderWindow *New();
    //##Documentation
    //##@brief derived from vtkRenderWindow to only swap buffer if we are about to finish the rendering
    virtual void CopyResultFrame();

    itkSetMacro(FinishRendering, bool);

  protected:
    VtkARRenderWindow();
    virtual ~VtkARRenderWindow(){};

    //flag to sign if the renderprocess is about to finish and the buffers shall be swapped
    bool m_FinishRendering;
  };

}

#endif /* MITKVtkARRenderWindow_H_HEADER_INCLUDED */

