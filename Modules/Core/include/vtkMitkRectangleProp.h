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
 * \brief Renders a colored rectangle border into a vtkRenderWindow.
 *
 * This class draws a rectangular frame (border) into the render window overlay.
 * The rectangle automatically adjusts its size to match the viewport dimensions.
 *
 * \ingroup Rendering
 */
class MITKCORE_EXPORT vtkMitkRectangleProp : public vtkActor2D
{
public:
  /** \brief Create a new instance of vtkMitkRectangleProp. */
  static vtkMitkRectangleProp *New();
  vtkTypeMacro(vtkMitkRectangleProp, vtkProp);

  /**
   * \brief Set the color of the rectangle border.
   *
   * \param[in] red    Red component (0.0 to 1.0).
   * \param[in] green  Green component (0.0 to 1.0).
   * \param[in] blue   Blue component (0.0 to 1.0).
   */
  void SetColor(float red, float green, float blue);

  /**
   * \brief Set the line width of the rectangle border.
   *
   * \param[in] lineWidth  The line width in pixels.
   */
  void SetLineWidth(unsigned int lineWidth);

  /**
   * \brief Render the rectangle as an overlay.
   *
   * Updates the rectangle geometry if the viewport size has changed,
   * then renders the rectangle.
   *
   * \param[in] viewport  The viewport to render into.
   * \return 1 on success, 0 if the mapper is not set or the prop is not visible.
   */
  int RenderOverlay(vtkViewport *viewport) override;

protected:
  /** \brief Constructor. Creates the rectangle poly data and mapper. */
  vtkMitkRectangleProp();

  /** \brief Destructor. */
  ~vtkMitkRectangleProp() override;

private:
  /** \brief Initialize the rectangle poly data with four corner points and lines. */
  void CreateRectangle();

  /** \brief Update the rectangle corner positions to match the current viewport dimensions. */
  void UpdateRectangle();

  int m_Height;
  int m_Width;
  int m_OriginX;
  int m_OriginY;

  vtkIdType m_BottomLeft;
  vtkIdType m_BottomRight;
  vtkIdType m_TopRight;
  vtkIdType m_TopLeft;

  /** \brief The poly data containing the rectangle geometry. */
  vtkSmartPointer<vtkPolyData> m_PolyData;
};

#endif
