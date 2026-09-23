/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMGenericImageFrameInfo.h>
#include <mitkException.h>

#include <algorithm>
#include <map>
#include <optional>

/**
 * The values of one file, indexed so that a path query costs one key comparison
 * per distinct normalized path, and so that a frame-scoped query then visits
 * only its own frame's entries plus the frame-independent ones.
 *
 * For a file without functional groups the normalized path is the path, every
 * bucket holds exactly one entry, and the container is the std::map keyed by
 * DICOMTagPath that this class has always used.
 */
class mitk::DICOMGenericImageFrameInfo::ValueStore
{
public:
  struct Entry
  {
    /** As found in the file, explicit. */
    DICOMTagPath path;
    /** Empty unless functional-group rooted. */
    DICOMTagPath frameRelativePath;
    std::string value;
    DICOMFindingOrigin origin = DICOMFindingOrigin::TopLevel;
    /** The k of a (5200,9230)[k] root. */
    std::optional<unsigned int> perFrameItem;
  };

  void Set(const DICOMTagPath& explicitPath, const std::string& value)
  {
    Entry entry;
    entry.path = explicitPath;
    entry.value = value;

    // A path that is not functional-group rooted is stored under itself, which
    // is what this class has always done: one bucket, one entry, one path copy.
    if (!IsFunctionalGroupRooted(explicitPath))
    {
      this->Insert(explicitPath, std::move(entry));
      return;
    }

    entry.frameRelativePath = FunctionalGroupRelativePath(explicitPath);

    const auto& nodes = explicitPath.GetNodes();
    const auto& root = nodes.front();

    DICOMTagPath normalized;
    if (0x9230 == root.tag.GetElement())
    {
      // All frames' entries for one attribute share a bucket key, so the key is
      // compared once per attribute rather than once per attribute and frame;
      // the item selector is what tells them apart inside the bucket.
      // A shared-group entry keeps its own (5200,9229) bucket.
      entry.perFrameItem = static_cast<unsigned int>(root.selection);
      entry.origin = DICOMFindingOrigin::PerFrameFunctionalGroup;
      normalized.AddNode(DICOMTagPath::NodeInfo(root.tag, DICOMTagPath::NodeInfo::NodeType::AnySelection));
    }
    else
    {
      entry.origin = DICOMFindingOrigin::SharedFunctionalGroup;
      normalized.AddNode(root);
    }

    for (auto node = nodes.cbegin() + 1; node != nodes.cend(); ++node)
    {
      normalized.AddNode(*node);
    }

    this->Insert(normalized, std::move(entry));
  }

  /**
   * Entries for one attribute, split by the frame item they came from.
   *
   * A frame-scoped query touches its own slot and the frame-independent ones
   * only, so neither a lookup nor an insert walks the other frames' entries.
   */
  struct Bucket
  {
    /** The bucket key relative to the functional-group item; empty for a key
        that is not functional-group rooted. */
    DICOMTagPath relativeKey;
    /** Top-level and (5200,9229) findings: they answer for every frame. */
    std::vector<Entry> frameIndependent;
    /** (5200,9230)[k] findings, keyed by k. */
    std::map<unsigned int, std::vector<Entry>> byFrameItem;
  };

  /**
   * \param query          The path to match.
   * \param onlyFrameItem  Unset for a file-level info, whose query is compared
   *                       with the stored path only. Set for a frame-scoped
   *                       info, whose query is compared with the stored path of
   *                       a top-level entry and with the frame-relative path of a
   *                       functional-group entry; per-frame entries of other
   *                       items are then not visited at all.
   */
  std::vector<const Entry*> Matching(const DICOMTagPath& query,
                                     const std::optional<unsigned int>& onlyFrameItem = std::nullopt) const
  {
    std::vector<const Entry*> result;

    for (const auto& [normalized, bucket] : m_ByNormalizedPath)
    {
      // The comparison has to happen at the bucket key as well: a
      // frame-relative query can never equal a rooted key, because both node
      // lists must end together.
      const bool relative = onlyFrameItem.has_value() && !bucket.relativeKey.IsEmpty();

      if (!query.Equals(relative ? bucket.relativeKey : normalized))
      {
        continue;
      }

      // The bucket key generalises the item selectors, so a query that names a
      // concrete item still has to be tested against each entry.
      const auto collect = [&](const std::vector<Entry>& entries)
      {
        for (const auto& entry : entries)
        {
          if (query.Equals(relative ? entry.frameRelativePath : entry.path))
          {
            result.push_back(&entry);
          }
        }
      };

      collect(bucket.frameIndependent);

      if (onlyFrameItem.has_value())
      {
        const auto slot = bucket.byFrameItem.find(*onlyFrameItem);
        if (bucket.byFrameItem.cend() != slot)
        {
          collect(slot->second);
        }
      }
      else
      {
        for (const auto& slot : bucket.byFrameItem)
        {
          collect(slot.second);
        }
      }
    }

    return result;
  }

