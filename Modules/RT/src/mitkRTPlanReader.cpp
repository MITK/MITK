/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkRTPlanReader.h"

#include "mitkDICOMDCMTKTagScanner.h"
#include "mitkIOMimeTypes.h"
#include "mitkDICOMTagPath.h"
#include "mitkDICOMTagsOfInterestService.h"
#include "mitkTemporoSpatialStringProperty.h"

namespace mitk
{

    RTPlanReader::RTPlanReader() : AbstractFileReader(IOMimeTypes::DICOM_MIMETYPE_NAME(), "DICOM RTPlan File Reader") {
        m_FileReaderServiceReg = RegisterService();
        m_DICOMTagsOfInterestService = GetDicomTagsOfInterestService();
    }

    RTPlanReader::RTPlanReader(const RTPlanReader& other) : mitk::AbstractFileReader(other)
    {}

    RTPlanReader::~RTPlanReader(){}

    std::vector<itk::SmartPointer<BaseData> > RTPlanReader::Read()
    {
        std::vector<itk::SmartPointer<mitk::BaseData> > result;

        auto tagsOfInterest = m_DICOMTagsOfInterestService->GetTagsOfInterest();
        DICOMTagPathList tagsOfInterestList;
        for (const auto& tag : tagsOfInterest){
            tagsOfInterestList.push_back(tag.first);
        }

        std::string location = GetInputLocation();
        mitk::StringList files = { location };
        mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
        scanner->SetInputFiles(files);
        scanner->AddTagPaths(tagsOfInterestList);
        scanner->Scan();

        mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
        if (frames.empty()){
            MITK_ERROR << "Error reading the RTPLAN file" << std::endl;
            return result;
        }

        auto findings = ExtractPathsOfInterest(tagsOfInterestList, frames);

        //just create empty image. No image information available in RTPLAN. But properties will be attached.
        Image::Pointer dummyImage = Image::New();
        SetProperties(dummyImage, findings);

        result.push_back(dummyImage.GetPointer());

        return result;
    }

    RTPlanReader* RTPlanReader::Clone() const
    {
        return new RTPlanReader(*this);
    }

    RTPlanReader::FindingsListVectorType RTPlanReader::ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest, const mitk::DICOMDatasetAccessingImageFrameList& frames) const
    {
        std::vector<mitk::DICOMDatasetAccess::FindingsListType > findings;
        for (const auto& entry : pathsOfInterest){
            findings.push_back(frames.front()->GetTagValueAsString(entry));
        }
        return findings;
    }

    void RTPlanReader::SetProperties(Image::Pointer image, const FindingsListVectorType& findings) const
    {
        for (const auto& finding : findings){
            for (const auto& entry : finding){
                const std::string propertyName = mitk::DICOMTagPathToPropertyName(entry.path);
                auto property = mitk::TemporoSpatialStringProperty::New();
                property->SetValue(entry.value);
                image->SetProperty(propertyName.c_str(), property);
            }
        }
    }

    mitk::IDICOMTagsOfInterest* RTPlanReader::GetDicomTagsOfInterestService() const
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
