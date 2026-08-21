/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMapperProviderRegistry.h>

#include <mitkBaseData.h>
#include <mitkDataNode.h>
#include <mitkIMapperProvider.h>
#include <mitkLog.h>

#include <usModuleContext.h>
#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>

#include <algorithm>
#include <map>
#include <mutex>
#include <vector>

class mitk::MapperProviderRegistry::Impl : private us::ServiceTrackerCustomizer<IMapperProvider>
{
public:
  struct Entry
  {
    IMapperProvider *provider;
    int ranking;
    long serviceId;
  };

  Impl() = default;

  ~Impl() override
  {
    delete m_Tracker;
  }

  void Start(us::ModuleContext *context)
  {
    if (m_Tracker == nullptr)
    {
      m_Context = context;
      m_Tracker = new us::ServiceTracker<IMapperProvider>(context, this);
    }
    m_Tracker->Open();
  }

  void Stop()
  {
    if (m_Tracker != nullptr)
      m_Tracker->Close();
  }

  std::vector<Entry> GetMapperCandidates(const std::string &className, MapperSlotId slotId) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_MapperIndex.find(std::make_pair(className, slotId));
    return it != m_MapperIndex.end() ? it->second : std::vector<Entry>();
  }

  std::vector<Entry> GetDefaultsCandidates(const std::string &className) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_DefaultsIndex.find(className);
    return it != m_DefaultsIndex.end() ? it->second : std::vector<Entry>();
  }

private:
  TrackedType AddingService(const ServiceReferenceType &reference) override
  {
    std::string dataType;
    int slotId = 0;
    int ranking = 0;
    long serviceId = 0;

    try
    {
      dataType = us::any_cast<std::string>(reference.GetProperty(IMapperProvider::PROP_BASEDATA_TYPE()));
      slotId = us::any_cast<int>(reference.GetProperty(IMapperProvider::PROP_SLOT_ID()));

      us::Any rankingProperty = reference.GetProperty(us::ServiceConstants::SERVICE_RANKING());
      if (!rankingProperty.Empty())
        ranking = us::any_cast<int>(rankingProperty);

      serviceId = us::any_cast<long>(reference.GetProperty(us::ServiceConstants::SERVICE_ID()));
    }
    catch (const us::BadAnyCastException &e)
    {
      MITK_WARN << "Ignoring IMapperProvider service with invalid properties: " << e.what();
      return nullptr;
    }

    if (dataType.empty())
    {
      MITK_WARN << "Ignoring IMapperProvider service with empty data type property.";
      return nullptr;
    }

    auto provider = m_Context->GetService(reference);
    if (provider == nullptr)
      return nullptr;

    const Entry entry{provider, ranking, serviceId};

    std::lock_guard<std::mutex> lock(m_Mutex);
    this->InsertSorted(m_MapperIndex[std::make_pair(dataType, static_cast<MapperSlotId>(slotId))], entry);
    this->InsertSorted(m_DefaultsIndex[dataType], entry);

    return provider;
  }

  void ModifiedService(const ServiceReferenceType &, TrackedType) override
  {
  }

  void RemovedService(const ServiceReferenceType &reference, TrackedType provider) override
  {
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      this->RemoveProvider(m_MapperIndex, provider);
      this->RemoveProvider(m_DefaultsIndex, provider);
    }
    m_Context->UngetService(reference);
  }

  static void InsertSorted(std::vector<Entry> &entries, const Entry &entry)
  {
    auto position = std::lower_bound(entries.begin(), entries.end(), entry, [](const Entry &lhs, const Entry &rhs) {
      return lhs.ranking != rhs.ranking ? lhs.ranking > rhs.ranking : lhs.serviceId < rhs.serviceId;
    });
    entries.insert(position, entry);
  }

  template <typename TIndex>
  static void RemoveProvider(TIndex &index, const IMapperProvider *provider)
  {
    for (auto it = index.begin(); it != index.end();)
    {
      auto &entries = it->second;
      entries.erase(std::remove_if(entries.begin(), entries.end(), [provider](const Entry &entry) {
        return entry.provider == provider;
      }), entries.end());

      it = entries.empty() ? index.erase(it) : std::next(it);
    }
  }

  us::ServiceTracker<IMapperProvider> *m_Tracker = nullptr;
  us::ModuleContext *m_Context = nullptr;

  // Candidates per key, sorted by ranking (descending), then registration
  // order (ascending) for deterministic tie-breaking.
  std::map<std::pair<std::string, MapperSlotId>, std::vector<Entry>> m_MapperIndex;
  std::map<std::string, std::vector<Entry>> m_DefaultsIndex;
  mutable std::mutex m_Mutex;
};

mitk::MapperProviderRegistry::MapperProviderRegistry()
  : m_Impl(std::make_unique<Impl>())
{
}

mitk::MapperProviderRegistry::~MapperProviderRegistry()
{
}

mitk::MapperProviderRegistry &mitk::MapperProviderRegistry::GetInstance()
{
  static MapperProviderRegistry instance;
  return instance;
}

void mitk::MapperProviderRegistry::Start(us::ModuleContext *context)
{
  m_Impl->Start(context);
}

void mitk::MapperProviderRegistry::Stop()
{
  m_Impl->Stop();
}

mitk::Mapper::Pointer mitk::MapperProviderRegistry::CreateMapper(DataNode *node, MapperSlotId slotId) const
{
  if (node == nullptr)
    return nullptr;

  const auto *data = node->GetData();
  if (data == nullptr)
    return nullptr;

  for (const auto &className : data->GetClassHierarchy())
  {
    for (const auto &entry : m_Impl->GetMapperCandidates(className, slotId))
    {
      auto mapper = entry.provider->CreateMapper(node);
      if (mapper.IsNotNull())
        return mapper;
    }
  }

  return nullptr;
}

void mitk::MapperProviderRegistry::ApplyDefaultProperties(DataNode *node) const
{
  if (node == nullptr)
    return;

  const auto *data = node->GetData();
  if (data == nullptr)
    return;

  for (const auto &className : data->GetClassHierarchy())
  {
    for (const auto &entry : m_Impl->GetDefaultsCandidates(className))
      entry.provider->SetDefaultProperties(node);
  }
}
