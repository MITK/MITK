/*============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center (DKFZ)
  All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

============================================================================*/

#include "usModuleManifest_p.h"

#include <nlohmann/json.hpp>

#include <stdexcept>

using namespace nlohmann;

US_BEGIN_NAMESPACE

namespace {

  typedef std::map<std::string, Any> AnyMap;
  typedef std::vector<Any> AnyVector;

  void ParseJsonObject(const json& jsonObject, AnyMap& anyMap);
  void ParseJsonArray(const json& jsonArray, AnyVector& anyVector);

  Any ParseJsonValue(const json& jsonValue)
  {
    if (jsonValue.is_object())
    {
      Any any = AnyMap();
      ParseJsonObject(jsonValue, ref_any_cast<AnyMap>(any));
      return any;
    }
    else if (jsonValue.is_array())
    {
      Any any = AnyVector();
      ParseJsonArray(jsonValue, ref_any_cast<AnyVector>(any));
      return any;
    }
    else if (jsonValue.is_string())
    {
      return Any(jsonValue.get<std::string>());
    }
    else if (jsonValue.is_boolean())
    {
      return Any(jsonValue.get<bool>());
    }
    else if (jsonValue.is_number_integer())
    {
      return Any(jsonValue.get<int>());
    }
    else if (jsonValue.is_number_float())
    {
      return Any(jsonValue.get<double>());
    }

    return Any();
  }

  void ParseJsonObject(const json& jsonObject, AnyMap& anyMap)
  {
    for (const auto& [key, jsonValue] : jsonObject.items())
    {
      Any anyValue = ParseJsonValue(jsonValue);
      if (!anyValue.Empty())
      {
        anyMap.insert(std::make_pair(key, anyValue));
      }
    }
  }

  void ParseJsonArray(const json& jsonArray, AnyVector& anyVector)
  {
    for (const auto& jsonValue : jsonArray)
    {
      Any anyValue = ParseJsonValue(jsonValue);
      if (!anyValue.Empty())
      {
        anyVector.push_back(anyValue);
      }
    }
  }

}

ModuleManifest::ModuleManifest()
{
}

void ModuleManifest::Parse(std::istream& is)
{
  json root;

  try
  {
    root = json::parse(is);
  }
  catch (const json::exception& e)
  {
    throw std::runtime_error(e.what());
  }

  if (!root.is_object())
  {
    throw std::runtime_error("The Json root element must be an object.");
  }

  ParseJsonObject(root, m_Properties);
}

bool ModuleManifest::Contains(const std::string& key) const
{
  return m_Properties.count(key) > 0;
}

Any ModuleManifest::GetValue(const std::string& key) const
{
  AnyMap::const_iterator iter = m_Properties.find(key);
  if (iter != m_Properties.end())
  {
    return iter->second;
  }
  return Any();
}

std::vector<std::string> ModuleManifest::GetKeys() const
{
  std::vector<std::string> keys;
  for (AnyMap::const_iterator iter = m_Properties.begin();
       iter != m_Properties.end(); ++iter)
  {
    keys.push_back(iter->first);
  }
  return keys;
}

void ModuleManifest::SetValue(const std::string& key, const Any& value)
{
  m_Properties[key] = value;
}

US_END_NAMESPACE
