/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPlanarFigureRestRoutes.h"

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkIRestServerService.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkRenderingManager.h>
#include <mitkRestGuiThread.h>
#include <mitkRestPoints.h>

#include <array>
#include <cctype>
#include <cmath>

using mitk::CallInGuiThread;
using mitk::ErrorResponse;
using mitk::RestException;
using mitk::RestRequest;
using mitk::RestResponse;
using mitk::RestRoute;

namespace
{
  [[noreturn]] void Throw(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
  {
    throw RestException(ErrorResponse::Create(code, title, detail, status, path));
  }

  [[noreturn]] void ThrowInvalid(const std::string& detail, const std::string& path)
  {
    throw RestException(ErrorResponse::InvalidRequest(detail, path));
  }

  /** The figures of the Measurement view: those of helper objects and contour markers belong to other components. */
  mitk::NodePredicateBase::Pointer GetFigurePredicate()
  {
    auto isHelperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
    auto isContourMarker = mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true));

    return mitk::NodePredicateAnd::New(
      mitk::TNodePredicateDataType<mitk::PlanarFigure>::New(),
      mitk::NodePredicateNot::New(isHelperObject),
      mitk::NodePredicateNot::New(isContourMarker)).GetPointer();
  }

  /** A figure of the types the API draws, or null for any other type. */
  mitk::PlanarFigure::Pointer CreateFigure(const std::string& type)
  {
    if (type == "line" || type == "circle" || type == "rectangle")
      return mitk::CreatePlanarFigure(type);

    return nullptr;
  }

  /** A name like the Measurement view gives, "Circle", "Circle 2", ..., that no data has yet. */
  std::string CreateName(const mitk::DataStorage* dataStorage, const std::string& type)
  {
    auto baseName = type;
    baseName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(baseName[0])));

    return dataStorage->GetUniqueName(baseName);
  }

  /**
   * A figure as the API describes it. The uids are left to ToJson(), as only the
   * handler's thread may look them up.
   */
  struct FigureDescription
  {
    mitk::DataNode::Pointer Node;

    /** The image that the figure marks, its parent in the Data Manager. */
    mitk::DataNode::Pointer Image;

    nlohmann::json Json;
  };

  /** Runs in the GUI thread. */
  FigureDescription DescribeFigure(const mitk::DataStorage* dataStorage, mitk::DataNode* node)
  {
    const auto* figure = static_cast<const mitk::PlanarFigure*>(node->GetData());

    auto points = nlohmann::json::array();

    for (std::size_t i = 0; i < figure->GetNumberOfControlPoints(); ++i)
      points.push_back(mitk::ToRestJson(figure->GetWorldControlPoint(i)));

    auto measurements = nlohmann::json::array();

    for (const auto& measurement : figure->GetMeasurements())
    {
      measurements.push_back({
        { "name", mitk::PlanarFigure::GetDisplayName(measurement.Type) },
        { "value", measurement.Value },
        { "unit", mitk::PlanarFigure::GetUnitSymbol(measurement.Type) } });
    }

    const auto images = dataStorage->GetSources(node, mitk::TNodePredicateDataType<mitk::Image>::New(), true);

    return {
      node,
      images->empty() ? mitk::DataNode::Pointer() : images->front(),
      {
        { "name", node->GetName() },
        { "type", figure->GetTypeId() },
        { "closed", figure->IsClosed() },
        { "points", points },
        { "measurements", measurements }
      }
    };
  }

  nlohmann::json ToJson(const mitk::IRestServerService& server, const FigureDescription& figure)
  {
    auto json = figure.Json;
    json["uid"] = server.GetNodeUid(figure.Node);
    json["image"] = figure.Image.IsNotNull() ? nlohmann::json(server.GetNodeUid(figure.Image)) : nlohmann::json(nullptr);
    return json;
  }
}

