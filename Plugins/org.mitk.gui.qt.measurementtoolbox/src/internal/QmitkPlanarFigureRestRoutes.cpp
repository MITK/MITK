/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPlanarFigureRestRoutes.h"

#include <QmitkPulseTimers.h>

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkIRestServerService.h>
#include <mitkPlanarFigure.h>
#include <mitkRenderingManager.h>
#include <mitkRestFigure.h>
#include <mitkRestGuiThread.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>

#include <QPointer>

#include <array>
#include <cmath>

using mitk::CallInGuiThread;
using mitk::ErrorResponse;
using mitk::RestException;
using mitk::RestRequest;
using mitk::RestResponse;
using mitk::RestRoute;

namespace
{
  [[noreturn]] void ThrowInvalid(const std::string& detail, const std::string& path)
  {
    throw RestException(ErrorResponse::InvalidRequest(detail, path));
  }
}

std::vector<RestRoute> CreatePlanarFigureRoutes(mitk::IRestServerService* server, QObject* owner)
{
  // Only used in the GUI thread, where it lives.
  const QPointer<QmitkPulseTimers> pulseTimers = new QmitkPulseTimers(owner);

  std::vector<RestRoute> routes;

  RestRoute list;
  list.Method = "GET";
  list.Path = "/planar-figures";
  list.Summary = "List the planar figures, the figures of the Measurement view";
  list.Description =
    "Including the figures the user drew. Returns {data: [{uid, name, type, image, closed, points, "
    "measurements, color, line_width, label, show_in_3d, pulse}]}: type like line, circle, rectangle, ellipse, "
    "polygon or angle; image, the uid of the image the figure marks, its parent in the Data Manager, or null; closed, "
    "whether it encloses a region; points, its control points in millimeters (of a circle its center and a point on "
    "it, of a rectangle two opposite corners); measurements: [{name, value, unit}] as the Measurement view shows "
    "them, like Length, Radius, Diameter, Circumference, Area or Angle, in mm, mm2 and deg; the look as PATCH "
    "/api/v1/planar-figures/{uid} changes it. GET /api/v1/statistics?mask=<uid> measures the gray values of the "
    "image in or along a figure.";
  list.Handler = [server](const RestRequest& request) {
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figures = CallInGuiThread(*server, [&]() {
      std::vector<mitk::RestFigureDescription> descriptions;
      const auto nodes = dataStorage->GetSubset(mitk::GetRestFigurePredicate());

      for (const auto& node : *nodes)
      {
        if (static_cast<const mitk::PlanarFigure*>(node->GetData())->IsComplete())
          descriptions.push_back(mitk::DescribeRestFigure(*server, dataStorage, node));
      }

      return descriptions;
    });

    auto json = nlohmann::json::array();

    for (const auto& figure : figures)
      json.push_back(mitk::ToRestJson(*server, figure));

    return RestResponse::FromData(json);
  };
  routes.push_back(list);

  RestRoute create;
  create.Method = "POST";
  create.Path = "/planar-figures";
  create.Summary = "Draw a line, circle or rectangle in a 2D window, as the Measurement view does";
  create.Description =
    "The figure lies in the plane of the slice that the window shows now. The points must lie in that plane and "
    "are taken as given, also beyond the extent of the plane. Body: "
    "{type: line, circle or rectangle; image: the uid of the image the figure marks, which becomes its parent in "
    "the Data Manager; window: axial, sagittal or coronal; two points, either pixels: [[x, y], [x, y]] of a "
    "screenshot of the window with image_size and view_state, as GET /api/v1/rendering/windows/{id}/screenshot "
    "describes it (409 VIEW_CHANGED if the view has changed since: take a new screenshot), or points: [[x, y, z], [x, y, z]] in millimeters in the slice the window shows; name (optional), Circle, "
    "Circle 2 and so on without it; and optionally the look fields of PATCH /api/v1/planar-figures/{uid}}. A line "
    "takes its two ends, a circle its center and a point on it, a rectangle two opposite corners. Returns the figure "
    "as GET /api/v1/planar-figures lists it, with status 201. Status 422 if image is no image or a point lies off "
    "the slice. DELETE /api/v1/datastorage/nodes/{uid} removes a figure.";
  create.Handler = [server, pulseTimers](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto type = body.value("type", std::string());

    if (!body.contains("image") || !body["image"].is_string())
      ThrowInvalid("\"image\", the uid of the image that the figure marks, is missing.", request.Path);

    if (!body.contains("window") || !body["window"].is_string())
      ThrowInvalid("\"window\", the 2D window in whose slice to draw, is missing.", request.Path);

    const auto lookChange = mitk::ParseRestFigureLookChange(body, request.Path);
    const auto window = body["window"].get<std::string>();
    const auto imageNode = mitk::FindRestNode(*server, body["image"], request.Path);

    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figure = CallInGuiThread(*server, [&]() {
      auto planarFigure = mitk::CreateRestFigure(type);

      if (planarFigure.IsNull())
        ThrowInvalid("\"type\" must be line, circle or rectangle.", request.Path);

      if (dynamic_cast<const mitk::Image*>(imageNode->GetData()) == nullptr)
        mitk::ThrowRestError("NOT_AN_IMAGE", "Unprocessable Content", "\"" + imageNode->GetName() + "\" is no image.", 422, request.Path);

      const auto* renderer = mitk::GetRestRenderer(window, request.Path);

      if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D)
        ThrowInvalid("Figures lie in the slice of a 2D window, not in the 3D window.", request.Path);

      const auto* plane = renderer->GetCurrentWorldPlaneGeometry();
      const auto points = mitk::GetRestWorldPoints(body, request.Path, 2);

      if (points.size() != 2)
      {
        ThrowInvalid("Give two points: the ends of a line, the center of a circle and a point on it, or two opposite "
          "corners of a rectangle.", request.Path);
      }

      // Points from elsewhere, like a pick, may lie anywhere within the slice, up to half its thickness off its plane.
      const auto halfThickness = plane->GetExtentInMM(2) / 2.0;
      std::array<mitk::Point2D, 2> planePoints;

      for (std::size_t i = 0; i < points.size(); ++i)
      {
        if (plane->Distance(points[i]) > halfThickness)
        {
          mitk::ThrowRestError("POINT_OFF_SLICE", "Unprocessable Content", "A point lies off the slice that the " + window +
            " window shows. Show the slice through the points first, for example with PUT "
            "/api/v1/rendering/selected-position, or give pixels of a screenshot.", 422, request.Path);
        }

        plane->Map(points[i], planePoints[i]);
      }

      const auto offset = planePoints[1] - planePoints[0];

      const bool isEmpty = type == "rectangle"
        ? (std::abs(offset[0]) < mitk::eps || std::abs(offset[1]) < mitk::eps)
        : offset.GetNorm() < mitk::eps;

      if (isEmpty)
        ThrowInvalid("The two points span no figure: they coincide or, for a rectangle, lie in one row or column.", request.Path);

      // As the planar figure interactor spans a figure drawn with the mouse,
      // from where the mouse is pressed to where it is released.
      planarFigure->SetPlaneGeometry(plane);
      planarFigure->SetFromSpan(planePoints[0], planePoints[1]);

      const auto name = body.value("name", std::string());

      auto node = mitk::DataNode::New();
      node->SetName(name.empty() ? mitk::CreateRestFigureName(dataStorage, type) : name);
      node->SetData(planarFigure);
      mitk::ApplyRestFigureLookChange(node, lookChange, pulseTimers);

      dataStorage->Add(node, imageNode.get());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return mitk::DescribeRestFigure(*server, dataStorage, node);
    });

    return RestResponse::FromData(mitk::ToRestJson(*server, figure), 201);
  };
  routes.push_back(create);

  RestRoute change;
  change.Method = "PATCH";
  change.Path = "/planar-figures/:uid";
  change.Summary = "Change the look of a planar figure: color, line width, label, display in 3D and pulse";
  change.Description =
    "Path parameter: uid of a figure as GET /api/v1/planar-figures lists it. Body, all optional: {color: "
    "\"#rrggbb\"; line_width: pixels from 1 to 10; label: {show: when the label with the name and measurements shows, "
    "always, on_hover, when_selected, on_hover_or_when_selected, on_hover_while_selected or never; name and "
    "measurements: whether the label has them}; show_in_3d: whether the 3D window shows the figure too; pulse: "
    "whether the figure grows and shrinks to draw attention to it; pulse_seconds, with pulse true: the seconds after "
    "which the pulse stops by itself, up to 600; without it, the figure pulses until pulse is false}. Other fields "
    "are refused, and an invalid field changes nothing. The render windows show the change right away. Returns the "
    "figure as GET /api/v1/planar-figures lists it. Status 422 if uid is no planar figure of the Measurement view, or one that is still being drawn.";
  change.Handler = [server, pulseTimers](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.is_object())
      ThrowInvalid("The body must be an object with the fields to change.", request.Path);

    for (const auto& item : body.items())
    {
      if (!mitk::IsRestFigureLookField(item.key()))
      {
        ThrowInvalid("\"" + item.key() + "\" is no field of the look. The fields are color, line_width, label, "
          "show_in_3d, pulse and pulse_seconds.", request.Path);
      }
    }

    const auto lookChange = mitk::ParseRestFigureLookChange(body, request.Path);
    const auto& uid = request.PathParams.at("uid");
    const auto node = mitk::FindRestNode(*server, uid, request.Path);

    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figure = CallInGuiThread(*server, [&]() {
      const auto* planarFigure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());

      if (planarFigure == nullptr || !planarFigure->IsComplete() || !mitk::GetRestFigurePredicate()->CheckNode(node.get()))
      {
        mitk::ThrowRestError("NOT_A_PLANAR_FIGURE", "Unprocessable Content", "\"" + node->GetName() + "\" is no planar figure of "
          "the Measurement view, or is still being drawn.", 422, request.Path);
      }

      mitk::ApplyRestFigureLookChange(node.get(), lookChange, pulseTimers);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return mitk::DescribeRestFigure(*server, dataStorage, node.get());
    });

    return RestResponse::FromData(mitk::ToRestJson(*server, figure));
  };
  routes.push_back(change);

  return routes;
}
