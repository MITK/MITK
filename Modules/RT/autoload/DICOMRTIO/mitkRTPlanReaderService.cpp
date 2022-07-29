/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <mitkRTPlanReaderService.h>

#include <mitkImage.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkIOMimeTypes.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMTagPath.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkDICOMDatasetAccessingImageFrameInfo.h>
#include <mitkDICOMRTMimeTypes.h>


namespace mitk
{

  RTPlanReaderService::RTPlanReaderService() : AbstractFileReader(CustomMimeType(mitk::DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE_NAME()), mitk::DICOMRTMimeTypes::DICOMRT_PLAN_MIMETYPE_DESCRIPTION()) {
    m_FileReaderServiceReg = RegisterService();

  }

  RTPlanReaderService::RTPlanReaderService(const RTPlanReaderService& other) : mitk::AbstractFileReader(other)
  {
  }

  RTPlanReaderService::~RTPlanReaderService() {}

  std::vector<itk::SmartPointer<BaseData> > RTPlanReaderService::DoRead()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    auto DICOMTagsOfInterestService = DICOMIOHelper::GetTagsOfInterestService();
    auto tagsOfInterest = DICOMTagsOfInterestService->GetTagsOfInterest();
    DICOMTagPathList tagsOfInterestList;
    for (const auto& tag : tagsOfInterest) {
      tagsOfInterestList.push_back(tag.first);
    }

    std::string location = GetInputLocation();
    mitk::StringList files = { location };
    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
    scanner->SetInputFiles(files);
    scanner->AddTagPaths(tagsOfInterestList);
    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    if (frames.empty()) {
      MITK_ERROR << "Error reading the RTPLAN file" << std::endl;
      return result;
    }

    auto findings = DICOMIOHelper::ExtractPathsOfInterest(tagsOfInterestList, frames);

    //just create empty image. No image information available in RTPLAN. But properties will be attached.
    Image::Pointer dummyImage = Image::New();
    mitk::PixelType pt = mitk::MakeScalarPixelType<int>();
    unsigned int dim[] = { 1,1};
    dummyImage->Initialize(pt, 2, dim);
    DICOMIOHelper::SetProperties(dummyImage, findings);

    result.push_back(dummyImage.GetPointer());

    return result;
  }

  RTPlanReaderService* RTPlanReaderService::Clone() const
  {
    return new RTPlanReaderService(*this);
  }

}
