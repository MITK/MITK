/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneClipping_h
#define mitkPlaneClipping_h

#include <mitkGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

namespace mitk
{
  /** \brief Utility functions for plane clipping calculations. */
  namespace PlaneClipping
  {
    /** \brief Internal helper method for intersection testing used only in CalculateClippedPlaneBounds().
     *
     * Tests whether the line segment between two points crosses the z=0 plane,
     * and if so, updates the bounds with the intersection coordinates.
     *
     * \param[in] points VTK point set containing the coordinates.
     * \param[in] p1 Index of the first point in the point set.
     * \param[in] p2 Index of the second point in the point set.
     * \param[in,out] bounds Array of 6 doubles [xmin, xmax, ymin, ymax, zmin, zmax]
     *                       updated with the intersection point if found.
     * \return True if an intersection with z=0 was found.
     */
    static bool LineIntersectZero(vtkPoints *points, int p1, int p2, double *bounds)
    {
      double point1[3];
      double point2[3];
      points->GetPoint(p1, point1);
      points->GetPoint(p2, point2);

      if ((point1[2] * point2[2] <= 0.0) && (point1[2] != point2[2]))
      {
        double x, y;
        x = (point1[0] * point2[2] - point1[2] * point2[0]) / (point2[2] - point1[2]);
        y = (point1[1] * point2[2] - point1[2] * point2[1]) / (point2[2] - point1[2]);

        if (x < bounds[0])
        {
          bounds[0] = x;
        }
        if (x > bounds[1])
        {
          bounds[1] = x;
        }
        if (y < bounds[2])
        {
          bounds[2] = y;
        }
        if (y > bounds[3])
        {
          bounds[3] = y;
        }
        bounds[4] = bounds[5] = 0.0;
        return true;
      }
      return false;
    }

    /** \brief Calculate the bounding box of the resliced image.
     *
     * This is necessary for arbitrarily rotated planes in an image volume.
     * A rotated plane (e.g. in swivel mode) will have a new bounding box,
     * which needs to be calculated by clipping the bounding geometry edges
     * against the plane.
     *
     * \param[in] boundingGeometry The geometry defining the volume bounding box.
     * \param[in] planeGeometry The plane geometry to clip against.
     * \param[out] bounds Array of 6 doubles receiving [xmin, xmax, ymin, ymax, zmin, zmax]
     *                    in physical coordinates (mm), adjusted by plane spacing.
     * \return True if valid bounds were computed, false if the plane does not intersect the volume.
     */
    static bool CalculateClippedPlaneBounds(const BaseGeometry *boundingGeometry,
                                            const PlaneGeometry *planeGeometry,
                                            double *bounds)
    {
      // Clip the plane with the bounding geometry. To do so, the corner points
      // of the bounding box are transformed by the inverse transformation
      // matrix, and the transformed bounding box edges derived therefrom are
      // clipped with the plane z=0. The resulting min/max values are taken as
      // bounds for the image reslicer.
      const mitk::BoundingBox *boundingBox = boundingGeometry->GetBoundingBox();

      mitk::BoundingBox::PointType bbMin = boundingBox->GetMinimum();
      mitk::BoundingBox::PointType bbMax = boundingBox->GetMaximum();

      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      if (boundingGeometry->GetImageGeometry())
      {
        points->InsertPoint(0, bbMin[0] - 0.5, bbMin[1] - 0.5, bbMin[2] - 0.5);
        points->InsertPoint(1, bbMin[0] - 0.5, bbMin[1] - 0.5, bbMax[2] - 0.5);
        points->InsertPoint(2, bbMin[0] - 0.5, bbMax[1] - 0.5, bbMax[2] - 0.5);
        points->InsertPoint(3, bbMin[0] - 0.5, bbMax[1] - 0.5, bbMin[2] - 0.5);
        points->InsertPoint(4, bbMax[0] - 0.5, bbMin[1] - 0.5, bbMin[2] - 0.5);
        points->InsertPoint(5, bbMax[0] - 0.5, bbMin[1] - 0.5, bbMax[2] - 0.5);
        points->InsertPoint(6, bbMax[0] - 0.5, bbMax[1] - 0.5, bbMax[2] - 0.5);
        points->InsertPoint(7, bbMax[0] - 0.5, bbMax[1] - 0.5, bbMin[2] - 0.5);
      }
      else
      {
        points->InsertPoint(0, bbMin[0], bbMin[1], bbMin[2]);
        points->InsertPoint(1, bbMin[0], bbMin[1], bbMax[2]);
        points->InsertPoint(2, bbMin[0], bbMax[1], bbMax[2]);
        points->InsertPoint(3, bbMin[0], bbMax[1], bbMin[2]);
        points->InsertPoint(4, bbMax[0], bbMin[1], bbMin[2]);
        points->InsertPoint(5, bbMax[0], bbMin[1], bbMax[2]);
        points->InsertPoint(6, bbMax[0], bbMax[1], bbMax[2]);
        points->InsertPoint(7, bbMax[0], bbMax[1], bbMin[2]);
      }

      vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();

      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      transform->Identity();
      transform->Concatenate(planeGeometry->GetVtkTransform()->GetLinearInverse());

      transform->Concatenate(boundingGeometry->GetVtkTransform());

      transform->TransformPoints(points, newPoints);

      bounds[0] = bounds[2] = 10000000.0;
      bounds[1] = bounds[3] = -10000000.0;
      bounds[4] = bounds[5] = 0.0;

      LineIntersectZero(newPoints, 0, 1, bounds);
      LineIntersectZero(newPoints, 1, 2, bounds);
      LineIntersectZero(newPoints, 2, 3, bounds);
      LineIntersectZero(newPoints, 3, 0, bounds);
      LineIntersectZero(newPoints, 0, 4, bounds);
      LineIntersectZero(newPoints, 1, 5, bounds);
      LineIntersectZero(newPoints, 2, 6, bounds);
      LineIntersectZero(newPoints, 3, 7, bounds);
      LineIntersectZero(newPoints, 4, 5, bounds);
      LineIntersectZero(newPoints, 5, 6, bounds);
      LineIntersectZero(newPoints, 6, 7, bounds);
      LineIntersectZero(newPoints, 7, 4, bounds);

      if ((bounds[0] > 9999999.0) || (bounds[2] > 9999999.0) || (bounds[1] < -9999999.0) || (bounds[3] < -9999999.0))
      {
        return false;
      }
      else
      {
        // The resulting bounds must be adjusted by the plane spacing, since we
        // we have so far dealt with index coordinates
        const mitk::Vector3D planeSpacing = planeGeometry->GetSpacing();
        bounds[0] *= planeSpacing[0];
        bounds[1] *= planeSpacing[0];
        bounds[2] *= planeSpacing[1];
        bounds[3] *= planeSpacing[1];
        bounds[4] *= planeSpacing[2];
        bounds[5] *= planeSpacing[2];
        return true;
      }
    }
  }
}

#endif
