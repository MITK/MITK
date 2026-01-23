/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageService.h"

#include <mitkStandaloneDataStorage.h>

namespace mitk
{

const std::string DataStorageService::DEFAULT_LABEL = "Default DataStorage";

DataStorageService::DataStorageService()
{
  auto defaultStorage = StandaloneDataStorage::New();
  m_DefaultStorage = DataStorageReference(DEFAULT_LABEL, defaultStorage.GetPointer(), true);
  // m_ActiveLabel empty means use default
}

DataStorageService::~DataStorageService()
{
}

DataStorage::Pointer DataStorageService::GetDefaultDataStorage() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_DefaultStorage.GetStorage();
}

DataStorage::Pointer DataStorageService::GetActiveDataStorage() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!m_ActiveLabel.empty())
  {
    auto info = this->FindStorageByLabel(m_ActiveLabel);
    if (info.IsValid())
    {
      return info.GetStorage();
    }
  }
  return m_DefaultStorage.GetStorage();
}

DataStorageReference DataStorageService::GetActiveDataStorageReference() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (!m_ActiveLabel.empty())
  {
    auto info = this->FindStorageByLabel(m_ActiveLabel);
    if (info.IsValid())
    {
      return info;
    }
  }
  return m_DefaultStorage;
}

bool DataStorageService::SetActiveDataStorage(const std::string& label)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (label.empty() || label == DEFAULT_LABEL)
  {
    m_ActiveLabel.clear();
    return true;
  }

  // Check that the label exists
  if (this->FindStorageByLabel(label).IsValid())
  {
    m_ActiveLabel = label;
    return true;
  }

  return false;
}

bool DataStorageService::AddDataStorage(const std::string& label, DataStorage::Pointer storage)
{
  if (label.empty() || storage.IsNull())
  {
    return false;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // Check for duplicate label
  if (label == DEFAULT_LABEL || this->FindStorageByLabel(label).IsValid())
  {
    return false;
  }

  m_Storages.emplace_back(label, storage);
  return true;
}

std::optional<DataStorageReference> DataStorageService::CreateDataStorage(const std::string& label)
{
  if (label.empty())
  {
    return std::nullopt;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // Check for duplicate label
  if (label == DEFAULT_LABEL || this->FindStorageByLabel(label).IsValid())
  {
    return std::nullopt;
  }

  auto storage = StandaloneDataStorage::New();
  DataStorageReference dsRef(label, storage.GetPointer(), false);
  m_Storages.push_back(dsRef);

  return dsRef;
}

std::optional<DataStorageReference> DataStorageService::GetDataStorageReference(const std::string& label) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (label == DEFAULT_LABEL || m_DefaultStorage.GetLabel() == label)
  {
    return m_DefaultStorage;
  }

  auto info = this->FindStorageByLabel(label);
  if (info.IsValid())
  {
    return info;
  }

  return std::nullopt;
}

std::optional<std::string> DataStorageService::GetLabel(const DataStorage* storage) const
{
  if (storage == nullptr)
  {
    return std::nullopt;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_DefaultStorage.GetStorage().GetPointer() == storage)
  {
    return m_DefaultStorage.GetLabel();
  }

  for (const auto& info : m_Storages)
  {
    if (info.GetStorage().GetPointer() == storage)
    {
      return info.GetLabel();
    }
  }

  return std::nullopt;
}

std::vector<std::string> DataStorageService::GetAllLabels() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  std::vector<std::string> result;
  result.reserve(m_Storages.size() + 1);
  result.push_back(m_DefaultStorage.GetLabel());

  for (const auto& info : m_Storages)
  {
    result.push_back(info.GetLabel());
  }

  return result;
}

std::vector<DataStorageReference> DataStorageService::GetAllDataStorages() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  std::vector<DataStorageReference> result;
  result.reserve(m_Storages.size() + 1);
  result.push_back(m_DefaultStorage);
  result.insert(result.end(), m_Storages.begin(), m_Storages.end());

  return result;
}

bool DataStorageService::HasDataStorage(const std::string& label) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (label == DEFAULT_LABEL || m_DefaultStorage.GetLabel() == label)
  {
    return true;
  }

  return this->FindStorageByLabel(label).IsValid();
}

bool DataStorageService::RemoveDataStorage(const std::string& label)
{
  if (label.empty())
  {
    return false;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);

  // Cannot remove default storage
  if (label == DEFAULT_LABEL || label == m_DefaultStorage.GetLabel())
  {
    return false;
  }

  // Find and remove the storage
  for (auto it = m_Storages.begin(); it != m_Storages.end(); ++it)
  {
    if (it->GetLabel() == label)
    {
      // If this was the active storage, reset to default
      if (m_ActiveLabel == label)
      {
        m_ActiveLabel.clear();
      }
      m_Storages.erase(it);
      return true;
    }
  }

  return false;
}

DataStorageReference DataStorageService::FindStorageByLabel(const std::string& label) const
{
  // Note: caller must hold m_Mutex
  for (const auto& info : m_Storages)
  {
    if (info.GetLabel() == label)
    {
      return info;
    }
  }
  return DataStorageReference();
}

} // namespace mitk
