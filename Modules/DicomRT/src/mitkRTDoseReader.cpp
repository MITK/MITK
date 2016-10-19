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


#include "mitkRTDoseReader.h"

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMFileReader.h>
#include <mitkRTConstants.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkDICOMTagHelper.h>

#include "usModuleContext.h"
#include "usGetModuleContext.h"

#include "dcmtk/dcmrt/drtdose.h"

#include <itkShiftScaleImageFilter.h>
#include <itkCastImageFilter.h>

namespace mitk
{

    RTDoseReader::RTDoseReader(){
    }

    RTDoseReader::~RTDoseReader(){}

    mitk::DataNode::Pointer RTDoseReader::LoadRTDose(const char* filename)
    {
        DICOMTag referencedRTPlan(0x300c, 0x0002);
        mitk::IDICOMTagsOfInterest* toiSrv = GetDicomTagsOfInterestService();
        if (toiSrv)
        {
            toiSrv->AddTagOfInterest(referencedRTPlan);
        }

        mitk::StringList oneFile = { filename };
        mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();
        selector->LoadBuiltIn3DConfigs();
        selector->SetInputFiles(oneFile);

        mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
        reader->SetAdditionalTagsOfInterest(toiSrv->GetTagsOfInterest());

        reader->SetInputFiles(oneFile);
        reader->AnalyzeInputFiles();
        reader->LoadImages();

        if (reader->GetNumberOfOutputs() == 0){
            MITK_ERROR << "Error reading the dicom file" << std::endl;
            return nullptr;
        }

        const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(0);

        mitk::Image::Pointer originalImage = desc.GetMitkImage();

        if (originalImage.IsNull())
        {
            MITK_ERROR << "Error reading the dcm file" << std::endl;
            return nullptr;
        }

        DcmFileFormat fileformat;
        OFCondition outp = fileformat.loadFile(filename, EXS_Unknown);
        if (outp.bad())
        {
            MITK_ERROR << "Can't read the RTDOSE file" << std::endl;
        }
        DcmDataset *dataset = fileformat.getDataset();

        DRTDoseIOD doseObject;
        OFCondition result = doseObject.read(*dataset);

        if(result.bad())
        {
            MITK_ERROR << "Error reading the Dataset" << std::endl;
            return nullptr;
        }

        OFString gridScaling;
        Float32 gridscale;

        doseObject.getDoseGridScaling(gridScaling);
        gridscale = OFStandard::atof(gridScaling.c_str());

        AccessByItk_1(originalImage, MultiplyGridScaling, gridscale);

        auto statistics = this->scaledDoseImage->GetStatistics();
        double maxDose = statistics->GetScalarValueMax();

        this->scaledDoseImage->SetPropertyList(originalImage->GetPropertyList());
        this->scaledDoseImage->SetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(), mitk::GenericProperty<double>::New(0.8*maxDose));

        mitk::DataNode::Pointer originalNode = mitk::DataNode::New();

        originalNode->SetName("RT Dose");
        originalNode->SetData(this->scaledDoseImage);
        originalNode->SetFloatProperty(mitk::RTConstants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), 40.0);

        return originalNode;
  }

    template<typename TPixel, unsigned int VImageDimension>
    void RTDoseReader::MultiplyGridScaling(itk::Image<TPixel, VImageDimension>* image, float gridscale)
    {
        typedef itk::Image<Float32, VImageDimension> OutputImageType;
        typedef itk::Image<TPixel, VImageDimension> InputImageType;

        typedef itk::CastImageFilter<InputImageType, OutputImageType> CastFilterType;
        typedef itk::ShiftScaleImageFilter<OutputImageType, OutputImageType> ScaleFilterType;
        typename CastFilterType::Pointer castFilter = CastFilterType::New();
        typename ScaleFilterType::Pointer scaleFilter = ScaleFilterType::New();

        castFilter->SetInput(image);
        scaleFilter->SetInput(castFilter->GetOutput());
        scaleFilter->SetScale(gridscale);
        scaleFilter->Update();
        typename OutputImageType::Pointer scaledOutput = scaleFilter->GetOutput();
        this->scaledDoseImage = mitk::Image::New();

        mitk::CastToMitkImage(scaledOutput, this->scaledDoseImage);
    }

    mitk::IDICOMTagsOfInterest* RTDoseReader::GetDicomTagsOfInterestService()
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
