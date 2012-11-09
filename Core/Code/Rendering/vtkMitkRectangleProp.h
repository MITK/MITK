/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef vtkMitkRectangleProp_H_HEADER_INCLUDED_C1C53723
#define vtkMitkRectangleProp_H_HEADER_INCLUDED_C1C53723

#include "vtkActor2D.h"
#include "vtkRenderWindow.h"
#include <MitkExports.h>

class MITK_CORE_EXPORT vtkMitkRectangleProp : public vtkProp
{
  public:
    static vtkMitkRectangleProp* New();
    vtkTypeMacro(vtkMitkRectangleProp,vtkProp);

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


