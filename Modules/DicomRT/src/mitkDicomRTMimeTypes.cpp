/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDicomRTMimeTypes.h>

#include <mitkIOMimeTypes.h>

#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMFileReader.h>

#include <itksys/SystemTools.hxx>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

namespace mitk
{

std::array<std::unique_ptr<CustomMimeType>, 3> DicomRTMimeTypes::Get()
{
  return {
    std::make_unique<RTDoseMimeType>(),
    std::make_unique<RTPlanMimeType>(),
    std::make_unique<RTStructMimeType>()
  };
}

// Mime Types

DicomRTMimeTypes::RTDoseMimeType::RTDoseMimeType()
  : CustomMimeType(DICOMRT_DOSE_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTDose");

  this->AddExtension("dcm");
}

bool DicomRTMimeTypes::RTDoseMimeType::AppliesTo(const std::string &path) const
{
  bool canRead( CustomMimeType::AppliesTo(path) );

  if (!canRead) {
    return false;
  }

  if (!canReadByDicomFileReader(path)) {
    return false;
  }

  auto modality = GetModality(path);

  if (modality == "RTDOSE") {
    return true;
  }
  else {
    return false;
    }
}

std::string DicomRTMimeTypes::GetModality(const std::string & path)
{
  mitk::IDICOMTagsOfInterest* toiSrv = GetDicomTagsOfInterestService();

  auto tagsOfInterest = toiSrv->GetTagsOfInterest();

  DICOMTagPathList tagsOfInterestList;
  for (const auto& tag : tagsOfInterest) {
    tagsOfInterestList.push_back(tag.first);
  }

  mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
  scanner->SetInputFiles({ path });
  scanner->AddTagPaths(tagsOfInterestList);
  scanner->Scan();

  mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
  std::string modality = "";
  if (frames.empty())
    return modality;
  auto findings = frames.front()->GetTagValueAsString(DICOMTagPath(0x0008, 0x0060));

  modality = findings.front().value;
  return modality;
}

bool DicomRTMimeTypes::canReadByDicomFileReader(const std::string & filename)
{
  mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();
  selector->LoadBuiltIn3DConfigs();
  selector->SetInputFiles({ filename });

  mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();

  if (reader.IsNull()) {
    return false;
  }
  else {
    return true;
  }
}

DicomRTMimeTypes::RTDoseMimeType* DicomRTMimeTypes::RTDoseMimeType::Clone() const
{
  return new RTDoseMimeType(*this);
}

DicomRTMimeTypes::RTStructMimeType::RTStructMimeType()
  : CustomMimeType(DICOMRT_STRUCT_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTStruct");

  this->AddExtension("dcm");
}

bool DicomRTMimeTypes::RTStructMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  if (!canRead) {
    return false;
  }

  auto modality = GetModality(path);
  if (modality == "RTSTRUCT") {
    return true;
  }
  else {
    return false;
  }
}

DicomRTMimeTypes::RTStructMimeType* DicomRTMimeTypes::RTStructMimeType::Clone() const
{
  return new RTStructMimeType(*this);
}

DicomRTMimeTypes::RTPlanMimeType::RTPlanMimeType()
  : CustomMimeType(DICOMRT_PLAN_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTPLAN");

  this->AddExtension("dcm");
}

bool DicomRTMimeTypes::RTPlanMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  if (!canRead) {
    return false;
  }

  auto modality = GetModality(path);
    if (modality == "RTPLAN") {
      return true;
    }
    else {
      return false;
    }
}

DicomRTMimeTypes::RTPlanMimeType* DicomRTMimeTypes::RTPlanMimeType::Clone() const
{
  return new RTPlanMimeType(*this);
}


DicomRTMimeTypes::RTDoseMimeType DicomRTMimeTypes::DICOMRT_DOSE_MIMETYPE()
{
  return RTDoseMimeType();
}

DicomRTMimeTypes::RTStructMimeType DicomRTMimeTypes::DICOMRT_STRUCT_MIMETYPE()
{
  return RTStructMimeType();
}

DicomRTMimeTypes::RTPlanMimeType DicomRTMimeTypes::DICOMRT_PLAN_MIMETYPE()
{
  return RTPlanMimeType();
}

// Names
std::string DicomRTMimeTypes::DICOMRT_DOSE_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.dose";
}

std::string DicomRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.struct";
}

std::string DicomRTMimeTypes::DICOMRT_PLAN_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.plan";
}

// Descriptions

std::string DicomRTMimeTypes::DICOMRT_DOSE_MIMETYPE_DESCRIPTION()
{
  return "RTDOSE reader";
}

std::string DicomRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_DESCRIPTION()
{
  return "RTSTRUCT reader";
}

std::string DicomRTMimeTypes::DICOMRT_PLAN_MIMETYPE_DESCRIPTION()
{
  return "RTPLAN reader";
}

mitk::IDICOMTagsOfInterest* DicomRTMimeTypes::GetDicomTagsOfInterestService()
{
  mitk::IDICOMTagsOfInterest* result = nullptr;

  std::vector<us::ServiceReference<mitk::IDICOMTagsOfInterest> > toiRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
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

}
