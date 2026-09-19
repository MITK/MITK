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
#include <set>
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

  // The providers whose mappers CreateMapper() elects for data of the given
  // class hierarchy, over all slots, in application order. Only they
  // contribute default properties: a base class provider outranked by a more
  // derived registration never renders the node, so its defaults would only
  // leak into a mapper that does not know them.
  std::vector<Entry> GetElectedCandidates(const std::vector<std::string> &classHierarchy) const
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::set<MapperSlotId> slots;
    for (const auto &indexEntry : m_MapperIndex)
      slots.insert(indexEntry.first.second);

    std::vector<Entry> elected;

    for (const auto slotId : slots)
    {
      for (const auto &className : classHierarchy)
      {
        auto it = m_MapperIndex.find(std::make_pair(className, slotId));
        if (it == m_MapperIndex.end())
          continue;

        // One provider object may serve several slots; apply its defaults once.
        for (const auto &entry : it->second)
        {
          if (std::none_of(elected.begin(), elected.end(), [&entry](const Entry &other) {
                return other.provider == entry.provider;
              }))
          {
            elected.push_back(entry);
          }
        }

        break;
      }
    }

    std::sort(elected.begin(), elected.end(), ByApplicationOrder);
    return elected;
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
    auto &candidates = m_MapperIndex[std::make_pair(dataType, static_cast<MapperSlotId>(slotId))];
    candidates.insert(std::lower_bound(candidates.begin(), candidates.end(), entry, ByElectionOrder), entry);

    return provider;
  }

  void ModifiedService(const ServiceReferenceType &, TrackedType) override
  {
  }

  void RemovedService(const ServiceReferenceType &reference, TrackedType) override
  {
    {
      // Identify entries by service id, not by provider pointer: one provider
      // object may hold several registrations, e.g. one per mapper slot.
      const auto serviceId = us::any_cast<long>(reference.GetProperty(us::ServiceConstants::SERVICE_ID()));

      std::lock_guard<std::mutex> lock(m_Mutex);
      this->RemoveRegistration(serviceId);
    }
    m_Context->UngetService(reference);
  }

  // Order in which CreateMapper() tries candidates: highest ranking first, so
  // the preferred provider gets the first chance to accept a node.
  static bool ByElectionOrder(const Entry &lhs, const Entry &rhs)
  {
    return lhs.ranking != rhs.ranking ? lhs.ranking > rhs.ranking : lhs.serviceId < rhs.serviceId;
  }

  // Order in which ApplyDefaultProperties() applies the elected providers:
  // lowest ranking first, so the preferred provider writes last and wins
  // conflicting values.
  static bool ByApplicationOrder(const Entry &lhs, const Entry &rhs)
  {
    return lhs.ranking != rhs.ranking ? lhs.ranking < rhs.ranking : lhs.serviceId < rhs.serviceId;
  }

  void RemoveRegistration(long serviceId)
  {
    for (auto it = m_MapperIndex.begin(); it != m_MapperIndex.end();)
    {
      auto &entries = it->second;
      entries.erase(std::remove_if(entries.begin(), entries.end(), [serviceId](const Entry &entry) {
        return entry.serviceId == serviceId;
      }), entries.end());

      it = entries.empty() ? m_MapperIndex.erase(it) : std::next(it);
    }
  }

  us::ServiceTracker<IMapperProvider> *m_Tracker = nullptr;
  us::ModuleContext *m_Context = nullptr;

  // Mapper candidates per data class and slot in election order, with
  // registration order as deterministic tie-breaker.
  std::map<std::pair<std::string, MapperSlotId>, std::vector<Entry>> m_MapperIndex;
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

  for (const auto &entry : m_Impl->GetElectedCandidates(data->GetClassHierarchy()))
    entry.provider->SetDefaultProperties(node);
}
