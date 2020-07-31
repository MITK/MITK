/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceEvent.h>

#include "mitkDICOMSegmentationIO.h"
#include <mitkDICOMTagsOfInterestAddHelper.h>

#include <mitkDICOMSegIOMimeTypes.h>

namespace mitk
{
  /**
  \brief Registers services for multilabel dicom module.
  */
  class DICOMQIIOActivator : public us::ModuleActivator
  {
    std::vector<AbstractFileIO *> m_FileIOs;
    DICOMTagsOfInterestAddHelper m_TagHelper;

  public:
    void Load(us::ModuleContext * context) override
    {
      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 10;

      std::vector<mitk::CustomMimeType *> mimeTypes = mitk::MitkDICOMSEGIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType *>::const_iterator mimeTypeIter = mimeTypes.begin(),
        iterEnd = mimeTypes.end();
        mimeTypeIter != iterEnd;
        ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_FileIOs.push_back(new DICOMSegmentationIO());

      DICOMTagsOfInterestAddHelper::TagsOfInterestVector tags = DICOMSegmentationIO::GetDICOMTagsOfInterest();
      m_TagHelper.Activate(context, tags);
    }

    void Unload(us::ModuleContext *) override
    {
      for (auto &elem : m_FileIOs)
      {
        delete elem;
      }
      m_TagHelper.Deactivate();
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMQIIOActivator)
