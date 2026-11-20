/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiRenderingRoutes.h"

#include <QmitkAbstractMultiWidgetEditor.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowScreenshot.h>
#include <QmitkStdMultiWidget.h>

#include <mitkErrorResponse.h>
#include <mitkException.h>
#include <mitkImageReadAccessor.h>
#include <mitkIRenderWindowPart.h>
#include <mitkIRestServerService.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkRestGuiThread.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>
#include <mitkRestRendering.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>

#include <QBuffer>
#include <QImage>
#include <QPainter>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

namespace
{
  /** The editor of the standard display, or nullptr if it is not open. */
  berry::IWorkbenchPart::Pointer FindStdMultiWidgetEditor()
  {
    for (const auto& window : berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows())
    {
      const auto page = window->GetActivePage();

      if (page.IsNull())
        continue;

      for (const auto& reference : page->GetEditorReferences())
      {
        if (reference->GetId() != "org.mitk.editors.stdmultiwidget")
          continue;

        if (auto part = reference->GetPart(false); part.IsNotNull())
          return part;
      }
    }

    return {};
  }

  QmitkRenderWindow* FindRenderWindow(const std::string& id, const std::string& path)
  {
    // Validates the id and that the window exists.
    mitk::GetRestRenderer(id, path);

    const auto editor = FindStdMultiWidgetEditor();
    auto* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());
    auto* renderWindow = renderWindowPart != nullptr ? renderWindowPart->GetQmitkRenderWindow(QString::fromStdString(id)) : nullptr;

    if (renderWindow == nullptr)
      throw mitk::RestException(mitk::ErrorResponse::RenderWindowNotAvailable(path));

