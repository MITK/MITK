/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestPoints_h
#define mitkRestPoints_h

#include <MitkRESTAPIExports.h>

#include <mitkPoint.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief The render window of the standard display that an id of the API
   *        names: "axial", "sagittal", "coronal" or "3d".
   *
   * Runs in the GUI thread.
   *
   * \throw RestException with status 404 for an unknown id or a window that
   *        does not exist.
   */
  MITKRESTAPI_EXPORT BaseRenderer* GetRestRenderer(const std::string& windowId, const std::string& path);

  /**
   * \brief The points that a request body gives, in world coordinates.
   *
   * A body gives points in one of two ways:
   * - "points": [[x, y, z], ...] in world coordinates (millimeters);
   * - "pixels": [[x, y], ...] in a screenshot of a 2D render window, with
   *   "window", the window's id, and "image_size": [width, height], the size
   *   of the screenshot. The origin is the top left corner of the screenshot.
   *   Pixels map onto the plane the window shows now.
   *
   * Runs in the GUI thread.
   *
   * \param minCount The least number of points the body must give.
   * \throw RestException with status 400 for a body without valid points.
   */
  MITKRESTAPI_EXPORT std::vector<Point3D> GetRestWorldPoints(const nlohmann::json& body, const std::string& path, std::size_t minCount = 1);

  /** \brief A point as JSON array, rounded to micrometers. */
  MITKRESTAPI_EXPORT nlohmann::json ToRestJson(const Point3D& point);
}

#endif