std::vector<RestRoute> CreatePlanarFigureRoutes(mitk::IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute list;
  list.Method = "GET";
  list.Path = "/planar-figures";
  list.Summary = "List the planar figures, the figures of the Measurement view";
  list.Description =
    "Including the figures the user drew. Returns {data: [{uid, name, type, image, closed, points, "
    "measurements}]}: type like line, circle, rectangle, ellipse, polygon or angle; image, the uid of the image "
    "the figure marks, its parent in the Data Manager, or null; closed, whether it encloses a region; points, its "
    "control points in millimeters (of a circle its center and a point on it, of a rectangle two opposite corners); "
    "measurements: [{name, value, unit}] as the Measurement view shows them, like Length, Radius, Diameter, "
    "Circumference or Area, in mm and mm2. GET /api/v1/statistics?mask=<uid> measures the grey values of the "
    "image in or along a figure.";
  list.Handler = [server](const RestRequest& request) {
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figures = CallInGuiThread(*server, [&]() {
      std::vector<FigureDescription> descriptions;
      const auto nodes = dataStorage->GetSubset(GetFigurePredicate());

      for (const auto& node : *nodes)
      {
        if (static_cast<const mitk::PlanarFigure*>(node->GetData())->IsComplete())
          descriptions.push_back(DescribeFigure(dataStorage, node));
      }

      return descriptions;
    });

    auto json = nlohmann::json::array();

    for (const auto& figure : figures)
      json.push_back(ToJson(*server, figure));

    return RestResponse::FromData(json);
  };
  routes.push_back(list);

  RestRoute create;
  create.Method = "POST";
  create.Path = "/planar-figures";
  create.Summary = "Draw a line, circle or rectangle in a 2D window, as the Measurement view does";
  create.Description =
    "The figure lies in the plane of the slice that the window shows now, as if drawn there with the mouse. Body: "
    "{type: line, circle or rectangle; image: the uid of the image the figure marks, which becomes its parent in "
    "the Data Manager; window: axial, sagittal or coronal; two points, either pixels: [[x, y], [x, y]] of a "
    "screenshot of the window with image_size, as GET /api/v1/rendering/windows/{id}/screenshot describes it, "
    "or points: [[x, y, z], [x, y, z]] in millimeters in the slice the window shows; name (optional), Circle, "
    "Circle 2 and so on without it}. A line takes its two ends, a circle its center and a point on it, a rectangle "
    "two opposite corners. Returns the figure as GET /api/v1/planar-figures lists it, with status 201. Status 422 "
    "if image is no image or a point lies off the slice. The figure's look follows properties of its node, which "
    "PATCH /api/v1/datastorage/nodes/{uid}/properties changes: color, line width in pixels, planarfigure.label, "
    "when the label with the name and measurements shows (Always, On hover, When selected, On hover or when "
    "selected, On hover while selected or Never), and the booleans planarfigure.drawname, "
    "planarfigure.drawquantities and planarfigure.drawcontrastoutline, a black or white outline that keeps the "
    "figure visible on any image. DELETE /api/v1/datastorage/nodes/{uid} removes a figure.";
  create.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto type = body.value("type", std::string());

    if (!body.contains("image") || !body["image"].is_string())
      ThrowInvalid("\"image\", the uid of the image that the figure marks, is missing.", request.Path);

    if (!body.contains("window") || !body["window"].is_string())
      ThrowInvalid("\"window\", the 2D window in whose slice to draw, is missing.", request.Path);

    const auto window = body["window"].get<std::string>();
    const auto imageNode = server->FindNode(body["image"].get<std::string>());

    if (imageNode.IsNull())
      throw RestException(ErrorResponse::NodeNotFound(body["image"].get<std::string>(), request.Path));

    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figure = CallInGuiThread(*server, [&]() {
      auto planarFigure = CreateFigure(type);

      if (planarFigure.IsNull())
        ThrowInvalid("\"type\" must be line, circle or rectangle.", request.Path);

      if (dynamic_cast<const mitk::Image*>(imageNode->GetData()) == nullptr)
        Throw("NOT_AN_IMAGE", "Unprocessable Content", "\"" + imageNode->GetName() + "\" is no image.", 422, request.Path);

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
          Throw("POINT_OFF_SLICE", "Unprocessable Content", "A point lies off the slice that the " + window +
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
      node->SetName(name.empty() ? CreateName(dataStorage, type) : name);
      node->SetData(planarFigure);

      dataStorage->Add(node, imageNode);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return DescribeFigure(dataStorage, node);
    });

    return RestResponse::FromData(ToJson(*server, figure), 201);
  };
  routes.push_back(create);

  return routes;
}