    return renderWindow;
  }

  QmitkStdMultiWidget* FindStdMultiWidget(const std::string& path)
  {
    const auto part = FindStdMultiWidgetEditor();
    auto* editor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(part.GetPointer());
    auto* multiWidget = editor != nullptr ? dynamic_cast<QmitkStdMultiWidget*>(editor->GetMultiWidget()) : nullptr;

    if (multiWidget == nullptr)
      throw mitk::RestException(mitk::ErrorResponse::RenderWindowNotAvailable(path));

    return multiWidget;
  }

  /** Whether the crosshair shows in the 2D windows and in the 3D window, as the render window menus toggle it. */
  nlohmann::json DescribeCrosshair(QmitkStdMultiWidget* multiWidget)
  {
    return mitk::DescribeRestCrosshair(
      { multiWidget->GetWidgetPlane1().GetPointer(), multiWidget->GetWidgetPlane2().GetPointer(), multiWidget->GetWidgetPlane3().GetPointer() },
      multiWidget->GetRenderWindow1()->GetRenderer(),
      multiWidget->GetRenderWindow4()->GetRenderer());
  }

  bool Is2D(QmitkRenderWindow* window)
  {
    return mitk::IsRestRenderer2D(window->GetRenderer());
  }

  nlohmann::json DescribeWindow(const std::string& id, QmitkRenderWindow* window)
  {
    return mitk::DescribeRestWindow(id, window->GetRenderer());
  }

  /** The capture options that the query asks for. */
  QmitkRenderWindowScreenshot::Options GetScreenshotOptions(const mitk::RestRequest& request)
  {
    const auto parsed = mitk::ParseRestScreenshotOptions(request);

    QmitkRenderWindowScreenshot::Options options;
    options.MaxSize = parsed.MaxSize;
    options.Decorations = parsed.Decorations;
    options.Magnification = parsed.Magnification;
    options.CropToContent = parsed.CropToContent;
    options.Padding = parsed.Padding;

    switch (parsed.BackgroundMode)
    {
      case mitk::RestScreenshotOptions::Background::AsShown:
        break;

      case mitk::RestScreenshotOptions::Background::Transparent:
        options.BackgroundMode = QmitkRenderWindowScreenshot::Background::Transparent;
        break;

      case mitk::RestScreenshotOptions::Background::Color:
        options.BackgroundMode = QmitkRenderWindowScreenshot::Background::Color;
        options.BackgroundColor = QColor(QString::fromStdString(mitk::ToRestColor(parsed.BackgroundColor)));
        break;
    }

    return options;
  }

  /** Faint lines every 50 pixels, labeled every 100, so that points can be read off. */
  void DrawGrid(QImage& image)
  {
    QPainter painter(&image);
    auto font = painter.font();
    font.setPixelSize(11);
    painter.setFont(font);

    const QColor faint(255, 255, 0, 40);
    const QColor label(255, 255, 0, 220);

    for (const auto& line : mitk::GetRestGridLines(image.width()))
    {
      painter.setPen(faint);
      painter.drawLine(line.Position, 0, line.Position, image.height());

      if (line.Labeled)
      {
        painter.setPen(label);
        painter.drawText(line.Position + 2, 12, QString::number(line.Position));
      }
    }

    for (const auto& line : mitk::GetRestGridLines(image.height()))
    {
      painter.setPen(faint);
      painter.drawLine(0, line.Position, image.width(), line.Position);

      if (line.Labeled)
      {
        painter.setPen(label);
        painter.drawText(2, line.Position - 2, QString::number(line.Position));
      }
    }
  }

  /**
   * The time step of the data at the selected time point.
   *
   * \throw RestException with status 422 if the data has no time step there.
   */
  mitk::TimeStepType GetSelectedTimeStep(const mitk::DataNode* node, const std::string& path)
  {
    return mitk::GetRestTimeStep(
      node, mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint(), path);
  }

  /** The corners of the box around the voxels of the labels, in world coordinates; empty if they have none. */
  std::vector<mitk::Point3D> GetLabelCorners(const mitk::MultiLabelSegmentation* segmentation,
    const std::vector<mitk::MultiLabelSegmentation::LabelValueType>& values, mitk::TimeStepType timeStep)
  {
    std::vector<bool> wanted(std::numeric_limits<mitk::Label::PixelType>::max() + 1, false);
    std::set<mitk::MultiLabelSegmentation::GroupIndexType> groups;

    for (const auto value : values)
    {
      wanted[value] = true;
      groups.insert(segmentation->GetGroupIndexOfLabel(value));
    }

    std::array<unsigned int, 3> lower = { std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max() };
    std::array<unsigned int, 3> upper = { 0, 0, 0 };
    bool found = false;

    for (const auto group : groups)
    {
      const auto* groupImage = segmentation->GetGroupImage(group);

      if (groupImage->GetPixelType().GetComponentType() != itk::IOComponentEnum::USHORT)
        continue;

      mitk::ImageReadAccessor accessor(mitk::Image::ConstPointer(groupImage), groupImage->GetVolumeData(timeStep));
      const auto* pixels = static_cast<const mitk::Label::PixelType*>(accessor.GetData());
      const std::array<unsigned int, 3> size = { groupImage->GetDimension(0), groupImage->GetDimension(1), groupImage->GetDimension(2) };

      for (unsigned int z = 0; z < size[2]; ++z)
      {
        for (unsigned int y = 0; y < size[1]; ++y)
        {
          const auto* row = pixels + (static_cast<std::size_t>(z) * size[1] + y) * size[0];

          for (unsigned int x = 0; x < size[0]; ++x)
          {
            if (!wanted[row[x]])
              continue;

            lower = { std::min(lower[0], x), std::min(lower[1], y), std::min(lower[2], z) };
            upper = { std::max(upper[0], x), std::max(upper[1], y), std::max(upper[2], z) };
            found = true;
          }
        }
      }
    }

    std::vector<mitk::Point3D> corners;

    if (!found)
      return corners;

    // Voxels reach half a voxel beyond their centers, which the indices name.
    const auto* geometry = segmentation->GetGeometry(timeStep);

    for (int corner = 0; corner < 8; ++corner)
    {
      mitk::Point3D index;

      for (int axis = 0; axis < 3; ++axis)
        index[axis] = (corner >> axis) & 1 ? upper[axis] + 0.5 : lower[axis] - 0.5;

      mitk::Point3D world;
      geometry->IndexToWorld(index, world);
      corners.push_back(world);
    }

    return corners;
  }

  /** The gray values and labels under a point, in the data that a window shows. */
  nlohmann::json DescribeData(const mitk::Point3D& point, mitk::BaseRenderer* renderer,
    const mitk::DataStorage* dataStorage, const mitk::IRestServerService* server)
  {
    auto values = nlohmann::json::array();

    if (dataStorage == nullptr)
      return values;

    const auto notHelper = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));
    const auto timePoint = renderer->GetTime();

    // Held in a variable: the set that a range-for over *GetSubset() walks would die with the temporary pointer.
    const auto nodes = dataStorage->GetSubset(notHelper);

    for (const auto& node : *nodes)
    {
      if (!node->IsVisible(renderer))
        continue;

      nlohmann::json entry = { { "uid", server->GetNodeUid(node) }, { "name", node->GetName() } };

      if (const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(node->GetData()); segmentation != nullptr)
      {
        auto labels = nlohmann::json::array();

        for (unsigned int group = 0; group < segmentation->GetNumberOfGroups(); ++group)
        {
          const auto value = mitk::GetRestImageValue(segmentation->GetGroupImage(group), point, timePoint);

          if (!value.has_value() || *value == 0)
            continue;

          const auto labelValue = static_cast<mitk::MultiLabelSegmentation::LabelValueType>(*value);
          const auto label = segmentation->GetLabel(labelValue);
          labels.push_back({ { "value", labelValue }, { "name", label.IsNotNull() ? label->GetName() : std::string() } });
        }

        entry["labels"] = labels;
      }
      else if (const auto* image = dynamic_cast<const mitk::Image*>(node->GetData()); image != nullptr)
      {
        const auto value = mitk::GetRestImageValue(image, point, timePoint);

        if (!value.has_value())
          continue;

        entry["value"] = *value;
      }
      else
      {
        continue;
      }

      values.push_back(std::move(entry));
    }

    return values;
  }
}

