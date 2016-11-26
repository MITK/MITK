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

#ifndef __mitkDICOMSegmentationIOWriter__cpp
#define __mitkDICOMSegmentationIOWriter__cpp

#include "mitkDICOMSegmentationIO.h"
#include "mitkBasePropertySerializer.h"
#include "mitkDICOMProperty.h"
#include "mitkIOMimeTypes.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkLabelSetIOHelper.h"
#include "mitkLabelSetImageConverter.h"
#include "mitkProperties.h"
#include "mitkPropertyNameHelper.h"
#include "mitkStringProperty.h"
#include <mitkLocaleSwitch.h>

// itk
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkThresholdImageFilter.h"

// dcmqi
#include <dcmqi/ImageSEGConverter.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

namespace mitk
{
  DICOMSegmentationIO::DICOMSegmentationIO()
    : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(), IOMimeTypes::DICOM_MIMETYPE(), "DICOM Segmentation")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  IFileIO::ConfidenceLevel DICOMSegmentationIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;

    const LabelSetImage *input = static_cast<const LabelSetImage *>(this->GetInput());
    if (input)
      return Supported;
    else
      return Unsupported;
  }

  void DICOMSegmentationIO::Write()
  {
    ValidateOutputLocation();

    mitk::LocaleSwitch localeSwitch("C");
    LocalFile localFile(this);
    const std::string path = localFile.GetFileName();

    auto input = dynamic_cast<const LabelSetImage *>(this->GetInput());
    if (input == nullptr)
      mitkThrow() << "Cannot write non-image data";

    // Get DICOM information from referenced image
    vector<DcmDataset *> dcmDatasets;
    DcmFileFormat *readFileFormat = new DcmFileFormat();
    try
    {
      // TODO: Generate dcmdataset witk DICOM tags from property list
      mitk::StringLookupTableProperty::Pointer filesProp =
        dynamic_cast<mitk::StringLookupTableProperty *>(input->GetProperty("files").GetPointer());

      if (filesProp.IsNull())
      {
        MITK_ERROR << "No property with dicom file path.";
        return;
      }

      StringLookupTable filesLut = filesProp->GetValue();
      const StringLookupTable::LookupTableType &map = filesLut.GetLookupTable();

      for (auto it = map.begin(); it != map.end(); ++it)
      {
        const char *fileName = (it->second).c_str();
        if (readFileFormat->loadFile(fileName, EXS_Unknown).good())
          dcmDatasets.push_back(readFileFormat->getAndRemoveDataset());
      }
    }
    catch (const std::exception &e)
    {
      mitkThrow() << e.what();
    }

    //Iterate over all layers. For each adcm file is generated
    for (unsigned int layer = 0; layer < input->GetNumberOfLayers(); ++layer)
    {
      vector<itkInternalImageType::Pointer> segmentations;

      try
      {
        const LabelSet *labelSet = input->GetLabelSet(layer);

        for (int label = 1; label < input->GetNumberOfLabels(layer); ++label)
        {
          typedef itk::CastImageFilter<itkInputImageType, itkInternalImageType> castItkImageFilterType;
          ImageToItk<itkInputImageType>::Pointer imageToItkFilter = ImageToItk<itkInputImageType>::New();
          // BUG: It must be the layer image, but there are some errors with it (dcmqi: generate the dcmSeg "No frame data available") --> input->GetLayerImage(layer)
          imageToItkFilter->SetInput(input);
          imageToItkFilter->Update();

          castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
          castFilter->SetInput(imageToItkFilter->GetOutput());
          castFilter->Update();
          itkInternalImageType::Pointer itkLabelImage = castFilter->GetOutput();

          itk::ThresholdImageFilter<itkInternalImageType>::Pointer thresholdFilter =
            itk::ThresholdImageFilter<itkInternalImageType>::New();

          thresholdFilter->SetInput(itkLabelImage);
          thresholdFilter->ThresholdOutside(label, label);
          thresholdFilter->SetOutsideValue(0);
          thresholdFilter->Update();
          itkInternalImageType::Pointer segmentImage = thresholdFilter->GetOutput();
          segmentImage->DisconnectPipeline();

          segmentations.push_back(segmentImage);
        }
      }
      catch (const itk::ExceptionObject &e)
      {
        MITK_ERROR << e.GetDescription() << endl;
        return;
      }

      // Create segmentation meta information
      const std::string &tmpMetaInfoFile = this->CreateMetaDataJsonFile(layer);


      MITK_INFO << "Writing image: " << path << std::endl;
      try
      {
        // Convert itk image to dicom image
        dcmqi::ImageSEGConverter *converter = new dcmqi::ImageSEGConverter();
        DcmDataset *result = converter->itkimage2dcmSegmentation(dcmDatasets, segmentations, tmpMetaInfoFile);
        // Todo for each layer
        // write dicom file
        DcmFileFormat dcmFileFormat(result);

        std::string filePath = path.substr(0, path.find_last_of("."));
        // if there is more than one layer, we have to write more than 1 dicom file
        if (input->GetNumberOfLayers() != 1)
          filePath = filePath + std::to_string(layer) + ".dcm";
        else
          filePath = filePath + ".dcm";

        dcmFileFormat.saveFile(filePath.c_str(), EXS_LittleEndianExplicit);
      }
      catch (const std::exception &e)
      {
        mitkThrow() << e.what();
      }
    }

    // end image write
    if (readFileFormat)
      delete readFileFormat;
    for (int i = 0; i < dcmDatasets.size(); i++)
    {
      delete dcmDatasets[i];
    }
  }

  IFileIO::ConfidenceLevel DICOMSegmentationIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;

    const std::string fileName = this->GetLocalFileName();

    DcmFileFormat dcmFileFormat;
    OFCondition status = dcmFileFormat.loadFile(fileName.c_str());

    if (status.bad())
      return Unsupported;

    OFString modality;
    if (dcmFileFormat.getDataset()->findAndGetOFString(DCM_Modality, modality).good())
    {
      if (modality.compare("SEG") == 0)
        return Supported;
      else
        return Unsupported;
    }
    return Unsupported;
  }

  std::vector<BaseData::Pointer> DICOMSegmentationIO::Read()
  {
    mitk::LocaleSwitch localeSwitch("C");

    LabelSetImage::Pointer labelSetImage;
    const std::string path = this->GetLocalFileName();

    MITK_INFO << "loading " << path << std::endl;

    // Check to see if we can read the file given the name or prefix
    if (path.empty())
    {
      mitkThrow() << "Empty filename in mitk::ItkImageIO ";
    }

    try
    {
      DcmFileFormat dcmFileFormat;
      OFCondition status = dcmFileFormat.loadFile(path.c_str());
      if (status.bad())
        MITK_ERROR << "Can't read the input file!";

      DcmDataset *dataSet = dcmFileFormat.getDataset();
      if (dataSet == nullptr)
        MITK_ERROR << "Can't read data from input file!";

      dcmqi::ImageSEGConverter *converter = new dcmqi::ImageSEGConverter();
      pair<map<unsigned, ImageType::Pointer>, string> dcmqiOutput = converter->dcmSegmentation2itkimage(dataSet);

      dcmqi::JSONSegmentationMetaInformationHandler metaInfo(dcmqiOutput.second.c_str());
      metaInfo.read();
      MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();

      map<unsigned, ImageType::Pointer> segItkImages = dcmqiOutput.first;

      vector<map<unsigned, dcmqi::SegmentAttributes *>>::const_iterator segmentIter =
        metaInfo.segmentsAttributesMappingList.begin();
      for (auto &element : segItkImages)
      {
        // Get the labeled image and cast it to mitkImage
        typedef itk::CastImageFilter<itkInternalImageType, itkInputImageType> castItkImageFilterType;
        castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
        castFilter->SetInput(element.second);
        castFilter->Update();

        Image::Pointer layerImage;
        CastToMitkImage(castFilter->GetOutput(), layerImage);

        // Get pixel value of the label
        itkInternalImageType::ValueType segValue = 1;
        typedef itk::ImageRegionIterator<const itkInternalImageType> IteratorType;

        IteratorType iter(element.second, element.second->GetLargestPossibleRegion());
        iter.GoToBegin();
        while (!iter.IsAtEnd())
        {
          itkInputImageType::PixelType value = iter.Get();

          if (value != 0)
          {
            segValue = value;
            break;
          }
          ++iter;
        }

        // Get the label information from segment attributes
        map<unsigned, dcmqi::SegmentAttributes *> segmentMap = (*segmentIter);
        map<unsigned, dcmqi::SegmentAttributes *>::const_iterator segmentMapIter = (*segmentIter).begin();
        dcmqi::SegmentAttributes *segmentAttr = (*segmentMapIter).second;

        OFString labelName;

        if (segmentAttr->getSegmentedPropertyType() != nullptr)
          segmentAttr->getSegmentedPropertyType()->getCodeMeaning(labelName);
        else
        {
          labelName = std::to_string(segmentAttr->getLabelID()).c_str();
          if (labelName.empty())
            labelName = "Unnamed";
        }

        float tmp[3] = {0.0, 0.0, 0.0};
        if (segmentAttr->getRecommendedDisplayRGBValue() != nullptr)
        {
          tmp[0] = segmentAttr->getRecommendedDisplayRGBValue()[0] / 255.0;
          tmp[1] = segmentAttr->getRecommendedDisplayRGBValue()[1] / 255.0;
          tmp[2] = segmentAttr->getRecommendedDisplayRGBValue()[2] / 255.0;
        }

        // if labelSetImage do not exists (first image)
        if (labelSetImage.IsNull())
        {
          // Initialize the labelSetImage with the read image
          labelSetImage = LabelSetImage::New();
          labelSetImage->InitializeByLabeledImage(layerImage);
          // Already a label generated, so set the information to this
          Label *activeLabel = labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer());
          activeLabel->SetName(labelName.c_str());
          activeLabel->SetColor(Color(tmp));
          activeLabel->SetValue(segValue);
        }
        else
        {
          // Add a new layer to the labelSetImage. Background label is set automatically
          labelSetImage->AddLayer(layerImage);

          // Add new label
          Label *newLabel = new Label;
          newLabel->SetName(labelName.c_str());
          newLabel->SetColor(Color(tmp));
          newLabel->SetValue(segValue);

          labelSetImage->GetLabelSet(labelSetImage->GetActiveLayer())->AddLabel(newLabel);
        }

        ++segmentIter;
      }
    }
    catch (const std::exception &e)
    {
      mitkThrow() << e.what();
    }

    if (labelSetImage->GetNumberOfLayers() > 1 && labelSetImage->GetActiveLayer() != 0)
      labelSetImage->SetActiveLayer(0);

    std::vector<BaseData::Pointer> result;
    result.push_back(labelSetImage.GetPointer());

    return result;
  }

  const std::string mitk::DICOMSegmentationIO::CreateMetaDataJsonFile(int layer)
  {
    const mitk::LabelSetImage *image = dynamic_cast<const mitk::LabelSetImage *>(this->GetInput());

    const std::string output;

    dcmqi::JSONSegmentationMetaInformationHandler handler;
    handler.setContentCreatorName("MITK");
    handler.setClinicalTrialSeriesID("Session 1");
    handler.setClinicalTrialTimePointID("0");

    std::string seriesDescription = "";
    image->GetPropertyList()->GetStringProperty("name", seriesDescription);
    if (seriesDescription.empty())
      seriesDescription = "Segmentation";
    handler.setSeriesDescription(seriesDescription);
    handler.setSeriesNumber("34"+ std::to_string(layer)); // TODO:Create own series number
    handler.setInstanceNumber("1");
    handler.setBodyPartExamined("");

    const LabelSet *labelSet = image->GetLabelSet(layer);

    for (int i = 1; i < image->GetNumberOfLabels(); ++i)
    {
      const Label *label = labelSet->GetLabel(i);
      if (label != nullptr)
      {
        mitk::DICOMTagPath segmentNumberPath;
        segmentNumberPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0004);
        StringProperty *segmentNumberProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentNumberPath).c_str()));

        mitk::DICOMTagPath segmentLabelPath;
        segmentLabelPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0005);
        StringProperty *segmentLabelProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentLabelPath).c_str()));

        mitk::DICOMTagPath segmentAlgorithmTypePath;
        segmentAlgorithmTypePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0008);
        StringProperty *algorithmTypeProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentAlgorithmTypePath).c_str()));

        mitk::DICOMTagPath segmentCategoryCodeValuePath;
        segmentCategoryCodeValuePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0100);
        StringProperty *segmentCategoryCodeValueProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeValuePath).c_str()));

        mitk::DICOMTagPath segmentCategoryCodeSchemePath;
        segmentCategoryCodeSchemePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0102);
        StringProperty *segmentCategoryCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeSchemePath).c_str()));

        mitk::DICOMTagPath segmentCategoryCodeMeaningPath;
        segmentCategoryCodeMeaningPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0104);
        StringProperty *segmentCategoryCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeMeaningPath).c_str()));

        mitk::DICOMTagPath segmentTypeCodeValuePath;
        segmentTypeCodeValuePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0100);
        StringProperty *segmentTypeCodeValueProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeValuePath).c_str()));

        mitk::DICOMTagPath segmentTypeCodeSchemePath;
        segmentTypeCodeSchemePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0102);
        StringProperty *segmentTypeCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeSchemePath).c_str()));

        mitk::DICOMTagPath segmentTypeCodeMeaningPath;
        segmentTypeCodeMeaningPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0104);
        StringProperty *segmentTypeCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeMeaningPath).c_str()));

        mitk::DICOMTagPath segmentModifierCodeValuePath;
        segmentModifierCodeValuePath.AddElement(0x0062, 0x0002)
          .AddElement(0x0062, 0x000F)
          .AddElement(0x0062, 0x0011)
          .AddElement(0x008, 0x0100);
        StringProperty *segmentModifierCodeValueProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeValuePath).c_str()));

        mitk::DICOMTagPath segmentModifierCodeSchemePath;
        segmentModifierCodeSchemePath.AddElement(0x0062, 0x0002)
          .AddElement(0x0062, 0x000F)
          .AddElement(0x0062, 0x0011)
          .AddElement(0x008, 0x0102);
        StringProperty *segmentModifierCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeSchemePath).c_str()));

        mitk::DICOMTagPath segmentModifierCodeMeaningPath;
        segmentModifierCodeMeaningPath.AddElement(0x0062, 0x0002)
          .AddElement(0x0062, 0x000F)
          .AddElement(0x0062, 0x0011)
          .AddElement(0x008, 0x0104);
        StringProperty *segmentModifierCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(
          label->GetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeMeaningPath).c_str()));

        int labelId = std::stoi(segmentNumberProp->GetValue());
        dcmqi::SegmentAttributes *segAttr = handler.createAndGetNewSegment(labelId);
        if (segAttr != nullptr)
        {
          segAttr->setSegmentDescription(segmentLabelProp->GetValueAsString());
          segAttr->setSegmentAlgorithmType(algorithmTypeProp->GetValueAsString());
          segAttr->setSegmentAlgorithmName("MITK Segmentation");
          if (segmentCategoryCodeValueProp != nullptr && segmentCategoryCodeSchemeProp != nullptr &&
              segmentCategoryCodeMeaningProp != nullptr)
            segAttr->setSegmentedPropertyCategoryCode(segmentCategoryCodeValueProp->GetValueAsString(),
                                                      segmentCategoryCodeSchemeProp->GetValueAsString(),
                                                      segmentCategoryCodeMeaningProp->GetValueAsString());
          if (segmentTypeCodeValueProp != nullptr && segmentTypeCodeSchemeProp != nullptr &&
              segmentTypeCodeMeaningProp != nullptr)
          {
            segAttr->setSegmentedPropertyType(segmentTypeCodeValueProp->GetValueAsString(),
                                              segmentTypeCodeSchemeProp->GetValueAsString(),
                                              segmentTypeCodeMeaningProp->GetValueAsString());
            handler.setBodyPartExamined(segmentTypeCodeMeaningProp->GetValueAsString());
          }
          if (segmentModifierCodeValueProp != nullptr && segmentModifierCodeSchemeProp != nullptr &&
              segmentModifierCodeMeaningProp != nullptr)
            segAttr->setSegmentedPropertyTypeModifier(segmentModifierCodeValueProp->GetValueAsString(),
                                                      segmentModifierCodeSchemeProp->GetValueAsString(),
                                                      segmentModifierCodeMeaningProp->GetValueAsString());

          Color color = label->GetColor();
          segAttr->setRecommendedDisplayRGBValue(color[0] * 255, color[1] * 255, color[2] * 255);
        }
      }
    }
    return handler.getJSONOutputAsString();
  }

  DICOMSegmentationIO *DICOMSegmentationIO::IOClone() const { return new DICOMSegmentationIO(*this); }
} // namespace

#endif //__mitkLabelSetImageWriter__cpp
