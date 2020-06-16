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

#include "mitkCESTIOMimeTypes.h"

namespace mitk
{
  void CESTIOActivator::Load(us::ModuleContext *context)
  {
    us::ServiceProperties props;
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;

    m_MimeTypes = mitk::MitkCESTIOMimeTypes::Get();
    for (auto& mimeType : m_MimeTypes)
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

    m_CESTDICOMReader.reset(new CESTDICOMReaderService());
    m_CESTDICOMManualWithMetaFileReader.reset(new CESTDICOMManualReaderService(MitkCESTIOMimeTypes::CEST_DICOM_WITH_META_FILE_MIMETYPE(), "CEST DICOM Manual Reader"));
    m_CESTDICOMManualWithOutMetaFileReader.reset(new CESTDICOMManualReaderService(MitkCESTIOMimeTypes::CEST_DICOM_WITHOUT_META_FILE_MIMETYPE(), "CEST DICOM Manual Reader"));
  }

  void CESTIOActivator::Unload(us::ModuleContext *)
  {
    for (auto& elem : m_MimeTypes)
    {
      delete elem;
    }
  }
}

US_EXPORT_MODULE_ACTIVATOR(mitk::CESTIOActivator)
