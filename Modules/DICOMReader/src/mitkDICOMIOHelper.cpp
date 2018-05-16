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

#include "mitkDICOMIOHelper.h"

#include <mitkImage.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  mitk::IDICOMTagsOfInterest *GetDicomTagsOfInterestService()
  {
    mitk::IDICOMTagsOfInterest *result = nullptr;

    us::ModuleContext *context = us::GetModuleContext();
    if (context == nullptr)
    {
      MITK_WARN << "No dicom context found.";
      return result;
    }
    std::vector<us::ServiceReference<mitk::IDICOMTagsOfInterest>> toiRegisters =
      context->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
    if (!toiRegisters.empty())
    {
      if (toiRegisters.size() > 1)
      {
        MITK_WARN << "Multiple DICOM tags of interest services found. Using just one.";
      }
      result = us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(toiRegisters.front());
    }

    return result;
  }

  FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList &pathsOfInterest,
                                                const DICOMDatasetAccessingImageFrameList &frames)
  {
    std::vector<mitk::DICOMDatasetAccess::FindingsListType> findings;
    for (const auto &entry : pathsOfInterest)
    {
      findings.push_back(frames.front()->GetTagValueAsString(entry));
    }
    return findings;
  }

  void SetProperties(BaseDataPointer image, const FindingsListVectorType &findings)
  {
    for (const auto &finding : findings)
    {
      for (const auto &entry : finding)
      {
        const std::string propertyName = mitk::DICOMTagPathToPropertyName(entry.path);
        auto property = mitk::TemporoSpatialStringProperty::New();
        property->SetValue(entry.value);
        image->SetProperty(propertyName.c_str(), property);
      }
    }
  }
}
