/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiManualRoutes.h"
#include "QmitkUserManual.h"

#include <mitkErrorResponse.h>
#include <mitkRestGuiThread.h>

#include <charconv>
#include <memory>

namespace
{
  nlohmann::json ToJson(const QmitkUserManual::Section& section)
  {
    nlohmann::json json = {
      { "id", section.Id },
      { "page", section.Page.toStdString() },
      { "title", section.Title.toStdString() }
    };

    if (!section.Text.isEmpty())
    {
      json["text"] = section.Text.toStdString();
      json["cut_off"] = section.IsCut;
    }

    return json;
  }

  /** Loads the manual in the GUI thread on first use and hands it to \p function there. */
  template <typename Function>
  auto UseManual(const mitk::IRestServerService& server, const std::shared_ptr<QmitkUserManual>& manual,
    ctkPluginContext* context, const std::string& path, Function function)
  {
    return mitk::CallInGuiThread(server, [&]() {
      const auto error = manual->Load(context);

      if (!error.isEmpty())
        throw mitk::RestException(mitk::ErrorResponse::InternalError(error.toStdString(), path));

      // Without this, every search would look like one that found nothing.
      if (manual->IsEmpty())
      {
        throw mitk::RestException(mitk::ErrorResponse::Create("MANUAL_NOT_AVAILABLE", "Service Unavailable",
          "This build of the application has no user manual.", 503, path));
      }

      return function(*manual);
    });
  }
}

std::vector<mitk::RestRoute> mitk::CreateManualRoutes(const IRestServerService* server, ctkPluginContext* context)
{
  auto manual = std::make_shared<QmitkUserManual>();

  RestRoute search;
  search.Method = "GET";
  search.Path = "/manual/search";
  search.Summary = "Search the user manual";
  search.Description =
    "The manual is the one the help view shows: it explains the views and tools, their options and how "
    "they behave. Query parameter: query, words to search for (at least three letters each), like "
    "\"nnInteractive auto refine\". Returns {data: {matches, sections: [{id, page, title, text, cut_off}], "
    "other_sections: [{id, page, title}]}}: the best matches with their text, then the next ones. Read "
    "those with GET /api/v1/manual/sections/{id}.";
  search.Handler = [server, manual, context](const RestRequest& request) {
    const auto query = request.GetQueryParam("query");

    if (!query.has_value())
      throw RestException(ErrorResponse::InvalidRequest("The query parameter \"query\" is missing.", request.Path));

    const auto result = UseManual(*server, manual, context, request.Path, [&query](const QmitkUserManual& userManual) {
      return userManual.Search(QString::fromStdString(*query));
    });

    if (result.WordCount == 0)
      throw RestException(ErrorResponse::InvalidRequest("The query needs a word of at least three letters.", request.Path));

    auto sections = nlohmann::json::array();
    auto otherSections = nlohmann::json::array();

    for (const auto& section : result.Sections)
      sections.push_back(ToJson(section));

    for (const auto& section : result.OtherSections)
      otherSections.push_back(ToJson(section));

    return RestResponse::FromData({
      { "matches", result.MatchCount },
      { "sections", sections },
      { "other_sections", otherSections }
    });
  };

  RestRoute section;
  section.Method = "GET";
  section.Path = "/manual/sections/:id";
  section.Summary = "Read a section of the user manual";
  section.Description =
    "Path parameter: id, from a search. Returns {data: {id, page, title, text, cut_off}}; very long "
    "sections are cut off.";
  section.Handler = [server, manual, context](const RestRequest& request) {
    const auto& text = request.PathParams.at("id");
    const auto* textEnd = text.data() + text.size();
    std::size_t id = 0;

    if (const auto [end, error] = std::from_chars(text.data(), textEnd, id); error != std::errc() || end != textEnd)
      throw RestException(ErrorResponse::InvalidRequest("The section id must be a number.", request.Path));

    const auto result = UseManual(*server, manual, context, request.Path, [id](const QmitkUserManual& userManual) {
      return userManual.GetSection(id);
    });

    if (!result.has_value())
    {
      throw RestException(ErrorResponse::Create("NOT_FOUND", "Not Found",
        "There is no section " + std::to_string(id) + " in the manual.", 404, request.Path));
    }

    return RestResponse::FromData(ToJson(*result));
  };

  return { search, section };
}