  const Entry* Find(const DICOMTag& tag) const
  {
    const auto bucket = m_ByNormalizedPath.find(DICOMTagPath(tag));
    if (m_ByNormalizedPath.cend() == bucket || bucket->second.frameIndependent.empty())
    {
      return nullptr;
    }

    return &bucket->second.frameIndependent.front();
  }

  DICOMFrameLayout layout;

private:
  void Insert(const DICOMTagPath& normalized, Entry&& entry)
  {
    auto& bucket = m_ByNormalizedPath[normalized];
    if (!entry.frameRelativePath.IsEmpty() && bucket.relativeKey.IsEmpty())
    {
      bucket.relativeKey = FunctionalGroupRelativePath(normalized);
    }
    auto& entries = entry.perFrameItem.has_value() ? bucket.byFrameItem[*entry.perFrameItem]
                                                   : bucket.frameIndependent;

    const auto existing = std::find_if(entries.begin(), entries.end(),
                                       [&](const Entry& candidate) { return candidate.path == entry.path; });
    if (existing != entries.end())
    {
      *existing = std::move(entry);
    }
    else
    {
      entries.push_back(std::move(entry));
    }
  }

  std::map<DICOMTagPath, Bucket> m_ByNormalizedPath;
};

mitk::DICOMGenericImageFrameInfo
::DICOMGenericImageFrameInfo(const std::string& filename, unsigned int frameNo)
:DICOMDatasetAccessingImageFrameInfo(filename, frameNo),
 m_Store(std::make_shared<ValueStore>())
{
}

mitk::DICOMGenericImageFrameInfo
::DICOMGenericImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo)
:DICOMDatasetAccessingImageFrameInfo(frameinfo->Filename, frameinfo->FrameNo),
 m_Store(std::make_shared<ValueStore>())
{
}

mitk::DICOMGenericImageFrameInfo::
~DICOMGenericImageFrameInfo()
{
}

mitk::DICOMGenericImageFrameInfo::Pointer
mitk::DICOMGenericImageFrameInfo::NewFrameScoped(const std::string& filename,
                                                 unsigned int frameNo,
                                                 ValueStorePointer store)
{
  if (nullptr == store)
  {
    mitkThrow() << "Cannot create a frame scoped image frame info without a value store. File: " << filename;
  }

  Pointer result = new Self(filename, frameNo);
  result->UnRegister();
  result->m_Store = store;
  result->m_FrameScoped = true;

  return result;
}

mitk::DICOMDatasetFinding
mitk::DICOMGenericImageFrameInfo
::GetTagValueAsString(const DICOMTag& tag) const
{
  DICOMDatasetFinding result;

  if (const auto* entry = m_Store->Find(tag))
  {
    result.isValid = true;
    result.value = entry->value;
    result.path = entry->path;
  }

  return result;
}

mitk::DICOMDatasetAccess::FindingsListType
mitk::DICOMGenericImageFrameInfo::GetTagValueAsString(const DICOMTagPath& path) const
{
  FindingsListType result;

  const auto onlyFrameItem = m_FrameScoped ? std::optional<unsigned int>(this->FrameNo)
                                           : std::nullopt;

  for (const auto* entry : m_Store->Matching(path, onlyFrameItem))
  {
    // Only a functional-group finding has a frame-relative path; every other
    // one reports the path it was found at, in both views.
    const bool reportRelative = m_FrameScoped && !entry->frameRelativePath.IsEmpty();
    result.emplace_back(true, entry->value, reportRelative ? entry->frameRelativePath : entry->path, entry->origin);
  }

  return result;
}

void
mitk::DICOMGenericImageFrameInfo::SetTagValue(const DICOMTagPath& path, const std::string& value)
{
  if (!path.IsExplicit())
  {
    mitkThrow() << "Only explicit tag paths (no wildcards) are allowed for tag values in DICOMGenericImageFrameInfo. Passed tag path:" << path.ToStr();
  }

  m_Store->Set(path, value);
}

std::string
mitk::DICOMGenericImageFrameInfo
::GetFilenameIfAvailable() const
{
  return this->Filename;
}

const mitk::DICOMFrameLayout& mitk::DICOMGenericImageFrameInfo::GetFrameLayout() const
{
  return m_Store->layout;
}

void mitk::DICOMGenericImageFrameInfo::SetFrameLayout(const DICOMFrameLayout& layout)
{
  m_Store->layout = layout;
}

mitk::DICOMGenericImageFrameInfo::ValueStorePointer mitk::DICOMGenericImageFrameInfo::GetStore() const
{
  return m_Store;
}
