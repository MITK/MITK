/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestRendering_h
#define mitkRestRendering_h

#include <MitkRESTAPIExports.h>

#include <mitkColorProperty.h>
#include <mitkCommon.h>
#include <mitkPoint.h>
#include <mitkRestRoute.h>
#include <mitkTimeGeometry.h>

#include <nlohmann/json.hpp>

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  class BaseRenderer;
  class DataNode;
  class Image;

  /**
   * \brief Converts between positions of the stepper and slice indices as the
   *        Image Navigator shows them, which follow the image index.
   *
   * Whether the two run opposite depends on the orientation of the image. The
   * conversion is its own inverse.
   *
   * \param count The number of slices of the renderer.
   * \param number A position of the stepper, or a slice index.
   */
  MITKRESTAPI_EXPORT int ConvertRestSliceNumber(BaseRenderer* renderer, int count, int number);

  /** \brief Whether the renderer shows 2D slices. */
  MITKRESTAPI_EXPORT bool IsRestRenderer2D(const BaseRenderer* renderer);

  /**
   * \brief A render window as GET /api/v1/rendering/windows describes it:
   *        {id, kind, size, timepoint_ms}, and for 2D windows
   *        view_direction and, if the window shows slices,
   *        slice: {index, count}.
   *
   * Runs in the GUI thread.
   */
  MITKRESTAPI_EXPORT nlohmann::json DescribeRestWindow(const std::string& id, BaseRenderer* renderer);

  /**
   * \brief Whether the crosshair shows in a 2D window and in the 3D window,
   *        as {visible, visible_3d}.
   *
   * The crosshair shows in a window if all of the planes are visible there; a
   * null plane hides it.
   *
   * Runs in the GUI thread.
   */
  MITKRESTAPI_EXPORT nlohmann::json DescribeRestCrosshair(
    const std::array<const DataNode*, 3>& planes, const BaseRenderer* window2D, const BaseRenderer* window3D);

  /**
   * \brief A query parameter that switches something on or off.
   *
   * \return The value of the parameter, or the default if the request has none.
   * \throw RestException with status 400 for a value other than true or false.
   */
  MITKRESTAPI_EXPORT bool GetRestQueryFlag(const RestRequest& request, const std::string& name, bool defaultValue);

  /**
   * \brief A query parameter that counts pixels: a whole number, which may be
   *        negative.
   *
   * \param name The name of the parameter, for the error.
   * \throw RestException with status 400 unless the whole value is a number
   *        that fits an int.
   */
  MITKRESTAPI_EXPORT int ParseRestPixelCount(const std::string& name, const std::string& value, const std::string& path);

  /**
   * \brief What the query of a screenshot request asks for.
   *
   * The defaults are those of a request without query.
   */
  struct MITKRESTAPI_EXPORT RestScreenshotOptions
  {
    enum class Background
    {
      /** The background the window shows. */
      AsShown,
      Transparent,
      /** The solid color of BackgroundColor. */
      Color
    };

    /** \brief The longest side of the image at most, padding included. */
    unsigned int MaxSize = 1000;

    Background BackgroundMode = Background::AsShown;

    /** \brief For Background::Color. */
    mitk::Color BackgroundColor = mitk::Color(0.0f);

    bool Decorations = true;

    /** \brief Image pixels per pixel of the window, 1 to 4. */
    unsigned int Magnification = 1;

    bool CropToContent = false;

    /** \brief Pixels of background around the content when cropping to it. */
    unsigned int Padding = 0;
  };

  /**
   * \brief The screenshot options of the query parameters max_size,
   *        background, decorations, scale, crop and padding.
   *
   * max_size is clamped to 100 to 4000.
   *
   * \throw RestException with status 400 for a parameter with a value that
   *        its description does not allow, and for padding without crop=true
   *        or one that leaves less than half of max_size.
   */
  MITKRESTAPI_EXPORT RestScreenshotOptions ParseRestScreenshotOptions(const RestRequest& request);

  /** \brief A line of the grid that a screenshot can carry. */
  struct MITKRESTAPI_EXPORT RestGridLine
  {
    /** \brief The pixel the line runs through, from the top left corner. */
    int Position = 0;

    /** \brief Whether the line is labeled with its position. */
    bool Labeled = false;

    bool operator==(const RestGridLine& other) const;
  };

  /**
   * \brief The lines of the grid for one side of an image: every 50 pixels
   *        from 50 on and inside the image, labeled every 100.
   *
   * \param length The size of the image in the direction that the lines
   *        cross.
   */
  MITKRESTAPI_EXPORT std::vector<RestGridLine> GetRestGridLines(int length);

  /**
   * \brief The time step of the data at a time point.
   *
   * \throw RestException with status 422 if the data has no time step there.
   */
  MITKRESTAPI_EXPORT TimeStepType GetRestTimeStep(const DataNode* node, TimePointType timePoint, const std::string& path);

  /**
   * \brief The center of the box around the points, in world coordinates.
   *
   * \pre The points are not empty.
   */
  MITKRESTAPI_EXPORT Point3D GetRestCenter(const std::vector<Point3D>& points);

  /**
   * \brief Shows the slice through the center of the corners and zooms so that
   *        they fill the window.
   *
   * \param margin The space around the corners, as a share of their size.
   *
   * Corners that coincide in the plane give no zoom: the scale stays, and only
   * the center moves. Runs in the GUI thread.
   *
   * \pre The corners are not empty.
   */
  MITKRESTAPI_EXPORT void FocusRestRenderer2D(BaseRenderer* renderer, const std::vector<Point3D>& corners, double margin);

  /**
   * \brief Keeps the direction of the camera and moves it so that the corners
   *        fill the window.
   *
   * \param margin The space around the corners, as a share of their size.
   *
   * Corners that coincide give neither a scale nor a distance, so the camera
   * keeps its own. Runs in the GUI thread.
   *
   * \pre The corners are not empty.
   */
  MITKRESTAPI_EXPORT void FocusRestRenderer3D(BaseRenderer* renderer, const std::vector<Point3D>& corners, double margin);

  /**
   * \brief The gray value of the first component of the image at a point.
   *
   * \return No value if the time point is not covered by the image, the image
   *         has fewer than three dimensions or the point lies outside of the
   *         image at that time point.
   */
  MITKRESTAPI_EXPORT std::optional<double> GetRestImageValue(const Image* image, const Point3D& point, TimePointType timePoint);
}

#endif
