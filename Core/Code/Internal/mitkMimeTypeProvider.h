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
#include "mitkIMimeType.h"

#include "usServiceTracker.h"
#include "usServiceTrackerCustomizer.h"

#include <set>

namespace mitk {

struct MimeTypeTrackerTypeTraits : public us::TrackedTypeTraitsBase<us::ServiceReference<IMimeType>,MimeTypeTrackerTypeTraits>
{
  typedef us::ServiceReference<IMimeType> TrackedType;

  static bool IsValid(const TrackedType& t)
  {
    return t;
  }

  static TrackedType DefaultValue()
  {
    return TrackedType();
  }

  static void Dispose(TrackedType& t)
  {
    t = 0;
  }
};

class MimeTypeProvider : public IMimeTypeProvider,
    private us::ServiceTrackerCustomizer<IMimeType, us::ServiceReference<IMimeType> >
{
public:

  MimeTypeProvider();

  void Start();
  void Stop();

  virtual std::vector<std::string> GetMimeTypes() const;
  virtual std::vector<std::string> GetMimeTypesForFile(const std::string& filePath) const;
  virtual std::vector<std::string> GetMimeTypesForExtension(const std::string& extension) const;
  virtual std::vector<std::string> GetMimeTypesForCategory(const std::string& category) const;

  virtual std::string GetDescription(const std::string& mimeType) const;

  virtual std::vector<std::string> GetExtensions(const std::string& mimeType) const;

  virtual std::string GetCategory(const std::string& mimeType) const;

  virtual std::vector<std::string> GetCategories() const;

private:

  virtual ServiceReferenceType AddingService(const ServiceReferenceType& reference);
  virtual void ModifiedService(const ServiceReferenceType& reference, ServiceReferenceType service);
  virtual void RemovedService(const ServiceReferenceType& reference, ServiceReferenceType service);

  us::ServiceTracker<IMimeType, MimeTypeTrackerTypeTraits>* m_Tracker;

  typedef std::map<std::string, std::set<us::ServiceReferenceU> > MapType;
  MapType m_MimeTypeToRefs;

};

}

#endif // MITKMIMETYPEPROVIDER_H
