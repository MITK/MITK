/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMimeTypeProvider_h
#define mitkMimeTypeProvider_h

#include "mitkCustomMimeType.h"
#include "mitkIMimeTypeProvider.h"

#include "usServiceTracker.h"
#include "usServiceTrackerCustomizer.h"

#include <set>

namespace mitk
{
  struct MimeTypeTrackerTypeTraits : public us::TrackedTypeTraitsBase<MimeType, MimeTypeTrackerTypeTraits>
  {
    typedef MimeType TrackedType;

    static bool IsValid(const TrackedType &t) { return t.IsValid(); }
    static TrackedType DefaultValue() { return TrackedType(); }
    static void Dispose(TrackedType & /*t*/) {}
  };

  class MimeTypeProvider : public IMimeTypeProvider, private us::ServiceTrackerCustomizer<CustomMimeType, MimeType>
  {
  public:
    MimeTypeProvider();
    ~MimeTypeProvider() override;

    void Start();
    void Stop();

    std::vector<MimeType> GetMimeTypes() const override;
    std::vector<MimeType> GetMimeTypesForFile(const std::string &filePath) const override;
    std::vector<MimeType> GetMimeTypesForCategory(const std::string &category) const override;
    MimeType GetMimeTypeForName(const std::string &name) const override;

    std::vector<std::string> GetCategories() const override;

  private:
    TrackedType AddingService(const ServiceReferenceType &reference) override;
    void ModifiedService(const ServiceReferenceType &reference, TrackedType service) override;
    void RemovedService(const ServiceReferenceType &reference, TrackedType service) override;

    MimeType GetMimeType(const ServiceReferenceType &reference) const;

    us::ServiceTracker<CustomMimeType, MimeTypeTrackerTypeTraits> *m_Tracker;

    typedef std::map<std::string, std::set<MimeType>> MapType;
    MapType m_NameToMimeTypes;

    std::map<std::string, MimeType> m_NameToMimeType;
  };
}

#endif
