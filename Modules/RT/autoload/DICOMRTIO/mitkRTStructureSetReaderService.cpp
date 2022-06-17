/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRTStructureSetReaderService.h"

#include <mitkDICOMRTMimeTypes.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMDCMTKTagScanner.h>

#include "dcmtk/dcmrt/drtstrct.h"

namespace mitk
{
  RTStructureSetReaderService::RTStructureSetReaderService() : AbstractFileReader(CustomMimeType(mitk::DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_NAME()), mitk::DICOMRTMimeTypes::DICOMRT_STRUCT_MIMETYPE_DESCRIPTION()) {
    m_FileReaderServiceReg = RegisterService();
  }

  RTStructureSetReaderService::RTStructureSetReaderService(const RTStructureSetReaderService& other) : mitk::AbstractFileReader(other)
  {
  }

  RTStructureSetReaderService::~RTStructureSetReaderService() {}

  RTStructureSetReaderService::RoiEntry::RoiEntry()
  {
    Number = 0;
    DisplayColor[0] = 1.0;
    DisplayColor[1] = 0.0;
    DisplayColor[2] = 0.0;
    ContourModelSet = mitk::ContourModelSet::New();
  }

  RTStructureSetReaderService::RoiEntry::RoiEntry(const RoiEntry& src)
  {
    Number = src.Number;
    Name = src.Name;
    Description = src.Description;
    DisplayColor[0] = src.DisplayColor[0];
    DisplayColor[1] = src.DisplayColor[1];
    DisplayColor[2] = src.DisplayColor[2];
    ContourModelSet = mitk::ContourModelSet::New();
    SetPolyData(src.ContourModelSet);
  }

  RTStructureSetReaderService::RoiEntry::~RoiEntry() {}

  RTStructureSetReaderService::RoiEntry& RTStructureSetReaderService::
    RoiEntry::operator =(const RoiEntry& src)
  {
    Number = src.Number;
    Name = src.Name;
    Description = src.Description;
    DisplayColor[0] = src.DisplayColor[0];
    DisplayColor[1] = src.DisplayColor[1];
    DisplayColor[2] = src.DisplayColor[2];
    SetPolyData(src.ContourModelSet);
    return (*this);
  }

  void RTStructureSetReaderService::RoiEntry::
    SetPolyData(mitk::ContourModelSet::Pointer roiPolyData)
  {
    if (roiPolyData == this->ContourModelSet)
    {
      return;
    }

    this->ContourModelSet = roiPolyData;
  }

  size_t RTStructureSetReaderService::GetNumberOfROIs() const
  {
    return this->ROISequenceVector.size();
  }

  RTStructureSetReaderService::RoiEntry* RTStructureSetReaderService::
    FindRoiByNumber(unsigned int roiNum)
  {
    for (unsigned int i = 0; i < this->ROISequenceVector.size(); ++i)
    {
      if (this->ROISequenceVector[i].Number == roiNum)
      {
        return &this->ROISequenceVector[i];
      }
    }

    return nullptr;
  }

  std::vector<itk::SmartPointer<BaseData> > RTStructureSetReaderService::DoRead()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;

    std::string location = GetInputLocation();

    auto DICOMTagsOfInterestService = DICOMIOHelper::GetTagsOfInterestService();
    auto tagsOfInterest = DICOMTagsOfInterestService->GetTagsOfInterest();
    DICOMTagPathList tagsOfInterestList;
    for (const auto& tag : tagsOfInterest) {
      tagsOfInterestList.push_back(tag.first);
    }

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
    scanner->SetInputFiles({ location });
    scanner->AddTagPaths(tagsOfInterestList);
    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    if (frames.empty()) {
      MITK_ERROR << "Error reading the RTSTRUCT file" << std::endl;
      return result;
    }

    auto findings = DICOMIOHelper::ExtractPathsOfInterest(tagsOfInterestList, frames);

    DcmFileFormat file;
    OFCondition output = file.loadFile(location.c_str(), EXS_Unknown);

    if (output.bad())
    {
      MITK_ERROR << "Can't read the file" << std::endl;
      return result;
    }

    DcmDataset* dataset = file.getDataset();

    DRTStructureSetIOD structureSetObject;
    OFCondition outp = structureSetObject.read(*dataset);

    if (!outp.good())
    {
      MITK_ERROR << "Error reading the file" << std::endl;
      return result;
    }

