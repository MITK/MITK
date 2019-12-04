/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTIOActivator.h"

#include "mitkCESTDICOMReaderService.h"

#include <usModuleContext.h>

#include "mitkCESTIOMimeTypes.h"

namespace mitk
{
  void CESTIOActivator::Load(us::ModuleContext *context)
  {
    us::ServiceProperties props;
    props[us::ServiceConstants::SERVICE_RANKING()] = 10;

    m_MimeTypes = mitk::MitkCESTIOMimeTypes::Get();
    for (std::vector<mitk::CustomMimeType *>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
                                                             iterEnd = m_MimeTypes.end();
         mimeTypeIter != iterEnd;
         ++mimeTypeIter)
    {
      context->RegisterService(*mimeTypeIter, props);
    }

    m_CESTDICOMReader.reset(new CESTDICOMReaderService());
  }

  void CESTIOActivator::Unload(us::ModuleContext *) {}
}

US_EXPORT_MODULE_ACTIVATOR(mitk::CESTIOActivator)
