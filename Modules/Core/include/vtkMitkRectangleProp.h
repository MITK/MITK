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

#ifndef vtkMitkRectangleProp_h
#define vtkMitkRectangleProp_h

#include <MitkCoreExports.h>

#include <vtkActor2D.h>
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkViewport;

/**
 * @brief The vtkMitkRectangleProp class renders a rectangle into a renderwindow as a frame.
 */
class MITKCORE_EXPORT vtkMitkRectangleProp : public vtkActor2D
{
public:
  static vtkMitkRectangleProp *New();
  vtkTypeMacro(vtkMitkRectangleProp, vtkProp);

  void SetColor(float red, float green, float blue);
  void SetLineWidth(unsigned int lineWidth);
  int RenderOverlay(vtkViewport *viewport) override;

protected:
  vtkMitkRectangleProp();
  ~vtkMitkRectangleProp() override;

private:
  /**
   * @brief Internal helper to fill a vtkPolydata with a rectangle.
   */
  void CreateRectangle();
  void UpdateRectangle();

  int m_Height;
  int m_Width;
  int m_OriginX;
  int m_OriginY;

  vtkIdType m_BottomLeft;
  vtkIdType m_BottomRight;
  vtkIdType m_TopRight;
  vtkIdType m_TopLeft;

  /**
   * @brief Holds the rectangle.
   */
  vtkSmartPointer<vtkPolyData> m_PolyData;
};

#endif
