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

#ifndef __mitkDICOMSegmentationIO__cpp
#define __mitkDICOMSegmentationIO__cpp

#include "mitkDICOMSegmentationIO.h"

#include "mitkDICOMSegmentationConstants.h"
#include <mitkDICOMProperty.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkIOMimeTypes.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>

// itk
#include <itkThresholdImageFilter.h>

// dcmqi
#include <dcmqi/ImageSEGConverter.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  DICOMSegmentationIO::DICOMSegmentationIO()
    : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(), IOMimeTypes::DICOM_MIMETYPE(), "DICOM Segmentation")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();

    this->AddDICOMTagsToService();
  }

  void DICOMSegmentationIO::AddDICOMTagsToService()
  {
    IDICOMTagsOfInterest *toiService = nullptr;

    std::vector<us::ServiceReference<IDICOMTagsOfInterest>> toiRegisters =
      us::GetModuleContext()->GetServiceReferences<IDICOMTagsOfInterest>();
    if (!toiRegisters.empty())
    {
      if (toiRegisters.size() > 1)
        MITK_WARN << "Multiple DICOM tags of interest services found. Using just one.";
      toiService = us::GetModuleContext()->GetService<IDICOMTagsOfInterest>(toiRegisters.front());
    }

    if (toiService != nullptr)
    {
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_LABEL_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH());
    }
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
      // TODO: Generate dcmdataset witk DICOM tags from property list; ATM the source are the filepaths from the
      // property list
      mitk::StringLookupTableProperty::Pointer filesProp =
        dynamic_cast<mitk::StringLookupTableProperty *>(input->GetProperty("referenceFiles").GetPointer());

      if (filesProp.IsNull())
      {
        mitkThrow() << "No property with dicom file path.";
        return;
      }

      StringLookupTable filesLut = filesProp->GetValue();
      const StringLookupTable::LookupTableType &lookUpTableMap = filesLut.GetLookupTable();

      for (auto it : lookUpTableMap)
      {
        const char *fileName = (it.second).c_str();
        if (readFileFormat->loadFile(fileName, EXS_Unknown).good())
          dcmDatasets.push_back(readFileFormat->getAndRemoveDataset());
      }
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "An error occurred while getting the dicom informations: " << e.what() << endl;
      return;
    }

    // Iterate over all layers. For each a dcm file will be generated
    for (unsigned int layer = 0; layer < input->GetNumberOfLayers(); ++layer)
    {
      vector<itkInternalImageType::Pointer> segmentations;

      try
      {
        // Cast mitk layer image to itk
        ImageToItk<itkInputImageType>::Pointer imageToItkFilter = ImageToItk<itkInputImageType>::New();
        // BUG: It must be the layer image, but there are some errors with it (dcmqi: generate the dcmSeg "No frame data
        // available") --> input->GetLayerImage(layer)
        imageToItkFilter->SetInput(input);
        imageToItkFilter->Update();

        // Cast from original itk type to dcmqi input itk image type
        typedef itk::CastImageFilter<itkInputImageType, itkInternalImageType> castItkImageFilterType;
        castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
        castFilter->SetInput(imageToItkFilter->GetOutput());
        castFilter->Update();

        itkInternalImageType::Pointer itkLabelImage = castFilter->GetOutput();
        itkLabelImage->DisconnectPipeline();

        // Iterate over all labels. For each label a segmentation image will be created
        const LabelSet *labelSet = input->GetLabelSet(layer);
        auto labelIter = labelSet->IteratorConstBegin();
        // Ignore background label
        ++labelIter;

        for (; labelIter != labelSet->IteratorConstEnd(); ++labelIter)
        {
          // Thresold over the image with the given label value
          itk::ThresholdImageFilter<itkInternalImageType>::Pointer thresholdFilter =
            itk::ThresholdImageFilter<itkInternalImageType>::New();
          thresholdFilter->SetInput(itkLabelImage);
          thresholdFilter->ThresholdOutside(labelIter->first, labelIter->first);
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
        // Convert itk segmentation images to dicom image
        dcmqi::ImageSEGConverter *converter = new dcmqi::ImageSEGConverter();
        DcmDataset *result = converter->itkimage2dcmSegmentation(dcmDatasets, segmentations, tmpMetaInfoFile);

        // Write dicom file
        DcmFileFormat dcmFileFormat(result);

        std::string filePath = path.substr(0, path.find_last_of("."));
        // If there is more than one layer, we have to write more than 1 dicom file
        if (input->GetNumberOfLayers() != 1)
          filePath = filePath + std::to_string(layer) + ".dcm";
        else
          filePath = filePath + ".dcm";

        dcmFileFormat.saveFile(filePath.c_str(), EXS_LittleEndianExplicit);

        // Clean up
        if (converter != nullptr)
          delete converter;
        if (result != nullptr)
          delete result;
      }
      catch (const std::exception &e)
      {
        MITK_ERROR << "An error occurred during writing the DICOM Seg: " << e.what() << endl;
        return;
      }
    } // Write a dcm file for the next layer

    // End of image writing; clean up
    if (readFileFormat)
      delete readFileFormat;

    for (auto obj : dcmDatasets)
      delete obj;
    dcmDatasets.clear();
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
    std::vector<BaseData::Pointer> result;

    const std::string path = this->GetLocalFileName();

    MITK_INFO << "loading " << path << std::endl;

    if (path.empty())
      mitkThrow() << "Empty filename in mitk::ItkImageIO ";

    try
    {
      // Get the dcm data set from file path
      DcmFileFormat dcmFileFormat;
      OFCondition status = dcmFileFormat.loadFile(path.c_str());
      if (status.bad())
        mitkThrow() << "Can't read the input file!";

      DcmDataset *dataSet = dcmFileFormat.getDataset();
      if (dataSet == nullptr)
        mitkThrow() << "Can't read data from input file!";

      // Read the DICOM SEG images (segItkImages) and DICOM tags (metaInfo)
      dcmqi::ImageSEGConverter *converter = new dcmqi::ImageSEGConverter();
      pair<map<unsigned, ImageType::Pointer>, string> dcmqiOutput = converter->dcmSegmentation2itkimage(dataSet);

      map<unsigned, ImageType::Pointer> segItkImages = dcmqiOutput.first;

      // For each itk image add a layer to the LabelSetImage output
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
        // Iterate over the image to find the pixel value of the label
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

        dcmqi::JSONSegmentationMetaInformationHandler metaInfo(dcmqiOutput.second.c_str());
        metaInfo.read();
        MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();
        // TODO: Read all DICOM Tags

        // Get the label information from segment attributes
        vector<map<unsigned, dcmqi::SegmentAttributes *>>::const_iterator segmentIter =
          metaInfo.segmentsAttributesMappingList.begin();
        map<unsigned, dcmqi::SegmentAttributes *> segmentMap = (*segmentIter);
        map<unsigned, dcmqi::SegmentAttributes *>::const_iterator segmentMapIter = (*segmentIter).begin();
        dcmqi::SegmentAttributes *segmentAttr = (*segmentMapIter).second;

        OFString labelName;

        if (segmentAttr->getSegmentedPropertyTypeCodeSequence() != nullptr)
          segmentAttr->getSegmentedPropertyTypeCodeSequence()->getCodeMeaning(labelName);
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

        // If labelSetImage do not exists (first image)
        if (labelSetImage.IsNull())
        {
          // Initialize the labelSetImage with the read image
          labelSetImage = LabelSetImage::New();
          labelSetImage->InitializeByLabeledImage(layerImage);
          // Already a label was generated, so set the information to this
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
      // Clean up
      if (converter != nullptr)
        delete converter;
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "An error occurred while reading the DICOM Seg file: " << e.what();
      return result;
    }

    // Set active layer to the first layer of the labelset image
    if (labelSetImage->GetNumberOfLayers() > 1 && labelSetImage->GetActiveLayer() != 0)
      labelSetImage->SetActiveLayer(0);

    result.push_back(labelSetImage.GetPointer());

    return result;
  }

  const std::string mitk::DICOMSegmentationIO::CreateMetaDataJsonFile(int layer)
  {
    const mitk::LabelSetImage *image = dynamic_cast<const mitk::LabelSetImage *>(this->GetInput());

    const std::string output;
    dcmqi::JSONSegmentationMetaInformationHandler handler;

    // Default values
    handler.setContentCreatorName("MITK");
    handler.setClinicalTrialSeriesID("Session 1");
    handler.setClinicalTrialTimePointID("0");
    handler.setClinicalTrialCoordinatingCenterName("Unknown");

    std::string seriesDescription = "";
    image->GetPropertyList()->GetStringProperty("name", seriesDescription);
    if (seriesDescription.empty())
      seriesDescription = "MITK Segmentation";
    handler.setSeriesDescription(seriesDescription);
    handler.setSeriesNumber("34" + std::to_string(layer)); // TODO:Create own series number
    handler.setInstanceNumber("1");
    handler.setBodyPartExamined("");

    const LabelSet *labelSet = image->GetLabelSet(layer);
    auto labelIter = labelSet->IteratorConstBegin();
    // Ignore background label
    ++labelIter;

    for (; labelIter != labelSet->IteratorConstEnd(); ++labelIter)
    {
      const Label *label = labelIter->second;

      if (label != nullptr)
      {
        StringProperty *segmentNumberProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()).c_str()));

        StringProperty *segmentLabelProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_LABEL_PATH()).c_str()));

        StringProperty *algorithmTypeProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()).c_str()));

        StringProperty *segmentCategoryCodeValueProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()).c_str()));

        StringProperty *segmentCategoryCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()).c_str()));

        StringProperty *segmentCategoryCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()).c_str()));

        StringProperty *segmentTypeCodeValueProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()).c_str()));

        StringProperty *segmentTypeCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()).c_str()));

        StringProperty *segmentTypeCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()).c_str()));

        StringProperty *segmentModifierCodeValueProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()).c_str()));

        StringProperty *segmentModifierCodeSchemeProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()).c_str()));

        StringProperty *segmentModifierCodeMeaningProp = dynamic_cast<mitk::StringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()).c_str()));

        dcmqi::SegmentAttributes *segAttr;

        if (segmentNumberProp->GetValue() == nullptr)
        {
          MITK_ERROR << "Something went wrong with the label ID.";
        }
        else
        {
          int labelId = std::stoi(segmentNumberProp->GetValue());
          segAttr = handler.createAndGetNewSegment(labelId);
        }
        if (segAttr != nullptr)
        {
          segAttr->setSegmentDescription(segmentLabelProp->GetValueAsString());
          segAttr->setSegmentAlgorithmType(algorithmTypeProp->GetValueAsString());
          segAttr->setSegmentAlgorithmName("MITK Segmentation");
          if (segmentCategoryCodeValueProp != nullptr && segmentCategoryCodeSchemeProp != nullptr &&
              segmentCategoryCodeMeaningProp != nullptr)
            segAttr->setSegmentedPropertyCategoryCodeSequence(segmentCategoryCodeValueProp->GetValueAsString(),
                                                              segmentCategoryCodeSchemeProp->GetValueAsString(),
                                                              segmentCategoryCodeMeaningProp->GetValueAsString());
          else
            // some default values
            segAttr->setSegmentedPropertyCategoryCodeSequence("M-01000", "SRT", "Morphologically Altered Structure");

          if (segmentTypeCodeValueProp != nullptr && segmentTypeCodeSchemeProp != nullptr &&
              segmentTypeCodeMeaningProp != nullptr)
          {
            segAttr->setSegmentedPropertyTypeCodeSequence(segmentTypeCodeValueProp->GetValueAsString(),
                                                          segmentTypeCodeSchemeProp->GetValueAsString(),
                                                          segmentTypeCodeMeaningProp->GetValueAsString());
            handler.setBodyPartExamined(segmentTypeCodeMeaningProp->GetValueAsString());
          }
          else
          {
            // some default values
            segAttr->setSegmentedPropertyTypeCodeSequence("M-03000", "SRT", "Mass");
            handler.setBodyPartExamined("Mass");
          }
          if (segmentModifierCodeValueProp != nullptr && segmentModifierCodeSchemeProp != nullptr &&
              segmentModifierCodeMeaningProp != nullptr)
            segAttr->setSegmentedPropertyTypeModifierCodeSequence(segmentModifierCodeValueProp->GetValueAsString(),
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

#endif //__mitkDICOMSegmentationIO__cpp
