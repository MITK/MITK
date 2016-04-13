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

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkActor2D.h>

class vtkPolyData;

/**
 * @brief The vtkMitkRectangleProp2 class Renders a rectangle into a renderwindow as a frame.
 *
 * This class is a replacement for the deprecated vtkMitkRectangleProp, which
 * used to render the same effect with pure OpenGL.
 */
class MITKCORE_EXPORT vtkMitkRectangleProp : public vtkActor2D
{
public:
  static vtkMitkRectangleProp* New();
  vtkTypeMacro(vtkMitkRectangleProp,vtkProp);

  /**
     * @brief SetColor Set the color of the rectangle.
     * @param col1 red
     * @param col2 green
     * @param col3 blue
     */
  void SetColor(float col1, float col2, float col3);

  void SetLineWidth(unsigned int lineWidth);

  int RenderOverlay(vtkViewport *viewport);

protected:

  vtkMitkRectangleProp();
  virtual ~vtkMitkRectangleProp();

  int m_Height;
  int m_Width;
  int m_OriginX;
  int m_OriginY;
  vtkIdType m_BottomLeftR,
  m_BottomRightL,
  m_BottomLeftU,
  m_TopLeftD,
  m_TopLeftR,
  m_TopRightL,
  m_TopRightD,
  m_BottomRightU;

  /**
     * @brief CreateRectangle internal helper to fill a vtkPolydata with a rectangle.
     */
  void CreateRectangle();

  void UpdateRectangle();

  /**
     * @brief m_PolyData holds the rectangle.
     */
  vtkSmartPointer<vtkPolyData> m_PolyData;
};
#endif /* vtkMitkRectangleProp_h */
