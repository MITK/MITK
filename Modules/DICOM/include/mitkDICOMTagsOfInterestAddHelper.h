/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagsOfInterestAddHelper_h
#define mitkDICOMTagsOfInterestAddHelper_h

#include <unordered_map>

#include <mitkDICOMTagPath.h>

#include <usServiceEvent.h>

#include <memory>
#include <mutex>

#include "MitkDICOMExports.h"

namespace us
{
  class ModuleContext;
}

namespace mitk
{
  class IDICOMTagsOfInterest;

  /** Helper class that can be used to ensure that a given list
   of DICOM tags will be registered at existing DICOMTagsOfInterest
   services. If the services are available the tags will be directly
   added. As long as the helper is active, it will add the given tags to
   all new DICOMTagsOfInterest services as soon as they are registered.
   This helper class is e.g. used in module activators where the
   DICOMTagsOfInterest service might not be registered when a module
   is loaded but the module wants to ensure that the tags are added
   as soon as possible.
   The helper must be deactivated before the module context used on activation
   gets invalid (e.g. in the Unload function of the module activator that
   uses the helper to ensure that the tags will be added).
  */
  class MITKDICOM_EXPORT DICOMTagsOfInterestAddHelper
  {
  public:
    using TagsOfInterestVector = std::vector<mitk::DICOMTagPath>;
    ~DICOMTagsOfInterestAddHelper();

    void Activate(us::ModuleContext* context, TagsOfInterestVector tags);
    void Deactivate();

  private:
    void RegisterTagsOfInterest(IDICOMTagsOfInterest* toiService) const;
    void DICOMTagsOfInterestServiceChanged(const us::ServiceEvent event);

    TagsOfInterestVector m_TagsOfInterest;

    bool m_Active = false;
    us::ModuleContext* m_Context = nullptr;

    /**mutex to guard the service listening */
    std::mutex m_Mutex;
  };
}

#endif
