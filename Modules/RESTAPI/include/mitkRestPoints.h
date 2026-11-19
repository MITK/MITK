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
   * \brief The ids of the render windows of the standard display, in the
   *        order of its renderers: "axial", "sagittal", "coronal", "3d".
   */
  MITKRESTAPI_EXPORT const std::vector<std::string>& GetRestWindowIds();

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
   *   "window", the window's id, "image_size": [width, height], the size
   *   of the screenshot, and "view_state", the view state the screenshot
   *   reports. The origin is the top left corner of the screenshot. Pixels map
   *   onto the plane the window shows now, which is why the view state must
   *   still be the window's.
   *
   * Runs in the GUI thread.
   *
   * \param minCount The least number of points the body must give.
   * \throw RestException with status 400 for a body without valid points or
   *        pixels without "view_state", and with status 409 and the code
   *        VIEW_CHANGED for pixels of a view that the window no longer shows.
   */
  MITKRESTAPI_EXPORT std::vector<Point3D> GetRestWorldPoints(const nlohmann::json& body, const std::string& path, std::size_t minCount = 1);

  /**
   * \brief A fingerprint of everything that changes how the pixels of a
   *        renderer map onto the world.
   *
   * It covers the camera, the world plane the renderer shows, the time point
   * and the window size. Doubles are rounded to a millionth, so that an
   * unchanged view always gives the same string.
   *
   * Runs in the GUI thread.
   *
   * \return Hexadecimal digits, valid in this process only.
   */
  MITKRESTAPI_EXPORT std::string GetRestViewState(const BaseRenderer* renderer);

  /** \brief A point as JSON array, rounded to micrometers. */
  MITKRESTAPI_EXPORT nlohmann::json ToRestJson(const Point3D& point);
}

#endif
