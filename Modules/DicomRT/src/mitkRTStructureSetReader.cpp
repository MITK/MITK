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

#include "mitkRTStructureSetReader.h"

#include <mitkIOMimeTypes.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkTemporoSpatialStringProperty.h>

#include "dcmtk/dcmrt/drtstrct.h"

namespace mitk
{
    RTStructureSetReader::RTStructureSetReader() : AbstractFileReader(IOMimeTypes::DICOM_MIMETYPE_NAME(), "DICOM RTSTRUCT File Reader") {
        m_ServiceReg = RegisterService();
        m_DICOMTagsOfInterestService = GetDicomTagsOfInterestService();
    }

    RTStructureSetReader::RTStructureSetReader(const RTStructureSetReader& other) : mitk::AbstractFileReader(other)
    {

    }

    RTStructureSetReader::~RTStructureSetReader() {}

    RTStructureSetReader::RoiEntry::RoiEntry()
    {
        Number = 0;
        DisplayColor[0] = 1.0;
        DisplayColor[1] = 0.0;
        DisplayColor[2] = 0.0;
        ContourModelSet = mitk::ContourModelSet::New();
    }

    RTStructureSetReader::RoiEntry::RoiEntry(const RoiEntry& src)
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

    RTStructureSetReader::RoiEntry::~RoiEntry() {}

    RTStructureSetReader::RoiEntry& RTStructureSetReader::
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

    void RTStructureSetReader::RoiEntry::
        SetPolyData(mitk::ContourModelSet::Pointer roiPolyData)
    {
        if (roiPolyData == this->ContourModelSet)
        {
            return;
        }

        this->ContourModelSet = roiPolyData;
    }

    size_t RTStructureSetReader::GetNumberOfROIs()
    {
        return this->ROISequenceVector.size();
    }

    RTStructureSetReader::RoiEntry* RTStructureSetReader::
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

    std::vector<itk::SmartPointer<BaseData> > RTStructureSetReader::Read()
    {
        std::vector<itk::SmartPointer<mitk::BaseData> > result;

        std::string location = GetInputLocation();

        auto tagsOfInterest = m_DICOMTagsOfInterestService->GetTagsOfInterest();
        DICOMTagPathList tagsOfInterestList;
        for (const auto& tag : tagsOfInterest){
            tagsOfInterestList.push_back(tag.first);
        }


        mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
        scanner->SetInputFiles({ location });
        scanner->AddTagPaths(tagsOfInterestList);
        scanner->Scan();

        mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
        if (frames.empty()){
            MITK_ERROR << "Error reading the RTSTRUCT file" << std::endl;
            return result;
        }

        auto findings = ExtractPathsOfInterest(tagsOfInterestList, frames);

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

            if (contourSeqObject.getNumberOfItems()>0 && contourSeqObject.gotoFirstItem().good())
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
            SetProperties(result.at(result.size() - 1).GetPointer(), findings);
        }

        return result;
    }

    RTStructureSetReader* RTStructureSetReader::Clone() const
    {
        return new RTStructureSetReader(*this);
    }

    RTStructureSetReader::FindingsListVectorType RTStructureSetReader::ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest, const mitk::DICOMDatasetAccessingImageFrameList& frames) const
    {
        std::vector<mitk::DICOMDatasetAccess::FindingsListType > findings;
        for (const auto& entry : pathsOfInterest){
            findings.push_back(frames.front()->GetTagValueAsString(entry));
        }
        return findings;
    }

    void RTStructureSetReader::SetProperties(BaseData::Pointer data, const FindingsListVectorType& findings) const
    {
        for (const auto& finding : findings){
            for (const auto& entry : finding){
                const std::string propertyName = mitk::DICOMTagPathToPropertyName(entry.path);
                auto property = mitk::TemporoSpatialStringProperty::New();
                property->SetValue(entry.value);
                data->SetProperty(propertyName.c_str(), property);
            }
        }
    }

    mitk::IDICOMTagsOfInterest* RTStructureSetReader::GetDicomTagsOfInterestService() const
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
