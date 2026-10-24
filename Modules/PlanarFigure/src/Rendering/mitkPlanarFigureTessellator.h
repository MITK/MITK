/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureTessellator_h
#define mitkPlanarFigureTessellator_h

#include <mitkNumericTypes.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>

#include <array>
#include <vector>

namespace mitk
{
  /**
   * \brief Builds anti-aliased 2D shapes in display coordinates as triangles
   *        with per-vertex RGBA colors.
   *
   * Coverage is evaluated per vertex instead of per fragment: every shape gets
   * a feather band along its edges whose alpha falls to zero, and the
   * rasterizer interpolates it linearly. For straight edges this is the exact
   * coverage of a box filter of the feather width, so no multisampling and no
   * custom shader is needed. All lengths are in device pixels.
   *
   * Shapes are drawn in the order they are added, which is the order of the
   * triangles in the poly data. Only triangles are emitted, because poly data
   * mappers draw strips after all polygons.
   */
  class PlanarFigureTessellator
  {
  public:
    /** \brief RGBA, each component in [0, 1]. */
    using Color = std::array<double, 4>;

    PlanarFigureTessellator();

    /**
     * \brief Adds a polyline stroke with miter joins and butt caps.
     *
     * Joins sharper than the miter limit are beveled. A width below the
     * feather is drawn as wide as the feather with proportionally reduced
     * alpha, which keeps its integrated coverage.
     */
    void AddStroke(const std::vector<Point2D> &points, bool closed, double width, const Color &color, double feather = 1.0);

    /**
     * \brief Adds a stroke interrupted by gaps, measured along the polyline.
     *
     * Only the dashes that can touch the clip rectangle are added, so the cost
     * depends on the visible length, not on the length of the polyline. The
     * pattern stays anchored to the start of the polyline.
     */
    void AddDashedStroke(const std::vector<Point2D> &points, bool closed, double width, const Color &color, double dash, double gap,
      const Point2D &clipLowerLeft, const Point2D &clipUpperRight);

    /** \brief Adds a filled disc. */
    void AddDisc(const Point2D &center, double radius, const Color &color);

    /** \brief Adds a ring between two radii. */
    void AddRing(const Point2D &center, double innerRadius, double outerRadius, const Color &color);

    /** \brief Adds a filled rectangle with rounded corners, at least half a pixel round. */
    void AddRoundedRectangle(const Point2D &lowerLeft, const Point2D &upperRight, double cornerRadius, const Color &color);

    /** \brief The poly data the shapes are written into. */
    vtkPolyData *GetPolyData() const;

  private:
    struct Row
    {
      vtkIdType LeftOuter;
      vtkIdType LeftInner;
      vtkIdType RightInner;
      vtkIdType RightOuter;
    };

    vtkIdType AddVertex(const Point2D &position, const Color &color, double alpha);
    void AddTriangle(vtkIdType a, vtkIdType b, vtkIdType c);
    void AddQuad(vtkIdType a, vtkIdType b, vtkIdType c, vtkIdType d);
    void ConnectRows(const Row &from, const Row &to);
    Row AddRow(const Point2D &center, const Vector2D &normal, double innerOffset, double outerOffset, const Color &color, double alpha);
    void AddCircularBands(const Point2D &center, const std::vector<double> &radii, const std::vector<double> &alphas, const Color &color);

    vtkSmartPointer<vtkPolyData> m_PolyData;
    vtkSmartPointer<vtkPoints> m_Points;
    vtkSmartPointer<vtkUnsignedCharArray> m_Colors;
    vtkSmartPointer<vtkCellArray> m_Triangles;
  };
}

#endif
