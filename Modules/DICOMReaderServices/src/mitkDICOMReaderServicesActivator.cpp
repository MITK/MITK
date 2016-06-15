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

#include "mitkDICOMReaderServicesActivator.h"

#include "mitkAutoSelectingDICOMReaderService.h"
#include "mitkClassicDICOMSeriesReaderService.h"
#include "mitkDICOMTagsOfInterestService.h"

#include "mitkDICOMTagHelper.h"

#include <usModuleContext.h>

namespace mitk {

  void DICOMReaderServicesActivator::Load(us::ModuleContext* context)
  {
    m_AutoSelectingDICOMReader.reset(new AutoSelectingDICOMReaderService());
    m_ClassicDICOMSeriesReader.reset(new ClassicDICOMSeriesReaderService());

    m_DICOMTagsOfInterestService.reset(new DICOMTagsOfInterestService());
    context->RegisterService<mitk::IDICOMTagsOfInterest>(m_DICOMTagsOfInterestService.get());

    DefaultDICOMTagMapType tagmap = GetDefaultDICOMTagsOfInterest();
    for (auto tag : tagmap)
    {
      m_DICOMTagsOfInterestService->AddTagOfInterest(tag.second);
    }
  }

  void DICOMReaderServicesActivator::Unload(us::ModuleContext*)
  {
  }

}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMReaderServicesActivator)
