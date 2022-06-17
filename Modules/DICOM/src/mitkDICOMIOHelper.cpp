/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMIOHelper.h>

#include <mitkImage.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk::DICOMIOHelper
{
  IDICOMTagsOfInterest* GetTagsOfInterestService()
  {
    auto* context = us::GetModuleContext();

    if (context == nullptr)
    {
      MITK_WARN << "No MitkDICOM module context found.";
      return nullptr;
    }

    auto toiRegisters = context->GetServiceReferences<mitk::IDICOMTagsOfInterest>();

    if (!toiRegisters.empty())
    {
      if (toiRegisters.size() > 1)
        MITK_WARN << "Multiple DICOM tags of interest services found. Using just one.";

      return us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(toiRegisters.front());
    }

    return nullptr;
  }

  FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest,
                                                const DICOMDatasetAccessingImageFrameList& frames)
  {
    std::vector<DICOMDatasetAccess::FindingsListType> findings;

    for (const auto& entry : pathsOfInterest)
      findings.push_back(frames.front()->GetTagValueAsString(entry));

    return findings;
  }

  void SetProperties(BaseData* image, const FindingsListVectorType& findings)
  {
    for (const auto& finding : findings)
    {
      for (const auto& entry : finding)
      {
        auto property = TemporoSpatialStringProperty::New();
        property->SetValue(entry.value);

        image->SetProperty(DICOMTagPathToPropertyName(entry.path), property);
      }
    }
  }
}
