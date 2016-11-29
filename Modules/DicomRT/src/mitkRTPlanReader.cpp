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


#include "mitkRTPlanReader.h"

#include "mitkDICOMDCMTKTagScanner.h"
#include "mitkImage.h"
#include "mitkIOMimeTypes.h"
#include "mitkDICOMTagPath.h"

#include <boost/lexical_cast.hpp>

namespace mitk
{

    RTPlanReader::RTPlanReader() : AbstractFileReader(IOMimeTypes::DICOM_MIMETYPE_NAME(), "DICOM RTPlan File Reader") {
        m_ServiceReg = RegisterService();
    }

    RTPlanReader::RTPlanReader(const RTPlanReader& other) : mitk::AbstractFileReader(other)
    {

    }

    RTPlanReader::~RTPlanReader(){}

    std::vector<itk::SmartPointer<BaseData> > RTPlanReader::Read()
    {
        std::vector<itk::SmartPointer<mitk::BaseData> > result;

        auto pathsOfInterestInformation = GeneratePathsOfInterest();
        auto pathsOfInterest = ExtractDicomPathList(pathsOfInterestInformation);

        std::string location = GetInputLocation();
        mitk::StringList files = { location };
        mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
        scanner->SetInputFiles(files);
        scanner->AddTagPaths(pathsOfInterest);
        scanner->Scan();

        mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
        if (frames.empty()){
            MITK_ERROR << "Error reading the RTPLAN file" << std::endl;
            return result;
        }

        auto findings = ReadPathsOfInterest(pathsOfInterestInformation, frames);

        //just create empty image. No image information available in RTPLAN. But properties will be attached.
        Image::Pointer dummyImage = Image::New();
        SetProperties(dummyImage, findings);

        result.push_back(dummyImage.GetPointer());

        return result;
    }

    mitk::DICOMTagPath RTPlanReader::GenerateDicomTagPath(unsigned int tag1, unsigned int tag2, unsigned int sqTag1, unsigned int sqTag2) const
    {
        mitk::DICOMTagPath aTagPath;
        aTagPath.AddAnySelection(tag1, tag2).AddElement(sqTag1, sqTag2);
        return aTagPath;
    }

    RTPlanReader* RTPlanReader::Clone() const
    {
        return new RTPlanReader(*this);
    }

    std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> > RTPlanReader::GeneratePathsOfInterest() const
    {
        std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> > pathsOfInterestInformation;

        //dose reference UID
        auto tagPath = GenerateDicomTagPath(0x300A, 0x0010, 0x300A, 0x0013);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "string", tagPath));

        //dose reference description
        tagPath = GenerateDicomTagPath(0x300A, 0x0010, 0x300A, 0x0016);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "string", tagPath));

        //target prescription dose
        tagPath = GenerateDicomTagPath(0x300A, 0x0010, 0x300A, 0x0026);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "double", tagPath));

        //planned number of fractions
        tagPath = GenerateDicomTagPath(0x300A, 0x0070, 0x300A, 0x0078);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "int", tagPath));

        //number of beams
        tagPath = GenerateDicomTagPath(0x300A, 0x0070, 0x300A, 0x0080);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "int", tagPath));

        //radiation type
        tagPath = GenerateDicomTagPath(0x300A, 0x00B0, 0x300A, 0x00C6);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "string", tagPath));

        //structure set UID
        tagPath = GenerateDicomTagPath(0x300C, 0x0060, 0x0008, 0x1155);
        pathsOfInterestInformation.push_back(std::make_tuple(mitk::DICOMTagPathToPropertyName(tagPath), "string", tagPath));

        return pathsOfInterestInformation;
    }

    mitk::DICOMTagPathList RTPlanReader::ExtractDicomPathList(const std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> >& pathsOfInterestInformation) const
    {
        mitk::DICOMTagPathList list;
        for (auto element : pathsOfInterestInformation){
            list.push_back(std::get<2>(element));
        }
        return list;
    }

    std::vector<std::tuple<std::string, std::string, mitk::DICOMDatasetAccess::FindingsListType> > RTPlanReader::ReadPathsOfInterest(const std::vector<std::tuple<std::string, std::string, mitk::DICOMTagPath> > & pathsOfInterestInformation, const mitk::DICOMDatasetAccessingImageFrameList& frames) const
    {
        std::vector<std::tuple<std::string, std::string, mitk::DICOMDatasetAccess::FindingsListType> > findings;
        for (auto& entry : pathsOfInterestInformation){
            std::string name, type;
            mitk::DICOMTagPath dicomTag;
            std::tie(name, type, dicomTag) = entry;
            findings.push_back(std::make_tuple(name, type, frames.front()->GetTagValueAsString(dicomTag)));
        }
        return findings;
    }

    void RTPlanReader::SetProperties(Image::Pointer dummyImage, const std::vector<std::tuple<std::string, std::string, mitk::DICOMDatasetAccess::FindingsListType> >& findings) const
    {
        for (const auto& finding : findings){
            unsigned int count = 0;
            std::string name, type;
            mitk::DICOMDatasetAccess::FindingsListType foundValues;
            std::tie(name, type, foundValues) = finding;
            for (auto& entry : foundValues){
                std::string nameWithNumber = name + "." + std::to_string(count);
                if (type == "string"){
                    auto genericProperty = mitk::GenericProperty<std::string>::New(entry.value);
                    dummyImage->SetProperty(nameWithNumber.c_str(), genericProperty);
                }
                else if (type == "double"){
                    auto genericProperty = mitk::GenericProperty<double>::New(boost::lexical_cast<double>(entry.value));
                    dummyImage->SetProperty(nameWithNumber.c_str(), genericProperty);
                }
                else if (type == "int"){
                    auto genericProperty = mitk::GenericProperty<int>::New(boost::lexical_cast<int>(entry.value));
                    dummyImage->SetProperty(nameWithNumber.c_str(), genericProperty);
                }
                count++;
            }
        }
    }

}
