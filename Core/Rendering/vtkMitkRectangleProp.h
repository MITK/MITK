/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2006-05-04 16:45:18 +0200 (Do, 04 Mai 2006) $
Version:   $Revision: 6790 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef vtkMitkRectangleProp_H_HEADER_INCLUDED_C1C53723
#define vtkMitkRectangleProp_H_HEADER_INCLUDED_C1C53723

#include "vtkActor2D.h"
#include "vtkRenderWindow.h"

class vtkMitkRectangleProp : public vtkProp
{
  public:
    
    static vtkMitkRectangleProp* New();

    int RenderOpaqueGeometry(vtkViewport* viewport);
    int RenderTranslucentGeometry(vtkViewport* viewport);
    int RenderOverlay(vtkViewport* viewport);

    void SetRenderWindow(vtkRenderWindow* renWin);

    void SetColor(float col1, float col2, float col3);

    double* GetBounds();

  protected:
    
    vtkMitkRectangleProp();
    virtual ~vtkMitkRectangleProp();

    void Enable2DOpenGL();
    void Disable2DOpenGL();

    vtkRenderWindow* m_RenderWindow;
    float m_Color[3];
};


#endif /* vtkMitkRectangleProp_H_HEADER_INCLUDED_C1C53723 */

