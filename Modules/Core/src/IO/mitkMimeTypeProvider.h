/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKMIMETYPEPROVIDER_H
#define MITKMIMETYPEPROVIDER_H

#include "mitkIMimeTypeProvider.h"
#include "mitkCustomMimeType.h"

#include "usServiceTracker.h"
#include "usServiceTrackerCustomizer.h"

#include <set>

namespace mitk {

struct MimeTypeTrackerTypeTraits : public us::TrackedTypeTraitsBase<MimeType,MimeTypeTrackerTypeTraits>
{
  typedef MimeType TrackedType;

  static bool IsValid(const TrackedType& t)
  {
    return t.IsValid();
  }

  static TrackedType DefaultValue()
  {
    return TrackedType();
  }

  static void Dispose(TrackedType& /*t*/)
  {
  }
};

class MimeTypeProvider : public IMimeTypeProvider,
    private us::ServiceTrackerCustomizer<CustomMimeType, MimeType>
{
public:

  MimeTypeProvider();
  ~MimeTypeProvider();

  void Start();
  void Stop();

  virtual std::vector<MimeType> GetMimeTypes() const;
  virtual std::vector<MimeType> GetMimeTypesForFile(const std::string& filePath) const;
  virtual std::vector<MimeType> GetMimeTypesForCategory(const std::string& category) const;
  virtual MimeType GetMimeTypeForName(const std::string& name) const;

  virtual std::vector<std::string> GetCategories() const;

private:

  virtual TrackedType AddingService(const ServiceReferenceType& reference);
  virtual void ModifiedService(const ServiceReferenceType& reference, TrackedType service);
  virtual void RemovedService(const ServiceReferenceType& reference, TrackedType service);

  MimeType GetMimeType(const ServiceReferenceType& reference) const;

  us::ServiceTracker<CustomMimeType, MimeTypeTrackerTypeTraits>* m_Tracker;

  typedef std::map<std::string, std::set<MimeType> > MapType;
  MapType m_NameToMimeTypes;

  std::map<std::string, MimeType> m_NameToMimeType;
};

}

#endif // MITKMIMETYPEPROVIDER_H
