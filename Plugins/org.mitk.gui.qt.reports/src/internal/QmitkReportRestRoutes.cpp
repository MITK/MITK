/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportRestRoutes.h"
#include "QmitkReportEditor.h"
#include "QmitkReportEditorInput.h"
#include "QmitkReportsPluginActivator.h"
#include "QmitkReportStore.h"

#include <mitkErrorResponse.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkIRestServerService.h>
#include <mitkRestGuiThread.h>

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>
#include <berryPlatformUI.h>

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QRegularExpression>

#include <algorithm>
#include <cctype>
#include <optional>

using mitk::CallInGuiThread;
using mitk::ErrorResponse;
using mitk::RestException;
using mitk::RestRequest;
using mitk::RestResponse;
using mitk::RestRoute;

namespace
{
  /** The longest side of the images that clients get back to check a capture. */
  constexpr int MaxPreviewSize = 1000;

  /** The most HTML that a report takes, in bytes: 32 MB. */
  constexpr qsizetype MaxHtmlSize = 33554432;

  [[noreturn]] void Throw(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
  {
    throw RestException(ErrorResponse::Create(code, title, detail, status, path));
  }

  QmitkReportStore* GetStore(const std::string& path)
  {
    auto* store = QmitkReportsPluginActivator::GetStore();

    if (store == nullptr)
      Throw("REPORTS_NOT_AVAILABLE", "Service Unavailable", "Reports are not available.", 503, path);

    return store;
  }

  int GetReportId(const RestRequest& request)
  {
    const auto& text = request.PathParams.at("id");

    if (!text.empty() && text.size() < 9 && std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
      return std::stoi(text);

    Throw("REPORT_NOT_FOUND", "Not Found", "There is no report " + text + ". GET /api/v1/reports lists them.", 404, request.Path);
  }

  const QmitkReportStore::Report& GetReport(const QmitkReportStore* store, int id, const std::string& path)
  {
    const auto* report = store->Get(id);

    if (report == nullptr)
    {
      Throw("REPORT_NOT_FOUND", "Not Found", "There is no report " + std::to_string(id) +
        ". Closing its editor discards a report; GET /api/v1/reports lists the open ones.", 404, path);
    }

    return *report;
  }

  std::optional<QString> GetText(const nlohmann::json& body, const std::string& name, const std::string& path)
  {
    if (!body.contains(name) || body[name].is_null())
      return std::nullopt;

    if (!body[name].is_string())
      throw RestException(ErrorResponse::InvalidRequest(name + " must be a string.", path));

    return QString::fromStdString(body[name].get<std::string>());
  }

  /** Refuse HTML that is too large, or that refers to an image the store would not find. */
  void CheckHtml(const QString& html, const std::string& path)
  {
    if (html.toUtf8().size() > MaxHtmlSize)
      Throw("REPORT_TOO_LARGE", "Payload Too Large", "The html takes at most 32 MB (33554432 bytes).", 413, path);

    // The store finds images by their quoted src values only.
    static const QRegularExpression unquotedSource(QStringLiteral("<img\\b[^>]*?(?<![\\w-])src\\s*+=\\s*+(?![\"'])"),
      QRegularExpression::CaseInsensitiveOption);

    if (unquotedSource.match(html).hasMatch())
      throw RestException(ErrorResponse::InvalidRequest("An img tag has a src without quotes. Images need src in quotes, like <img src=\"image-3.png\">.", path));
  }

  nlohmann::json Describe(const QmitkReportStore::Report& report, bool withHtml)
  {
    auto images = nlohmann::json::array();

    for (const auto& name : report.Images.keys())
      images.push_back(name.toStdString());

    nlohmann::json json = {
      { "id", report.Id },
      { "title", report.Title.toStdString() },
      { "images", images },
      { "modified", report.Modified.toString(Qt::ISODate).toStdString() }
    };

    if (withHtml)
      json["html"] = report.Html.toStdString();

    return json;
  }

  [[noreturn]] void ThrowUnknownImages(const mitk::Exception& e, const std::string& path)
  {
    Throw("IMAGE_NOT_FOUND", "Bad Request", e.GetDescription() +
      std::string(" Reports show images of POST /api/v1/reports/images by their names; nothing else. Only the last "
        "100 captured images are kept for new reports; capture an older one again."), 400, path);
  }

  /** Open the report's editor, or bring it to the top, leaving the keyboard focus where it is. */
  void ShowReport(int id, const std::string& path)
  {
    auto* workbench = berry::PlatformUI::GetWorkbench();
    auto window = workbench->GetActiveWorkbenchWindow();

    if (window.IsNull() && !workbench->GetWorkbenchWindows().isEmpty())
      window = workbench->GetWorkbenchWindows().front();

    const auto page = window.IsNotNull() ? window->GetActivePage() : berry::IWorkbenchPage::Pointer();

    if (page.IsNull())
      Throw("WORKBENCH_NOT_AVAILABLE", "Service Unavailable", "The Workbench has no window.", 503, path);

    const berry::IEditorInput::Pointer input(new QmitkReportEditorInput(id));
    page->OpenEditor(input, QmitkReportEditor::EDITOR_ID, false);
  }

  std::string EncodePng(const QImage& image)
  {
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return png.toStdString();
  }

  /** The description of POST /reports: the page's components and the CSS that the Workbench renders. */
  std::string LoadGuide()
  {
    QFile file(QStringLiteral(":/org.mitk.gui.qt.reports/ReportGuide.txt"));

    if (!file.open(QIODevice::ReadOnly))
      mitkThrow() << "Cannot read the report guide: " << file.errorString().toStdString();

    return file.readAll().trimmed().toStdString();
  }
}

std::vector<RestRoute> CreateReportRoutes(mitk::IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute list;
  list.Method = "GET";
  list.Path = "/reports";
  list.Summary = "List the reports that are open";
  list.Description =
    "Returns {data: [{id, title, images, modified}]}. A report lives as long as its editor: when the user closes "
    "it, the report is gone.";
  list.Handler = [server](const RestRequest& request) {
    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto reports = nlohmann::json::array();

      for (const auto* report : GetStore(request.Path)->GetReports())
        reports.push_back(Describe(*report, false));

      return reports;
    }));
  };
  routes.push_back(list);

  RestRoute create;
  create.Method = "POST";
  create.Path = "/reports";
  create.Summary = "Write a report that opens in an editor of its own";
  create.Description = LoadGuide();
  create.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto title = GetText(body, "title", request.Path);
    const auto html = GetText(body, "html", request.Path);

    if (!title.has_value() || title->trimmed().isEmpty() || !html.has_value())
      throw RestException(ErrorResponse::InvalidRequest("The body needs a title and html.", request.Path));

    CheckHtml(*html, request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* store = GetStore(request.Path);
      int id = 0;

      try
      {
        id = store->Add(title->trimmed(), *html);
      }
      catch (const mitk::Exception& e)
      {
        ThrowUnknownImages(e, request.Path);
      }

      try
      {
        ShowReport(id, request.Path);
      }
      catch (...)
      {
        store->Remove(id);
        throw;
      }

      return Describe(GetReport(store, id, request.Path), false);
    }), 201);
  };
  routes.push_back(create);

  RestRoute get;
  get.Method = "GET";
  get.Path = "/reports/:id";
  get.Summary = "Get a report as written";
  get.Description = "Returns {data: {id, title, html, images, modified}}, html being the body as written.";
  get.Handler = [server](const RestRequest& request) {
    const auto id = GetReportId(request);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      return Describe(GetReport(GetStore(request.Path), id, request.Path), true);
    }));
  };
  routes.push_back(get);

  RestRoute change;
  change.Method = "PUT";
  change.Path = "/reports/:id";
  change.Summary = "Rewrite a report";
  change.Description =
    "Body: {html, title (optional)}, html replacing the whole body; see POST /api/v1/reports. The editor shows the "
    "new version where the user was reading and comes to the top. Returns {data: {id, title, images, modified}}.";
  change.Handler = [server](const RestRequest& request) {
    const auto id = GetReportId(request);
    const auto body = request.GetJsonBody();
    const auto title = GetText(body, "title", request.Path);
    const auto html = GetText(body, "html", request.Path);

    if (!html.has_value() || (title.has_value() && title->trimmed().isEmpty()))
      throw RestException(ErrorResponse::InvalidRequest("The body needs html, and a title is not empty.", request.Path));

    CheckHtml(*html, request.Path);

    return RestResponse::FromData(CallInGuiThread(*server, [&]() {
      auto* store = GetStore(request.Path);
      const auto& report = GetReport(store, id, request.Path);

      try
      {
        store->Update(id, title.has_value() ? title->trimmed() : report.Title, *html);
      }
      catch (const mitk::Exception& e)
      {
        ThrowUnknownImages(e, request.Path);
      }

      ShowReport(id, request.Path);
      return Describe(GetReport(store, id, request.Path), false);
    }));
  };
  routes.push_back(change);

  RestRoute capture;
  capture.Method = "POST";
  capture.Path = "/reports/images";
  capture.Summary = "Capture a render window as an image for reports";
  capture.Description =
    "Body: {window, one of axial, sagittal, coronal, 3d; and optional: max_size (default 1000), the longest side in "
    "pixels; background (default transparent, which suits any page), a color like #ffffff, or window for the "
    "window's own; decorations (default false), whether the window's corner texts, logo and border show; scale, 2 "
    "to 4, for sharper images with a max_size to match; crop (default false), true cuts the image down to the "
    "part of the scene that is not background; padding, pixels of background kept around it}, as GET "
    "/api/v1/rendering/windows/{id}/screenshot takes them. Cropped images of a series, like one view at several "
    "time points, differ in size and position. Set the window up first: slice, visible data, crosshair, camera. The image is kept under a name that "
    "reports show it by, like <img src=\"image-3.png\">; the header X-MITK-Report-Image gives {name, image_size, "
    "timepoint_ms}, the time point the window showed. Only the last 100 captured images are kept for new reports; a "
    "report keeps the images it shows. Returns the image to check it, at most 1000 pixels on its longest side.";
  capture.Handler = [server](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto window = body.contains("window") && body["window"].is_string() ? body["window"].get<std::string>() : std::string();

    if (window.empty() || !std::all_of(window.begin(), window.end(), [](unsigned char c) { return std::isalnum(c) != 0 || c == '_' || c == '-'; }))
      throw RestException(ErrorResponse::InvalidRequest("window must name a render window, like axial or 3d.", request.Path));

    if (body.contains("decorations") && !body["decorations"].is_boolean())
      throw RestException(ErrorResponse::InvalidRequest("decorations must be true or false.", request.Path));

    // Reports show the scene without the window's corner texts, logo and border by default.
    auto target = "/api/v1/rendering/windows/" + window + "/screenshot?decorations=" +
      (body.value("decorations", false) ? "true" : "false");

    for (const auto* name : { "max_size", "scale", "padding" })
    {
      if (!body.contains(name))
        continue;

      if (!body[name].is_number_integer())
        throw RestException(ErrorResponse::InvalidRequest(std::string(name) + " must be a whole number.", request.Path));

      // Passed on as written, so that the screenshot endpoint refuses a number beyond its range.
      target += std::string("&") + name + "=" + body[name].dump();
    }

    if (body.contains("crop"))
    {
      if (!body["crop"].is_boolean())
        throw RestException(ErrorResponse::InvalidRequest("crop must be true or false.", request.Path));

      if (body["crop"].get<bool>())
        target += "&crop=true";
    }

    // Transparent by default: the window's background, wherever the scene does not
    // fill the window, would otherwise frame the image on the report's page.
    const auto background = body.contains("background") && body["background"].is_string()
      ? body["background"].get<std::string>()
      : std::string(body.contains("background") ? "" : "transparent");
    const bool isColor = background.size() == 7 && background[0] == '#' &&
      std::all_of(background.begin() + 1, background.end(), [](unsigned char c) { return std::isxdigit(c) != 0; });

    if (background != "transparent" && background != "window" && !isColor)
      throw RestException(ErrorResponse::InvalidRequest("background must be transparent, window or a color like #ffffff.", request.Path));

    if (background != "window")
      target += "&background=" + (isColor ? "%23" + background.substr(1) : background);

    // One way to capture windows: the screenshot endpoint's.
    RestRequest screenshot;
    screenshot.Method = "GET";
    screenshot.SetTarget(target);
    screenshot.ClientAddress = request.ClientAddress;

    if (const auto authorization = request.GetHeader("Authorization"); authorization.has_value())
      screenshot.Headers["authorization"] = *authorization;

    const auto response = server->HandleRequest(screenshot);

    if (response.Status >= 400)
      return response;

    const auto png = QByteArray::fromStdString(response.Body);
    auto image = QImage::fromData(png, "PNG");

    if (image.isNull())
      throw RestException(ErrorResponse::InternalError("The render window gave no image.", request.Path));

    const auto name = CallInGuiThread(*server, [&]() { return GetStore(request.Path)->AddImage(png); });
    nlohmann::json info = { { "name", name.toStdString() }, { "image_size", { image.width(), image.height() } } };

    if (const auto header = response.Headers.find("X-MITK-Screenshot"); header != response.Headers.end())
    {
      const auto window = nlohmann::json::parse(header->second, nullptr, false);

      if (window.is_object() && window.contains("timepoint_ms"))
        info["timepoint_ms"] = window["timepoint_ms"];
    }

    auto preview = response.Body;
    const auto longest = std::max(image.width(), image.height());

    if (longest > MaxPreviewSize)
    {
      image = image.scaled(MaxPreviewSize, MaxPreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      preview = EncodePng(image);
    }

    auto result = RestResponse::FromBytes(std::move(preview), "image/png");
    result.Headers["X-MITK-Report-Image"] = info.dump();
    return result;
  };
  routes.push_back(capture);

  return routes;
}
