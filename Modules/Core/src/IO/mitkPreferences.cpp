/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPreferences.h"

#include <mitkIPreferencesStorage.h>
#include <mitkExceptionMacro.h>

#include <boost/algorithm/string.hpp>
#include <boost/beast/core/detail/base64.hpp>

mitk::Preferences::Preferences(const Properties& properties, const std::string& name, Preferences* parent, IPreferencesStorage* storage)
  : m_Properties(properties),
    m_Path(parent != nullptr ? parent->AbsolutePath() + (parent->AbsolutePath() == "/" ? "" : "/") + name : "/"),
    m_Name(name),
    m_Parent(parent),
    m_Root(parent != nullptr ? parent->m_Root : this),
    m_Storage(storage)
{
  if (storage == nullptr)
    mitkThrow() << "Storage must not be null!";

  if (parent != nullptr)
    parent->m_Children.emplace_back(this);
}

mitk::Preferences::~Preferences()
{
}

std::string mitk::Preferences::Get(const std::string& key, const std::string& def) const
{
  auto iter = m_Properties.find(key);

  return iter != m_Properties.end()
    ? iter->second
    : def;
}

void mitk::Preferences::Put(const std::string& key, const std::string& value)
{
  this->Put<std::string>(key, value, [](const auto& value) { return value; });
}

int mitk::Preferences::GetInt(const std::string& key, int def) const
{
  auto iter = m_Properties.find(key);

  if (iter == m_Properties.end())
    return def;

  try
  {
    return std::stoi(iter->second);
  }
  catch (...)
  {
    mitkThrow() << "The property [\"" << key << "\": \"" << iter->second << "\"] does not represent a valid int value!";
  }
}

void mitk::Preferences::PutInt(const std::string& key, int value)
{
  this->Put<int>(key, value, [](const auto& value) { return std::to_string(value); });
}

bool mitk::Preferences::GetBool(const std::string& key, bool def) const
{
  auto iter = m_Properties.find(key);

  return iter != m_Properties.end()
    ? boost::algorithm::to_lower_copy(iter->second) == "true"
    : def;
}

void mitk::Preferences::PutBool(const std::string& key, bool value)
{
  this->Put<bool>(key, value, [](const auto& value) { return value ? "true" : "false"; });
}

float mitk::Preferences::GetFloat(const std::string& key, float def) const
{
  auto iter = m_Properties.find(key);

  if (iter == m_Properties.end())
    return def;

  try
  {
    return std::stof(iter->second);
  }
  catch (...)
  {
    mitkThrow() << "The property [\"" << key << "\": \"" << iter->second << "\"] does not represent a valid float value!";
  }
}

void mitk::Preferences::PutFloat(const std::string& key, float value)
{
  this->Put<float>(key, value, [](const auto& value) { return std::to_string(value); });
}

double mitk::Preferences::GetDouble(const std::string& key, double def) const
{
  auto iter = m_Properties.find(key);

  if (iter == m_Properties.end())
    return def;

  try
  {
    return std::stod(iter->second);
  }
  catch (...)
  {
    mitkThrow() << "The property [\"" << key << "\": \"" << iter->second << "\"] does not represent a valid double value!";
  }
}

void mitk::Preferences::PutDouble(const std::string& key, double value)
{
  this->Put<double>(key, value, [](const auto& value) { return std::to_string(value); });
}

std::vector<std::byte> mitk::Preferences::GetByteArray(const std::string& key, const std::byte* def, size_t size) const
{
  using namespace boost::beast::detail;

  auto iter = m_Properties.find(key);

  if (iter == m_Properties.end())
  {
    std::vector<std::byte> array(size);
    std::copy(def, def + size, array.data());

    return array;
  }

  const auto& encodedArray = iter->second;
  std::vector<std::byte> array(base64::decoded_size(encodedArray.size()));
  auto sizes = base64::decode(array.data(), encodedArray.data(), encodedArray.size());

  array.resize(sizes.first);

  return array;
}

void mitk::Preferences::PutByteArray(const std::string& key, const std::byte* array, size_t size)
{
  this->Put<std::pair<decltype(array), decltype(size)>>(key, std::make_pair(array, size), [](const auto& value) {
    using namespace boost::beast::detail;

    std::vector<char> encodedArray(base64::encoded_size(value.second) + 1, '\0');
    base64::encode(encodedArray.data(), value.first, value.second);

    return std::string(encodedArray.data());
  });
}

void mitk::Preferences::Remove(const std::string& key)
{
  m_Properties.erase(key);
  this->OnChanged(this);
}

void mitk::Preferences::Clear()
{
  m_Properties.clear();
  this->OnChanged(this);
}

std::vector<std::string> mitk::Preferences::Keys() const
{
  std::vector<std::string> keys;

  for (const auto& property : m_Properties)
    keys.push_back(property.first);

  return keys;
}

std::vector<std::string> mitk::Preferences::ChildrenNames() const
{
  std::vector<std::string> names;

  for (const auto& child : m_Children)
    names.push_back(child->Name());

  return names;
}

mitk::IPreferences* mitk::Preferences::Parent()
{
  return m_Parent;
}

const mitk::IPreferences* mitk::Preferences::Parent() const
{
  return m_Parent;
}

mitk::IPreferences* mitk::Preferences::Root()
{
  return m_Root;
}

const mitk::IPreferences* mitk::Preferences::Root() const
{
  return m_Root;
}

mitk::IPreferences* mitk::Preferences::Node(const std::string& path)
{
  if (path.empty())
    return this;

  if (path[0] == '/')
    return m_Root->Node(path.substr(1));

  std::string name;
  std::string remainingPath;

  auto pos = path.find('/');

  if (pos != std::string::npos)
  {
    name = path.substr(0, pos);
    remainingPath = path.substr(pos + 1);
  }
  else
  {
    name = path;
  }

  IPreferences* node = nullptr;

  for (const auto& child : m_Children)
  {
    if (child->m_Name == name)
    {
      node = child.get();
      break;
    }
  }

  if (node == nullptr)
  {
    node = new Preferences(Properties(), name, this, m_Storage);
    this->OnChanged(this);
  }

  if (pos != std::string::npos)
    node = node->Node(remainingPath);

  return node;
}

void mitk::Preferences::RemoveNode()
{
  auto& siblings = m_Parent->m_Children;
  auto isMyself = [this](const std::unique_ptr<Preferences>& sibling) { return sibling.get() == this; };

  siblings.erase(std::find_if(siblings.begin(), siblings.end(), isMyself));
}

std::string mitk::Preferences::Name() const
{
  return m_Name;
}

std::string mitk::Preferences::AbsolutePath() const
{
  return m_Path;
}

void mitk::Preferences::Flush()
{
  m_Storage->Flush();
}

const mitk::Preferences::Properties& mitk::Preferences::GetProperties() const
{
  return m_Properties;
}

const std::vector<std::unique_ptr<mitk::Preferences>>& mitk::Preferences::GetChildren() const
{
  return m_Children;
}