    DRTStructureSetROISequence& roiSequence =
      structureSetObject.getStructureSetROISequence();

    if (!roiSequence.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the structure sequence" << std::endl;
      return result;
    }

    do
    {
      DRTStructureSetROISequence::Item& currentSequence =
        roiSequence.getCurrentItem();

      if (!currentSequence.isValid())
      {
        continue;
      }

      OFString roiName;
      OFString roiDescription;
      Sint32 roiNumber;
      RoiEntry roi;

      currentSequence.getROIName(roiName);
      currentSequence.getROIDescription(roiDescription);
      currentSequence.getROINumber(roiNumber);

      roi.Name = roiName.c_str();
      roi.Description = roiDescription.c_str();
      roi.Number = roiNumber;

      this->ROISequenceVector.push_back(roi);
    } while (roiSequence.gotoNextItem().good());

    Sint32 refRoiNumber;
    DRTROIContourSequence& roiContourSeqObject =
      structureSetObject.getROIContourSequence();

    if (!roiContourSeqObject.gotoFirstItem().good())
    {
      MITK_ERROR << "Error reading the contour sequence" << std::endl;
      return result;
    }

    do
    {
      mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();
      DRTROIContourSequence::Item& currentRoiObject =
        roiContourSeqObject.getCurrentItem();

      if (!currentRoiObject.isValid())
      {
        continue;
      }

      currentRoiObject.getReferencedROINumber(refRoiNumber);
      DRTContourSequence& contourSeqObject =
        currentRoiObject.getContourSequence();

      if (contourSeqObject.getNumberOfItems() > 0 && contourSeqObject.gotoFirstItem().good())
      {
        do
        {
          DRTContourSequence::Item& contourItem =
            contourSeqObject.getCurrentItem();

          if (!contourItem.isValid())
          {
            continue;
          }

          OFString contourNumber;
          OFString numberOfPoints;
          OFVector<Float64> contourData_LPS;
          mitk::ContourModel::Pointer contourSequence =
            mitk::ContourModel::New();

          contourItem.getContourNumber(contourNumber);
          contourItem.getNumberOfContourPoints(numberOfPoints);
          contourItem.getContourData(contourData_LPS);

          for (unsigned int i = 0; i < contourData_LPS.size() / 3; i++)
          {
            mitk::Point3D point;
            point[0] = contourData_LPS.at(3 * i);
            point[1] = contourData_LPS.at(3 * i + 1);
            point[2] = contourData_LPS.at(3 * i + 2);
            contourSequence->AddVertex(point);
          }

          contourSequence->Close();
          contourSet->AddContourModel(contourSequence);
        } while (contourSeqObject.gotoNextItem().good());
      }
      else
      {
        MITK_WARN << "contourSeqObject has no items in sequence. Object is neglected and not read. Struct name: " << this->FindRoiByNumber(refRoiNumber)->Name << std::endl;
      }

      RoiEntry* refROI = this->FindRoiByNumber(refRoiNumber);

      if (refROI == nullptr)
      {
        MITK_ERROR << "Can not find references ROI" << std::endl;
        continue;
      }

      Sint32 roiColor;

      for (unsigned int j = 0; j < 3; j++)
      {
        currentRoiObject.getROIDisplayColor(roiColor, j);
        refROI->DisplayColor[j] = roiColor / 255.0;
      }

      refROI->ContourModelSet = contourSet;
      contourSet->SetProperty("name", mitk::StringProperty::New(refROI->Name));
      contourSet->SetProperty("contour.color", mitk::ColorProperty::New(
        refROI->DisplayColor[0],
        refROI->DisplayColor[1],
        refROI->DisplayColor[2]));

    } while (roiContourSeqObject.gotoNextItem().good());

    for (auto const& aROI : ROISequenceVector)
    {
      result.push_back(aROI.ContourModelSet.GetPointer());
      result.at(result.size() - 1)->SetProperty("name", aROI.ContourModelSet->GetProperty("name"));
      result.at(result.size() - 1)->SetProperty("color", aROI.ContourModelSet->GetProperty("contour.color"));
      result.at(result.size() - 1)->SetProperty("contour.color", aROI.ContourModelSet->GetProperty("contour.color"));
      DICOMIOHelper::SetProperties(result.at(result.size() - 1).GetPointer(), findings);
    }

    return result;
  }

  RTStructureSetReaderService* RTStructureSetReaderService::Clone() const
  {
    return new RTStructureSetReaderService(*this);
  }

}
