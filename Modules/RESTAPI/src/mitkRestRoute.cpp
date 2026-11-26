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

  /** The length of the valid UTF-8 sequence at the position, or 0 if none starts there. */
  std::size_t GetUtf8SequenceLength(const std::string& text, std::size_t position)
  {
    const auto byte = [&text](std::size_t i) { return static_cast<unsigned char>(text[i]); };
    const auto lead = byte(position);

    if (lead < 0x80)
      return 1;

    std::size_t length = 0;
    unsigned char low = 0x80;
    unsigned char high = 0xBF;

    // The bounds of the second byte rule out overlong forms, surrogates and code points above U+10FFFF.
    if (lead >= 0xC2 && lead <= 0xDF)
    {
      length = 2;
    }
    else if (lead >= 0xE0 && lead <= 0xEF)
    {
      length = 3;
      low = lead == 0xE0 ? 0xA0 : 0x80;
      high = lead == 0xED ? 0x9F : 0xBF;
    }
    else if (lead >= 0xF0 && lead <= 0xF4)
    {
      length = 4;
      low = lead == 0xF0 ? 0x90 : 0x80;
      high = lead == 0xF4 ? 0x8F : 0xBF;
    }
    else
    {
      return 0;
    }

    if (position + length > text.size() || byte(position + 1) < low || byte(position + 1) > high)
      return 0;

    for (std::size_t i = 2; i < length; ++i)
    {
      if (byte(position + i) < 0x80 || byte(position + i) > 0xBF)
        return 0;
    }

    return length;
  }

  /** The text with every byte that is no part of a valid UTF-8 sequence read as Latin-1. */
  std::string ReadInvalidUtf8AsLatin1(const std::string& text)
  {
    std::string result;
    result.reserve(text.size() + text.size() / 4);

    for (std::size_t position = 0; position < text.size();)
    {
      if (const auto length = GetUtf8SequenceLength(text, position); length > 0)
      {
        result.append(text, position, length);
        position += length;
        continue;
      }

      const auto byte = static_cast<unsigned char>(text[position++]);
      result += static_cast<char>(0xC0 | (byte >> 6));
      result += static_cast<char>(0x80 | (byte & 0x3F));
    }

    return result;
  }

  nlohmann::json ReadInvalidUtf8AsLatin1(const nlohmann::json& json)
  {
    if (json.is_string())
      return ReadInvalidUtf8AsLatin1(json.get_ref<const std::string&>());

    if (json.is_array())
    {
      auto result = nlohmann::json::array();

      for (const auto& item : json)
        result.push_back(ReadInvalidUtf8AsLatin1(item));

      return result;
    }

    if (json.is_object())
    {
      auto result = nlohmann::json::object();

      for (const auto& [key, value] : json.items())
        result[ReadInvalidUtf8AsLatin1(key)] = ReadInvalidUtf8AsLatin1(value);

      return result;
    }

    return json;
  }
}

std::string mitk::DumpRestJson(const nlohmann::json& json)
{
  try
  {
    return json.dump();
  }
  catch (const nlohmann::json::type_error& e)
  {
    // 316: a string is not valid UTF-8. Only then is the copy worth making.
    if (e.id != 316)
      throw;

    return ReadInvalidUtf8AsLatin1(json).dump();
  }
}

void mitk::RestRequest::SetTarget(const std::string& target)
{
  // The server does not see a fragment over HTTP either.
  const auto withoutFragment = target.substr(0, target.find('#'));
  const auto queryStart = withoutFragment.find('?');

  Path = httplib::decode_path_component(withoutFragment.substr(0, queryStart));
  QueryParams.clear();

  if (queryStart != std::string::npos)
  {
    httplib::Params params;
    httplib::detail::parse_query_text(withoutFragment.substr(queryStart + 1), params);
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
  static const std::string Prefix = "bearer ";
  const auto header = this->GetHeader("Authorization");

  if (!header.has_value() || header->size() <= Prefix.size() || ToLower(header->substr(0, Prefix.size())) != Prefix)
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
  response.Body = DumpRestJson(nlohmann::json{ { "data", data } });
  return response;
}

mitk::RestResponse mitk::RestResponse::FromError(const nlohmann::json& error)
{
  RestResponse response;
  response.Status = 500;

  // Runs where exceptions become responses, so it must not throw for an error of another shape.
  if (error.is_object() && error.contains("error") && error["error"].is_object() && error["error"].contains("status"))
  {
    const auto& status = error["error"]["status"];

    if (status.is_number_integer() && status.get<std::int64_t>() >= 400 && status.get<std::int64_t>() <= 599)
      response.Status = status.get<int>();
  }
  response.ContentType = "application/json";
  response.Body = DumpRestJson(error);
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
  : std::runtime_error(error.contains("error") ? error["error"].value("message", std::string()) : DumpRestJson(error)),
    m_Error(error)
{
}

const nlohmann::json& mitk::RestException::GetError() const
{
  return m_Error;
}
