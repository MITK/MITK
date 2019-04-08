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

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceEvent.h>

#include "mitkDICOMSegmentationIO.h"

#include <mitkDICOMSegIOMimeTypes.h>

namespace mitk
{
  /**
  \brief Registers services for multilabel dicom module.
  */
  class DICOMQIIOActivator : public us::ModuleActivator
  {
    std::vector<AbstractFileIO *> m_FileIOs;

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
    }
    void Unload(us::ModuleContext *) override
    {
      for (auto &elem : m_FileIOs)
      {
        delete elem;
      }
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMQIIOActivator)
