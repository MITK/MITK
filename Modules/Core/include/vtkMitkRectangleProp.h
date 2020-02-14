/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
