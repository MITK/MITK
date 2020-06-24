/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCESTIOActivator_H
#define MITKCESTIOActivator_H

#include <mitkCustomMimeType.h>
#include <mitkIDICOMTagsOfInterest.h>

#include <usModuleActivator.h>
#include <usServiceEvent.h>

#include <memory>
#include <mutex>

namespace mitk
{
  struct IFileReader;
  class IDICOMTagsOfInterest;

  class CESTIOActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *context) override;
    void Unload(us::ModuleContext *context) override;


  private:
    void RegisterTagsOfInterest(IDICOMTagsOfInterest* toiService) const;
    void DICOMTagsOfInterestServiceChanged(const us::ServiceEvent event);

    std::unique_ptr<IFileReader> m_CESTDICOMReader;
    std::unique_ptr<IFileReader> m_CESTDICOMManualWithMetaFileReader;
    std::unique_ptr<IFileReader> m_CESTDICOMManualWithOutMetaFileReader;
    std::vector<mitk::CustomMimeType *> m_MimeTypes;

    // Module context
    us::ModuleContext* m_Context;
    /**mutex to guard the service listening */
    std::mutex m_Mutex;
  };
}

#endif // MITKCESTIOActivator_H