std::vector<mitk::RestRoute> mitk::CreateRenderingRoutes(IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute windows;
  windows.Method = "GET";
  windows.Path = "/rendering/windows";
  windows.Summary = "List the render windows of the standard display";
  windows.Description =
    "Returns {data: [{id, kind: 2d|3d, size: [width, height] in pixels, timepoint_ms, the time point the window "
    "shows, view_direction and slice: {index, count} for 2D windows}]}. The slice index counts as the Image "
    "Navigator does, and as PUT /api/v1/rendering/windows/{id}/slice takes it.";
  windows.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&request]() {
      auto json = nlohmann::json::array();

      for (const auto& id : GetRestWindowIds())
        json.push_back(DescribeWindow(id, FindRenderWindow(id, request.Path)));

      return json;
    }));
  };
  routes.push_back(windows);

  RestRoute slice;
  slice.Method = "PUT";
  slice.Path = "/rendering/windows/:id/slice";
  slice.Summary = "Show a slice in a 2D render window";
  slice.Description =
    "Path parameter: id of a 2D window. Body: {index}, the slice to show, counted from 0 as GET "
    "/api/v1/rendering/windows and the Image Navigator count them; to move k slices, send the current index "
    "plus k. Returns {data: window} as GET /api/v1/rendering/windows describes it, or 400 for the 3d window.";
  slice.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("index") || !body["index"].is_number_integer())
      throw RestException(ErrorResponse::InvalidRequest("\"index\", the number of a slice, is missing or no integer.", request.Path));

    const auto index = body["index"].get<std::int64_t>();
    const auto& id = request.PathParams.at("id");

    return RestResponse::FromData(CallInGuiThread(*server, [&request, &id, index]() {
      auto* window = FindRenderWindow(id, request.Path);

      if (!Is2D(window))
        throw RestException(ErrorResponse::InvalidRequest("The " + id + " window shows no slices.", request.Path));

      auto* stepper = window->GetSliceNavigationController()->GetStepper();
      const auto count = static_cast<int>(stepper->GetSteps());

      if (count == 0)
      {
        throw RestException(ErrorResponse::Create("NO_SLICES", "Conflict",
          "The " + id + " window shows no data to step through.", 409, request.Path));
      }

      if (index < 0 || index >= count)
      {
        throw RestException(ErrorResponse::Create("SLICE_OUT_OF_RANGE", "Unprocessable Content",
          "The " + id + " window has " + std::to_string(count) + " slices, numbered from 0 to " +
          std::to_string(count - 1) + ".", 422, request.Path));
      }

      stepper->SetPos(static_cast<unsigned int>(mitk::ConvertRestSliceNumber(window->GetRenderer(), count, static_cast<int>(index))));
      return DescribeWindow(id, window);
    }));
  };
  routes.push_back(slice);

  RestRoute focus;
  focus.Method = "PUT";
  focus.Path = "/rendering/windows/:id/focus";
  focus.Summary = "Frame labels or data in a render window";
  focus.Description =
    "Path parameter: id, one of axial, sagittal, coronal, 3d. Body: {segmentation: uid, labels: [value]} to frame "
    "labels of a segmentation, or {uids: [uid]} to frame data, a planar figure by its outline in its own slice; "
    "margin (optional, default 0.2), the space around "
    "them as a share of their size. A 2D window shows the slice through their center and zooms so that they fill "
    "it. The 3D window keeps its direction, which the camera endpoint's standard_view sets, and moves so that they "
    "fill it. A window in which they have no extent keeps its zoom and only centers on them. Returns {data: "
    "window} as GET /api/v1/rendering/windows describes it.";
  focus.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto margin = body.value("margin", 0.2);

    if (margin < 0.0 || margin > 10.0)
      throw RestException(ErrorResponse::InvalidRequest("margin must lie between 0 and 10.", request.Path));

    const auto labelNotFound = [&request](std::uint64_t value) {
      return RestException(ErrorResponse::Create("LABEL_NOT_FOUND", "Not Found",
        "The segmentation has no label " + std::to_string(value) + ".", 404, request.Path));
    };

    DataNodeHandle segmentationNode;
    std::vector<MultiLabelSegmentation::LabelValueType> labels;
    std::vector<DataNodeHandle> nodes;

    if (body.contains("segmentation"))
    {
      segmentationNode = FindRestNode(*server, body["segmentation"], request.Path);

      for (const auto& value : body.value("labels", nlohmann::json::array()))
      {
        if (!value.is_number_unsigned())
          throw RestException(ErrorResponse::InvalidRequest("labels are label values, numbers.", request.Path));

        // No label has a value beyond the range of its type, which the conversion would wrap into.
        const auto label = value.get<std::uint64_t>();

        if (label > std::numeric_limits<MultiLabelSegmentation::LabelValueType>::max())
          throw labelNotFound(label);

        labels.push_back(static_cast<MultiLabelSegmentation::LabelValueType>(label));
      }

      if (labels.empty())
        throw RestException(ErrorResponse::InvalidRequest("Name the labels to frame by their values.", request.Path));
    }
    else
    {
      for (const auto& uid : body.value("uids", nlohmann::json::array()))
        nodes.push_back(FindRestNode(*server, uid, request.Path));

      if (nodes.empty())
        throw RestException(ErrorResponse::InvalidRequest("Give a segmentation with labels, or uids.", request.Path));
    }

    const auto& id = request.PathParams.at("id");

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* window = FindRenderWindow(id, request.Path);
      std::vector<Point3D> corners;

      if (segmentationNode != nullptr)
      {
        const auto* segmentation = dynamic_cast<const MultiLabelSegmentation*>(segmentationNode->GetData());

        if (segmentation == nullptr)
        {
          throw RestException(ErrorResponse::Create("NOT_A_SEGMENTATION", "Unprocessable Content",
            "\"" + segmentationNode->GetName() + "\" is no segmentation.", 422, request.Path));
        }

        for (const auto value : labels)
        {
          if (!segmentation->ExistLabel(value))
            throw labelNotFound(value);
        }

        corners = GetLabelCorners(segmentation, labels, GetSelectedTimeStep(segmentationNode.get(), request.Path));

        if (corners.empty())
        {
          throw RestException(ErrorResponse::Create("LABELS_EMPTY", "Unprocessable Content",
            "The labels are empty at the selected time point.", 422, request.Path));
        }
      }
      else
      {
        for (const auto& node : nodes)
        {
          auto* data = node->GetData();

          if (data == nullptr)
            continue;

          // The geometry of a planar figure is the whole plane it lies on, and its center is
          // half a slice off that plane. The outline frames the figure itself, in its slice.
          if (const auto* figure = dynamic_cast<const PlanarFigure*>(data); figure != nullptr)
          {
            const auto* plane = figure->GetPlaneGeometry();

            // A figure has no plane until its placement reaches a render window.
            if (plane == nullptr)
              continue;

            for (const auto& contour : figure->GetContours())
            {
              for (const auto& point : contour.Points)
              {
                Point3D corner;
                plane->Map(point, corner);
                corners.push_back(corner);
              }
            }

            continue;
          }

          const auto timeStep = GetSelectedTimeStep(node.get(), request.Path);

          // Point sets compute their bounds only on request, and one without points keeps its initial ones.
          if (data->IsEmptyTimeStep(timeStep))
            continue;

          // The bounds of a point set are at least 1 mm wide, so points that coincide, like a single one,
          // would frame that much. The points themselves frame no extent then.
          if (const auto* pointSet = dynamic_cast<const PointSet*>(data); pointSet != nullptr)
          {
            for (auto point = pointSet->Begin(timeStep); point != pointSet->End(timeStep); ++point)
              corners.push_back(pointSet->GetPoint(point.Index(), timeStep));

            continue;
          }

          const auto* geometry = data->GetUpdatedGeometry(timeStep);

          for (int corner = 0; corner < 8; ++corner)
            corners.push_back(geometry->GetCornerPoint(corner));
        }

        if (corners.empty())
          throw RestException(ErrorResponse::InvalidRequest("The data has no extent to frame.", request.Path));
      }

      if (Is2D(window))
        mitk::FocusRestRenderer2D(window->GetRenderer(), corners, margin);
      else
        mitk::FocusRestRenderer3D(window->GetRenderer(), corners, margin);

      RenderingManager::GetInstance()->RequestUpdate(window->GetVtkRenderWindow());
      return DescribeWindow(id, window);
    }));
  };
  routes.push_back(focus);

  RestRoute crosshair;
  crosshair.Method = "GET";
  crosshair.Path = "/rendering/crosshair";
  crosshair.Summary = "Tell whether the crosshair shows in the render windows";
  crosshair.Description =
    "Returns {data: {visible, visible_3d}}: whether the crosshair shows in the 2D windows, and in the 3D "
    "window.";
  crosshair.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&request]() { return DescribeCrosshair(FindStdMultiWidget(request.Path)); }));
  };
  routes.push_back(crosshair);

  RestRoute changeCrosshair;
  changeCrosshair.Method = "PUT";
  changeCrosshair.Path = "/rendering/crosshair";
  changeCrosshair.Summary = "Show or hide the crosshair, as the render window menus do";
  changeCrosshair.Description =
    "Body, at least one of: {visible, visible_3d}. visible shows or hides the crosshair in all windows, as "
    "the crosshair menu of each window does; visible_3d only in the 3D window, as the menu of the 3D window "
    "does, and applies after visible. Returns {data: {visible, visible_3d}} as GET does.";
  changeCrosshair.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    std::optional<bool> visible, visible3D;

    try
    {
      if (body.contains("visible"))
        visible = body["visible"].get<bool>();

      if (body.contains("visible_3d"))
        visible3D = body["visible_3d"].get<bool>();
    }
    catch (const nlohmann::json::exception&)
    {
      throw RestException(ErrorResponse::InvalidRequest("visible and visible_3d must be true or false.", request.Path));
    }

    if (!visible.has_value() && !visible3D.has_value())
      throw RestException(ErrorResponse::InvalidRequest("Give visible, visible_3d or both.", request.Path));

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* multiWidget = FindStdMultiWidget(request.Path);

      if (visible.has_value())
        multiWidget->SetCrosshairVisibility(*visible);

      if (visible3D.has_value())
        multiWidget->SetCrosshair3DVisibility(*visible3D);

      return DescribeCrosshair(multiWidget);
    }));
  };
  routes.push_back(changeCrosshair);

  const auto describeLayout = [](const std::string& path) {
    nlohmann::json maximized = nullptr;

    for (const auto& id : GetRestWindowIds())
    {
      if (FindRenderWindow(id, path)->IsFullScreen())
        maximized = id;
    }

    return nlohmann::json{ { "maximized", maximized } };
  };

  RestRoute layout;
  layout.Method = "GET";
  layout.Path = "/rendering/layout";
  layout.Summary = "Tell which render window is shown alone, if any";
  layout.Description =
    "Returns {data: {maximized}}: the id of the window that its full screen button shows alone, or null if "
    "that button is on no window. This is the state of the full screen button of the window's menu; a layout "
    "chosen in the layout menu (for example \"This big\") is not seen by it, and only displays that connect "
    "LayoutDesignChanged support it.";
  layout.Handler = [server, describeLayout](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() { return describeLayout(request.Path); }));
  };
  routes.push_back(layout);

  RestRoute changeLayout;
  changeLayout.Method = "PUT";
  changeLayout.Path = "/rendering/layout";
  changeLayout.Summary = "Show one render window alone, or all of them again, as the full screen button does";
  changeLayout.Description =
    "Body: {maximized}, the id of a window (axial, sagittal, coronal, 3d) to show it alone, or null to bring "
    "back the layout the windows had before. It sets the state of the full screen button of the window's menu; "
    "a layout chosen in the layout menu (for example \"This big\") is not seen by it, and only displays that "
    "connect LayoutDesignChanged support it. Returns {data: {maximized}} as GET does. Answers 409 "
    "LAYOUT_NOT_AVAILABLE while the editor of the windows is hidden, for example behind a report.";
  changeLayout.Handler = [server, describeLayout](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.contains("maximized") || !(body["maximized"].is_null() || body["maximized"].is_string()))
      throw RestException(ErrorResponse::InvalidRequest("\"maximized\" must be the id of a window or null.", request.Path));

    const auto target = body["maximized"].is_null() ? std::optional<std::string>() : body["maximized"].get<std::string>();

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* targetWindow = target.has_value() ? FindRenderWindow(*target, request.Path) : nullptr;

      try
      {
        // One window at a time: the others are hidden while it is shown alone,
        // so their buttons cannot be pressed either.
        for (const auto& id : GetRestWindowIds())
        {
          auto* window = FindRenderWindow(id, request.Path);

          if (window != targetWindow && window->IsFullScreen())
            window->SetFullScreen(false);
        }

        if (targetWindow != nullptr)
          targetWindow->SetFullScreen(true);
      }
      catch (const Exception& e)
      {
        throw RestException(ErrorResponse::Create("LAYOUT_NOT_AVAILABLE", "Conflict", e.GetDescription(), 409, request.Path));
      }

      return describeLayout(request.Path);
    }));
  };
  routes.push_back(changeLayout);

  RestRoute screenshot;
  screenshot.Method = "GET";
  screenshot.Path = "/rendering/windows/:id/screenshot";
  screenshot.Summary = "Take a screenshot of a render window that points can be read off";
  screenshot.Description =
    "Path parameter: id, one of axial, sagittal, coronal, 3d. Query parameters, all optional: max_size (default "
    "1000, clamped to 100 to 4000), the longest side in pixels; grid=true draws faint lines every 50 pixels, "
    "labeled every 100; "
    "background, transparent or a color like ffffff (six hex digits; %23ffffff works, too) instead of the "
    "window's; decorations=false leaves out "
    "the corner texts, logo and the gradient of the background; scale, 2 to 4, renders that many pixels per window pixel, without "
    "decorations, for sharper images (raise max_size with it, as no more pixels than max_size are rendered unless "
    "crop=true); crop=true cuts the image down to the part of the "
    "scene that is not background, without decorations, and padding keeps that many pixels of background "
    "around it. With an opaque background, pixels that have the background's color count as background, so a "
    "dark part of the image at the border is cut away; only the transparent background decides by opacity. "
    "Answers 400 if the crop finds nothing but background, and 500 if the window could not be captured. "
    "As the Screenshot Maker view takes them; transparent is transparent where "
    "the scene shows background, and partly so behind semitransparent objects. Returns a PNG. The header "
    "X-MITK-Screenshot describes it as JSON: the window as GET /api/v1/rendering/windows does, plus image_size: "
    "[width, height] and view_state, a fingerprint of the view that the screenshot shows (camera, slice, zoom, time "
    "point and window size). Endpoints that take points accept pixels of the screenshot with that window, image_size "
    "and view_state, unless it is cropped: its pixels no longer map onto the window, so crop does not go with grid "
    "either. They answer 409 VIEW_CHANGED if the view has changed since, for example by scrolling, zooming or "
    "another time point: take a new screenshot then.";
  screenshot.Handler = [server](const RestRequest& request) {
    const bool grid = mitk::GetRestQueryFlag(request, "grid", false);
    const auto options = GetScreenshotOptions(request);
    const auto& id = request.PathParams.at("id");

    if (grid && options.CropToContent)
    {
      throw RestException(ErrorResponse::InvalidRequest(
        "crop and grid do not go together: the pixels of a cropped screenshot do not map onto the window.", request.Path));
    }

    return CallInGuiThread(*server, [&request, &id, &options, grid]() {
      auto* window = FindRenderWindow(id, request.Path);
      const auto editor = FindStdMultiWidgetEditor();
      auto* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(editor.GetPointer());

      QImage image;

      try
      {
        image = QmitkRenderWindowScreenshot::Take(renderWindowPart, window, options);
      }
      catch (const Exception&)
      {
        throw RestException(ErrorResponse::InternalError("The window could not be captured.", request.Path));
      }

      // Take returns no image only for a crop that finds nothing but background.
      if (image.isNull())
        throw RestException(ErrorResponse::InvalidRequest("The window shows only background, there is nothing to crop to.", request.Path));

      if (grid)
        DrawGrid(image);

      QByteArray png;
      QBuffer buffer(&png);

      if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG"))
        throw RestException(ErrorResponse::InternalError("The screenshot could not be encoded.", request.Path));

      auto info = DescribeWindow(id, window);
      info["image_size"] = { image.width(), image.height() };
      info["view_state"] = GetRestViewState(window->GetRenderer());

      auto response = RestResponse::FromBytes(png.toStdString(), "image/png");
      response.Headers["X-MITK-Screenshot"] = info.dump();
      return response;
    });
  };
  routes.push_back(screenshot);

  RestRoute pick;
  pick.Method = "POST";
  pick.Path = "/rendering/windows/:id/pick";
  pick.Summary = "Find the world coordinates and data values under pixels of a screenshot";
  pick.Description =
    "Path parameter: id of a window. Body: {pixels: [[x, y], ...], image_size: [width, height], view_state} of a "
    "screenshot of that window, which must be a 2D window, with image_size and view_state as its header "
    "X-MITK-Screenshot gives them (409 VIEW_CHANGED if the view has changed since: take a new screenshot), or "
    "{points: [[x, y, z], ...]} in millimeters. "
    "Returns {data: [{position: [x, y, z] in millimeters, data: [{uid, name, "
    "value} for images, {uid, name, labels: [{value, name}]} for segmentations]}]}, for the data the window "
    "shows at each point.";
  pick.Handler = [server](const RestRequest& request) {
    auto body = request.GetJsonBody();
    body["window"] = request.PathParams.at("id");

    // Not in the GUI thread, which requests of the data storage may wait for.
    const auto dataStorage = server->GetDataStorage();

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* renderer = GetRestRenderer(body["window"].get<std::string>(), request.Path);
      auto json = nlohmann::json::array();

      for (const auto& point : GetRestWorldPoints(body, request.Path))
        json.push_back({ { "position", ToRestJson(point) }, { "data", DescribeData(point, renderer, dataStorage, server) } });

      return json;
    }));
  };
  routes.push_back(pick);

  return routes;
}
