/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTIOActivator.h"

#include "mitkCESTDICOMReaderService.h"
#include "mitkCESTGenericDICOMReaderService.h"

#include <usModuleContext.h>
#include <mitkDICOMIOHelper.h>

#include "mitkCESTIOMimeTypes.h"

#include <mutex>

namespace mitk
{
  void CESTIOActivator::Load(us::ModuleContext *context)
  {
    us::ServiceProperties props;
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;

    m_MimeTypes = mitk::MitkCESTIOMimeTypes::Get();
    for (const auto& mimeType : m_MimeTypes)
    {
      if (mimeType->GetName() == mitk::MitkCESTIOMimeTypes::CEST_DICOM_WITHOUT_META_FILE_NAME())
      { // "w/o meta" mimetype should only registered with low priority.
        context->RegisterService(mimeType);
      }
      else
      {
        context->RegisterService(mimeType, props);
      }
    }

    m_CESTDICOMReader = std::make_unique<CESTDICOMReaderService>();
    m_CESTDICOMManualWithMetaFileReader = std::make_unique<CESTDICOMManualReaderService>(MitkCESTIOMimeTypes::CEST_DICOM_WITH_META_FILE_MIMETYPE(), "CEST DICOM Manual Reader");
    m_CESTDICOMManualWithOutMetaFileReader = std::make_unique<CESTDICOMManualReaderService>(MitkCESTIOMimeTypes::CEST_DICOM_WITHOUT_META_FILE_MIMETYPE(), "CEST DICOM Manual Reader");

    m_Context = context;

    DICOMTagsOfInterestAddHelper::TagsOfInterestVector tags = { mitk::DICOM_IMAGING_FREQUENCY_PATH() };
    m_TagHelper.Activate(m_Context, tags);
  }

  void CESTIOActivator::Unload(us::ModuleContext *)
  {
    for (auto& elem : m_MimeTypes)
    {
      delete elem;
      elem = nullptr;
    }
    m_TagHelper.Deactivate();
  }
}

US_EXPORT_MODULE_ACTIVATOR(mitk::CESTIOActivator)
