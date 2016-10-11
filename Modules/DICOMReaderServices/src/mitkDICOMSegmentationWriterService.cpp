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

#include "mitkDICOMSegmentationWriterService.h"
#include "mitkIOMimeTypes.h"
#include "mitkLocaleSwitch.h"
#include "mitkImage.h"
#include "mitkImageToItk.h"
#include "ImageSEGConverter.h"

#include <locale>


mitk::DICOMSegmentationWriterService::DICOMSegmentationWriterService()
  : AbstractFileWriter(mitk::Image::GetStaticNameOfClass(), CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), "DICOM Segmentation Writer")
{
  RegisterService();
}

mitk::DICOMSegmentationWriterService::DICOMSegmentationWriterService(const mitk::DICOMSegmentationWriterService& other)
  : AbstractFileWriter(other)
{
}

mitk::DICOMSegmentationWriterService::~DICOMSegmentationWriterService()
{
}

void mitk::DICOMSegmentationWriterService::Write()
{
  typedef itk::Image<short, 3> ImageType;
  vector<ImageType::Pointer> segmentations;

  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  LocalFile localFile(this);
  const std::string path = localFile.GetFileName();

  //1. Handle segmentation as image
  const mitk::Image* image = dynamic_cast<const mitk::Image*>(this->GetInput());

  if (image == NULL)
  {
    mitkThrow() << "Cannot write non-image data";
  }


  ImageToItk<ImageType>::Pointer imageToItkFilter = ImageToItk<ImageType>::New();
  try
  {
    imageToItkFilter->SetInput(image);
  }
  catch (const itk::ExceptionObject &e)
  {
    // Most probably the input image type is wrong. Binary images are expected to be                             
    // >unsigned< char images.                                                                                   
    MITK_ERROR << e.GetDescription() << endl;
    return;
  }

  imageToItkFilter->Update();
  ImageType::Pointer itkImage = imageToItkFilter->GetOutput();

  segmentations.push_back(itkImage);

  //TODO: 2.  LabelSetImage
  try
  {
    //TODO: Fill dcmDatasets with information
    vector<DcmDataset*> dcmDatasets;
    //TODO: Fill meta data with information    
    const std::string &tmpMetaInfoFile = "";

    MITK_INFO << "Writing image: " << path << std::endl;

    //Convert itk image to dicom image
    dcmqi::ImageSEGConverter* converter = new dcmqi::ImageSEGConverter();
    DcmDataset* result = converter->itkimage2dcmSegmentation(dcmDatasets, segmentations, tmpMetaInfoFile);

    //write dicom file
    DcmFileFormat dcmFileFormat(result);
    //TODO: Check if path contains filename with ending .dcm
    dcmFileFormat.saveFile(path.c_str(), EXS_LittleEndianExplicit);
  }
  catch (const std::exception& e)
  {
    mitkThrow() << e.what();
  }

}

mitk::DICOMSegmentationWriterService*mitk::DICOMSegmentationWriterService::Clone() const
{
  return new DICOMSegmentationWriterService(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::DICOMSegmentationWriterService::GetConfidenceLevel() const
{//TODO check if segmentation with dicom infos
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());
  if (input.IsNull() /*|| !mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(input)*/)
  {
    return Unsupported;
  }
  else
  {
    return Supported;
  }
}