/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestRoute.h>

#include <mitkErrorResponse.h>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>

#include <algorithm>
#include <cctype>

namespace
{
  std::string ToLower(std::string text)
  {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });

    return text;
  }
}

void mitk::RestRequest::SetTarget(const std::string& target)
{
  const auto queryStart = target.find('?');

  Path = httplib::decode_path_component(target.substr(0, queryStart));
  QueryParams.clear();

  if (queryStart != std::string::npos)
  {
    httplib::Params params;
    httplib::detail::parse_query_text(target.substr(queryStart + 1), params);
    QueryParams.insert(params.begin(), params.end());
  }
}

std::optional<std::string> mitk::RestRequest::GetQueryParam(const std::string& name) const
{
  const auto param = QueryParams.find(name);

  if (param == QueryParams.end())
    return std::nullopt;

  return param->second;
}

std::optional<std::string> mitk::RestRequest::GetHeader(const std::string& name) const
{
  const auto header = Headers.find(ToLower(name));

  if (header == Headers.end())
    return std::nullopt;

  return header->second;
}

std::optional<std::string> mitk::RestRequest::GetBearerToken() const
{
  static const std::string Prefix = "Bearer ";
  const auto header = this->GetHeader("Authorization");

  if (!header.has_value() || header->size() <= Prefix.size() || 0 != header->compare(0, Prefix.size(), Prefix))
    return std::nullopt;

  return header->substr(Prefix.size());
}

nlohmann::json mitk::RestRequest::GetJsonBody() const
{
  if (Body.empty())
    return nlohmann::json::object();

  try
  {
    return nlohmann::json::parse(Body);
  }
  catch (const nlohmann::json::parse_error& e)
  {
    throw RestException(ErrorResponse::InvalidRequest(std::string("Invalid JSON: ") + e.what(), Path));
  }
}

mitk::RestResponse mitk::RestResponse::FromData(const nlohmann::json& data, int status)
{
  RestResponse response;
  response.Status = status;
  response.ContentType = "application/json";
  response.Body = nlohmann::json{ { "data", data } }.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
  return response;
}

mitk::RestResponse mitk::RestResponse::FromError(const nlohmann::json& error)
{
  RestResponse response;
  response.Status = error.contains("error") ? error["error"].value("status", 500) : 500;
  response.ContentType = "application/json";
  response.Body = error.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
  return response;
}

mitk::RestResponse mitk::RestResponse::FromBytes(std::string bytes, const std::string& contentType)
{
  RestResponse response;
  response.ContentType = contentType;
  response.Body = std::move(bytes);
  return response;
}

mitk::RestResponse mitk::RestResponse::NoContent()
{
  RestResponse response;
  response.Status = 204;
  return response;
}

bool mitk::RestResponse::HasJsonBody() const
{
  return ContentType.find("application/json") != std::string::npos;
}

mitk::RestException::RestException(const nlohmann::json& error)
  : std::runtime_error(error.contains("error") ? error["error"].value("message", std::string()) : error.dump()),
    m_Error(error)
{
}

const nlohmann::json& mitk::RestException::GetError() const
{
  return m_Error;
}
