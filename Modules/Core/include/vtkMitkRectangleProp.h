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
#include <vtkProp.h>

class vtkActor2D;
class vtkPolyData;

/**
 * @brief The vtkMitkRectangleProp2 class Renders a rectangle into a renderwindow as a frame.
 *
 * This class is a replacement for the deprecated vtkMitkRectangleProp, which
 * used to render the same effect with pure OpenGL.
 */
class MITKCORE_EXPORT vtkMitkRectangleProp : public vtkProp
{
public:
  static vtkMitkRectangleProp* New();
  vtkTypeMacro(vtkMitkRectangleProp,vtkProp);

  /**
     * @brief RenderOverlay Calls the render method of the actor and renders it.
     * @param viewport viewport of the renderwindow.
     * @return
     */
  int RenderOverlay(vtkViewport* viewport) override;

  /**
     * @brief SetRenderWindow Set the renderwindow.
     * @param renWin
     */
  void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renWin);

  /**
     * @brief SetColor Set the color of the rectangle.
     * @param col1 red
     * @param col2 green
     * @param col3 blue
     */
  void SetColor(float col1, float col2, float col3);

protected:

  vtkMitkRectangleProp();
  virtual ~vtkMitkRectangleProp();

  /**
     * @brief m_RenderWindow renderwindow to add the rectangle to.
     */
  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

  /**
     * @brief m_Color color of the rectangle.
     */
  float m_Color[3];

  /**
     * @brief CreateRectangle internal helper to fill a vtkPolydata with a rectangle.
     */
  void CreateRectangle();
  /**
     * @brief m_PolyData holds the rectangle.
     */
  vtkSmartPointer<vtkPolyData> m_PolyData;
  /**
     * @brief m_RectangleActor actor to render the rectangle.
     */
  vtkSmartPointer<vtkActor2D> m_RectangleActor;

  /**
     * @brief IDs of the corner points (the corners of the renderwindow).
     */
  vtkIdType m_BottomLeft, m_BottomRight, m_TopRight, m_TopLeft;
};
#endif /* vtkMitkRectangleProp_h */
