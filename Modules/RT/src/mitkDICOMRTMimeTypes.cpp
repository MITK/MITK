/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMRTMimeTypes.h>

#include <mitkIOMimeTypes.h>

#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMFileReader.h>

#include <itksys/SystemTools.hxx>



namespace mitk
{

std::array<std::unique_ptr<CustomMimeType>, 3> DICOMRTMimeTypes::Get()
{
  return {
    std::make_unique<RTDoseMimeType>(),
    std::make_unique<RTPlanMimeType>(),
    std::make_unique<RTStructMimeType>()
  };
}

// Mime Types

DICOMRTMimeTypes::RTDoseMimeType::RTDoseMimeType()
  : CustomMimeType(DICOMRT_DOSE_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTDose");

  this->AddExtension("dcm");
}

bool DICOMRTMimeTypes::RTDoseMimeType::AppliesTo(const std::string &path) const
{
  bool canRead( CustomMimeType::AppliesTo(path) );

  if (!canRead)
  {
    return false;
  }

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be parsed or further identifyed.
  //so as a work arround we just return the current canRead if the file does not exist.
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return canRead;
  }
  // end fix for bug 18572

  auto modality = GetModality(path);
  if (modality == "RTDOSE")
  {
    return canReadByDicomFileReader(path);
  }

  return false;
}

std::string DICOMRTMimeTypes::GetModality(const std::string & path)
{
  const auto modalityTagPath = DICOMTagPath(0x0008, 0x0060);

  mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
  scanner->SetInputFiles({ path });
  scanner->AddTagPaths({ modalityTagPath });
  scanner->Scan();

  mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
  std::string modality = "";
  if (frames.empty())
    return modality;
  auto findings = frames.front()->GetTagValueAsString(modalityTagPath);

  modality = findings.front().value;
  return modality;
}

bool DICOMRTMimeTypes::canReadByDicomFileReader(const std::string & filename)
{
  mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();
  selector->LoadBuiltIn3DConfigs();
  selector->SetInputFiles({ filename });

  mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();

  return reader.IsNotNull();
}

DICOMRTMimeTypes::RTDoseMimeType* DICOMRTMimeTypes::RTDoseMimeType::Clone() const
{
  return new RTDoseMimeType(*this);
}

DICOMRTMimeTypes::RTStructMimeType::RTStructMimeType()
  : CustomMimeType(DICOMRT_STRUCT_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTStruct");

  this->AddExtension("dcm");
}

bool DICOMRTMimeTypes::RTStructMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  if (!canRead) {
    return false;
  }

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be parsed or further identifyed.
  //so as a work arround we just return the current canRead if the file does not exist.
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return canRead;
  }
  // end fix for bug 18572

  auto modality = GetModality(path);
  if (modality == "RTSTRUCT") {
    return true;
  }

  return false;
}

DICOMRTMimeTypes::RTStructMimeType* DICOMRTMimeTypes::RTStructMimeType::Clone() const
{
  return new RTStructMimeType(*this);
}

DICOMRTMimeTypes::RTPlanMimeType::RTPlanMimeType()
  : CustomMimeType(DICOMRT_PLAN_MIMETYPE_NAME())
{
  std::string category = "DICOMRT";
  this->SetCategory(category);
  this->SetComment("RTPLAN");

  this->AddExtension("dcm");
}

bool DICOMRTMimeTypes::RTPlanMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  if (!canRead) {
    return false;
  }

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be parsed or further identifyed.
  //so as a work arround we just return the current canRead if the file does not exist.
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return canRead;
  }
  // end fix for bug 18572

  auto modality = GetModality(path);
  if (modality == "RTPLAN") {
    return true;
  }

  return false;
}

DICOMRTMimeTypes::RTPlanMimeType* DICOMRTMimeTypes::RTPlanMimeType::Clone() const
{
  return new RTPlanMimeType(*this);
}


DICOMRTMimeTypes::RTDoseMimeType DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE()
{
  return RTDoseMimeType();
}

DICOMRTMimeTypes::RTStructMimeType DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE()
{
  return RTStructMimeType();
}

DICOMRTMimeTypes::RTPlanMimeType DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE()
{
  return RTPlanMimeType();
}

// Names
std::string DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.dose";
}

std::string DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.struct";
}

std::string DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".dicomrt.plan";
}

// Descriptions

std::string DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE_DESCRIPTION()
{
  return "RTDOSE reader";
}

std::string DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_DESCRIPTION()
{
  return "RTSTRUCT reader";
}

std::string DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE_DESCRIPTION()
{
  return "RTPLAN reader";
}

}
