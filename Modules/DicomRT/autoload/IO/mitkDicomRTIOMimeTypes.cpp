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

#include "mitkDicomRTIOMimeTypes.h"

#include "mitkIOMimeTypes.h"

#include "mitkDICOMDCMTKTagScanner.h"
#include "mitkDICOMTagPath.h"
#include "mitkDICOMTagsOfInterestService.h"
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMFileReader.h>

#include <itksys/SystemTools.hxx>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

namespace mitk
{

std::vector<CustomMimeType*> DicomRTIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)
  mimeTypes.push_back(DICOMRT_DOSE_MIMETYPE().Clone());
  mimeTypes.push_back(DICOMRT_PLAN_MIMETYPE().Clone());
  mimeTypes.push_back(DICOMRT_STRUCT_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types

DicomRTIOMimeTypes::RTDoseMimeType::RTDoseMimeType()
  : CustomMimeType(DICOMRT_DOSE_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTDose");

  this->AddExtension("dcm");
}

bool DicomRTIOMimeTypes::RTDoseMimeType::AppliesTo(const std::string &path) const
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

std::string DicomRTIOMimeTypes::GetModality(const std::string & path)
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

bool DicomRTIOMimeTypes::canReadByDicomFileReader(const std::string & filename)
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

DicomRTIOMimeTypes::RTDoseMimeType* DicomRTIOMimeTypes::RTDoseMimeType::Clone() const
{
  return new RTDoseMimeType(*this);
}

DicomRTIOMimeTypes::RTStructMimeType::RTStructMimeType()
  : CustomMimeType(DICOMRT_STRUCT_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTStruct");

  this->AddExtension("dcm");
}

bool DicomRTIOMimeTypes::RTStructMimeType::AppliesTo(const std::string &path) const
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

DicomRTIOMimeTypes::RTStructMimeType* DicomRTIOMimeTypes::RTStructMimeType::Clone() const
{
  return new RTStructMimeType(*this);
}

DicomRTIOMimeTypes::RTPlanMimeType::RTPlanMimeType()
  : CustomMimeType(DICOMRT_PLAN_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTPLAN");

  this->AddExtension("dcm");
}

bool DicomRTIOMimeTypes::RTPlanMimeType::AppliesTo(const std::string &path) const
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

DicomRTIOMimeTypes::RTPlanMimeType* DicomRTIOMimeTypes::RTPlanMimeType::Clone() const
{
  return new RTPlanMimeType(*this);
}


DicomRTIOMimeTypes::RTDoseMimeType DicomRTIOMimeTypes::DICOMRT_DOSE_MIMETYPE()
{
  return RTDoseMimeType();
}

DicomRTIOMimeTypes::RTStructMimeType DicomRTIOMimeTypes::DICOMRT_STRUCT_MIMETYPE()
{
  return RTStructMimeType();
}

DicomRTIOMimeTypes::RTPlanMimeType DicomRTIOMimeTypes::DICOMRT_PLAN_MIMETYPE()
{
  return RTPlanMimeType();
}

// Names
std::string DicomRTIOMimeTypes::DICOMRT_DOSE_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.dose";
  return name;
}

std::string DicomRTIOMimeTypes::DICOMRT_STRUCT_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.struct";
  return name;
}

std::string DicomRTIOMimeTypes::DICOMRT_PLAN_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.plan";
  return name;
}

// Descriptions

std::string DicomRTIOMimeTypes::DICOMRT_DOSE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "RTDOSE reader";
  return description;
}

std::string DicomRTIOMimeTypes::DICOMRT_STRUCT_MIMETYPE_DESCRIPTION()
{
  static std::string description = "RTSTRUCT reader";
  return description;
}

std::string DicomRTIOMimeTypes::DICOMRT_PLAN_MIMETYPE_DESCRIPTION()
{
  static std::string description = "RTPLAN reader";
  return description;
}

mitk::IDICOMTagsOfInterest* DicomRTIOMimeTypes::GetDicomTagsOfInterestService()
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
