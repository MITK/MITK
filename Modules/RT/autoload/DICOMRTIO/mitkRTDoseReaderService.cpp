/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <mitkRTDoseReaderService.h>

#include <mitkLexicalCast.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkDICOMFileReader.h>
#include <mitkRTConstants.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMRTMimeTypes.h>
#include <mitkDICOMIOHelper.h>
#include <mitkProperties.h>
#include <mitkDICOMProperty.h>

#include <dcmtk/dcmrt/drtdose.h>

#include <itkShiftScaleImageFilter.h>
#include <itkCastImageFilter.h>

namespace mitk
{

  RTDoseReaderService::RTDoseReaderService() : AbstractFileReader(CustomMimeType(mitk::DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE_NAME()), mitk::DICOMRTMimeTypes::DICOMRT_DOSE_MIMETYPE_DESCRIPTION()) {
    m_FileReaderServiceReg = RegisterService();
  }

  RTDoseReaderService::RTDoseReaderService(const RTDoseReaderService& other) : mitk::AbstractFileReader(other)
  {

  }

  RTDoseReaderService::~RTDoseReaderService() {}

  template<typename TPixel, unsigned int VImageDimension>
  void RTDoseReaderService::MultiplyGridScaling(itk::Image<TPixel, VImageDimension>* image, float gridscale)
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

  std::vector<itk::SmartPointer<BaseData> > RTDoseReaderService::DoRead()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;


    mitk::IDICOMTagsOfInterest* toiSrv = GetDicomTagsOfInterestService();

    auto tagsOfInterest = toiSrv->GetTagsOfInterest();

    DICOMTagPathList tagsOfInterestList;
    for (const auto& tag : tagsOfInterest) {
      tagsOfInterestList.push_back(tag.first);
    }

    std::string location = GetInputLocation();
    mitk::DICOMFileReaderSelector::Pointer selector = mitk::DICOMFileReaderSelector::New();
    selector->LoadBuiltIn3DConfigs();
    selector->SetInputFiles({ location });

    mitk::DICOMFileReader::Pointer reader = selector->GetFirstReaderWithMinimumNumberOfOutputImages();
    reader->SetAdditionalTagsOfInterest(toiSrv->GetTagsOfInterest());
    reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);

    reader->SetInputFiles({ location });
    reader->AnalyzeInputFiles();
    reader->LoadImages();

    if (reader->GetNumberOfOutputs() == 0) {
      MITK_ERROR << "Could not determine a DICOM reader for this file" << std::endl;
      return result;
    }

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
    scanner->SetInputFiles({ location });
    scanner->AddTagPaths(tagsOfInterestList);
    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    if (frames.empty()) {
      MITK_ERROR << "Error reading the RTDOSE file" << std::endl;
      return result;
    }

    const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(0);

    mitk::Image::Pointer originalImage = desc.GetMitkImage();

    if (originalImage.IsNull())
    {
      MITK_ERROR << "Error reading the RTDOSE file in mitk::DicomFileReader" << std::endl;
      return result;
    }

    DcmFileFormat fileformat;
    OFCondition outp = fileformat.loadFile(location.c_str(), EXS_Unknown);
    if (outp.bad())
    {
      MITK_ERROR << "Error reading the RTDOSE file in DCMTK" << std::endl;
      return result;
    }
    DcmDataset *dataset = fileformat.getDataset();

    DRTDoseIOD doseObject;
    OFCondition DCMTKresult = doseObject.read(*dataset);

    if (DCMTKresult.bad())
    {
      MITK_ERROR << "Error reading the RTDOSE file in DCMTK" << std::endl;
      return result;
    }

    auto findingsGridScaling = frames.front()->GetTagValueAsString(DICOMTagPath(0x3004, 0x000e)); //(0x3004, 0x000e) is grid scaling

    double gridScaling;
    if (findingsGridScaling.empty()) {
      MITK_ERROR << "Could not find DoseGridScaling tag" << std::endl;
      return result;
    }
    else {
      gridScaling = boost::lexical_cast<double>(findingsGridScaling.front().value);
    }

    AccessByItk_1(originalImage, MultiplyGridScaling, gridScaling);

    auto statistics = this->scaledDoseImage->GetStatistics();
    double maxDose = statistics->GetScalarValueMax();

    this->scaledDoseImage->SetPropertyList(originalImage->GetPropertyList());
    this->scaledDoseImage->SetProperty(mitk::RTConstants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(), mitk::DoubleProperty::New(0.8*maxDose));
    auto findings = ExtractPathsOfInterest(tagsOfInterestList, frames);
    SetProperties(this->scaledDoseImage, findings);

    result.push_back(this->scaledDoseImage.GetPointer());
    return result;
  }

  RTDoseReaderService* RTDoseReaderService::Clone() const
  {
    return new RTDoseReaderService(*this);
  }
}
