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

#include "mitkDICOMSegIOMimeTypes.h"
#include "mitkDICOMSegmentationConstants.h"
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>
#include <mitkPropertyNameHelper.h>


// itk
#include <itkThresholdImageFilter.h>

// dcmqi
#include <dcmqi/ImageSEGConverter.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  DICOMSegmentationIO::DICOMSegmentationIO()
    : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(),
      mitk::MitkDICOMSEGIOMimeTypes::DICOMSEG_MIMETYPE_NAME(),
      "DICOM Segmentation")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();

    this->AddDICOMTagsToService();
  }

  void DICOMSegmentationIO::AddDICOMTagsToService()
  {
    IDICOMTagsOfInterest *toiService = GetDicomTagsOfInterestService();
    if (toiService != nullptr)
    {
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_LABEL_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH());

      toiService->AddTagOfInterest(DICOMSegmentationConstants::ANATOMIC_REGION_SEQUENCE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_PATH());
      toiService->AddTagOfInterest(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_PATH());

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

    // Check if the input file is a segmentation
    const LabelSetImage *input = static_cast<const LabelSetImage *>(this->GetInput());

    if (input)
    {
      if ((input->GetDimension() != 3))
      {
        MITK_INFO << "DICOM segmentation writer is tested only with 3D images, sorry.";
        return Unsupported;
      }

      // Check if input file has dicom information for the referenced image (original DICOM image, e.g. CT) Still necessary, see write() 
      mitk::StringLookupTableProperty::Pointer dicomFilesProp =
      dynamic_cast<mitk::StringLookupTableProperty *>(input->GetProperty("referenceFiles").GetPointer());

      if (dicomFilesProp.IsNotNull())
        return Supported;
    }

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
    vector<std::unique_ptr<DcmDataset>> dcmDatasetsSourceImage;
    std::unique_ptr<DcmFileFormat> readFileFormat(new DcmFileFormat());
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
        {
          std::unique_ptr<DcmDataset> readDCMDataset(readFileFormat->getAndRemoveDataset());
          dcmDatasetsSourceImage.push_back(std::move(readDCMDataset));
        }
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
        // Hack: Remove the const attribute to switch between the layer images. Normally you could get the different
        // layer images by input->GetLayerImage(layer)
        mitk::LabelSetImage *mitkLayerImage = const_cast<mitk::LabelSetImage *>(input);
        mitkLayerImage->SetActiveLayer(layer);

        // Cast mitk layer image to itk
        ImageToItk<itkInputImageType>::Pointer imageToItkFilter = ImageToItk<itkInputImageType>::New();
        imageToItkFilter->SetInput(mitkLayerImage);
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
      const std::string tmpMetaInfoFile = this->CreateMetaDataJsonFile(layer);

      MITK_INFO << "Writing image: " << path << std::endl;
      try
      {
        //TODO is there a better way? Interface expects a vector of raw pointer.
        vector<DcmDataset*> rawVecDataset;
        for (const auto& dcmDataSet : dcmDatasetsSourceImage)
          rawVecDataset.push_back(dcmDataSet.get());

        // Convert itk segmentation images to dicom image
        std::unique_ptr<dcmqi::ImageSEGConverter> converter = std::make_unique<dcmqi::ImageSEGConverter>();
        std::unique_ptr<DcmDataset> result(converter->itkimage2dcmSegmentation(rawVecDataset, segmentations, tmpMetaInfoFile));

        // Write dicom file
        DcmFileFormat dcmFileFormat(result.get());

        std::string filePath = path.substr(0, path.find_last_of("."));
        // If there is more than one layer, we have to write more than 1 dicom file
        if (input->GetNumberOfLayers() != 1)
          filePath = filePath + std::to_string(layer) + ".dcm";
        else
          filePath = filePath + ".dcm";

        dcmFileFormat.saveFile(filePath.c_str(), EXS_LittleEndianExplicit);
      }
      catch (const std::exception &e)
      {
        MITK_ERROR << "An error occurred during writing the DICOM Seg: " << e.what() << endl;
        return;
      }
    } // Write a dcm file for the next layer
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

      //=============================== dcmqi part ====================================
      // Read the DICOM SEG images (segItkImages) and DICOM tags (metaInfo)
      std::unique_ptr<dcmqi::ImageSEGConverter> converter = std::make_unique<dcmqi::ImageSEGConverter>();
      pair<map<unsigned, itkInternalImageType::Pointer>, string> dcmqiOutput =
        converter->dcmSegmentation2itkimage(dataSet);

      map<unsigned, itkInternalImageType::Pointer> segItkImages = dcmqiOutput.first;

      dcmqi::JSONSegmentationMetaInformationHandler metaInfo(dcmqiOutput.second.c_str());
      metaInfo.read();

      MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();
      //===============================================================================

      // Get the label information from segment attributes for each itk image
      vector<map<unsigned, dcmqi::SegmentAttributes *>>::const_iterator segmentIter =
        metaInfo.segmentsAttributesMappingList.begin();

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
        // Get Segment information map
        map<unsigned, dcmqi::SegmentAttributes *> segmentMap = (*segmentIter);
        map<unsigned, dcmqi::SegmentAttributes *>::const_iterator segmentMapIter = (*segmentIter).begin();
        dcmqi::SegmentAttributes *segmentAttribute = (*segmentMapIter).second;

        OFString labelName;

        if (segmentAttribute->getSegmentedPropertyTypeCodeSequence() != nullptr)
        {
          segmentAttribute->getSegmentedPropertyTypeCodeSequence()->getCodeMeaning(labelName);
          if (segmentAttribute->getSegmentedPropertyTypeModifierCodeSequence() != nullptr)
          {
            OFString modifier;
            segmentAttribute->getSegmentedPropertyTypeModifierCodeSequence()->getCodeMeaning(modifier);
            labelName.append(" (").append(modifier).append(")");
          }
        }
        else
        {
          labelName = std::to_string(segmentAttribute->getLabelID()).c_str();
          if (labelName.empty())
            labelName = "Unnamed";
        }

        float tmp[3] = { 0.0, 0.0, 0.0 };
        if (segmentAttribute->getRecommendedDisplayRGBValue() != nullptr)
        {
          tmp[0] = segmentAttribute->getRecommendedDisplayRGBValue()[0] / 255.0;
          tmp[1] = segmentAttribute->getRecommendedDisplayRGBValue()[1] / 255.0;
          tmp[2] = segmentAttribute->getRecommendedDisplayRGBValue()[2] / 255.0;
        }

        Label *newLabel = nullptr;
        // If labelSetImage do not exists (first image)
        if (labelSetImage.IsNull())
        {
          // Initialize the labelSetImage with the read image
          labelSetImage = LabelSetImage::New();
          labelSetImage->InitializeByLabeledImage(layerImage);
          // Already a label was generated, so set the information to this
          newLabel = labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer());
          newLabel->SetName(labelName.c_str());
          newLabel->SetColor(Color(tmp));
          newLabel->SetValue(segValue);
        }
        else
        {
          // Add a new layer to the labelSetImage. Background label is set automatically
          labelSetImage->AddLayer(layerImage);

          // Add new label
          newLabel = new Label;
          newLabel->SetName(labelName.c_str());
          newLabel->SetColor(Color(tmp));
          newLabel->SetValue(segValue);
          labelSetImage->GetLabelSet(labelSetImage->GetActiveLayer())->AddLabel(newLabel);
        }

        // Add some more label properties
        this->SetLabelProperties(newLabel, segmentAttribute);
        ++segmentIter;
      }

      labelSetImage->GetLabelSet()->SetAllLabelsVisible(true);

      // Add some general DICOM Segmentation properties
      mitk::IDICOMTagsOfInterest *toiSrv = GetDicomTagsOfInterestService();
      auto tagsOfInterest = toiSrv->GetTagsOfInterest();
      DICOMTagPathList tagsOfInterestList;
      for (const auto &tag : tagsOfInterest)
      {
        tagsOfInterestList.push_back(tag.first);
      }

      mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
      scanner->SetInputFiles({ GetInputLocation() });
      scanner->AddTagPaths(tagsOfInterestList);
      scanner->Scan();

      mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
      if (frames.empty())
      {
        MITK_ERROR << "Error reading the DICOM Seg file" << std::endl;
        return result;
      }

      auto findings = ExtractPathsOfInterest(tagsOfInterestList, frames);
      SetProperties(labelSetImage, findings);

      // Set active layer to the first layer of the labelset image
      if (labelSetImage->GetNumberOfLayers() > 1 && labelSetImage->GetActiveLayer() != 0)
        labelSetImage->SetActiveLayer(0);
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "An error occurred while reading the DICOM Seg file: " << e.what();
      return result;
    }
    catch (...)
    {
      MITK_ERROR << "An error occurred in dcmqi while reading the DICOM Seg file";
      return result;
    }

    result.push_back(labelSetImage.GetPointer());
    return result;
  }

  const std::string mitk::DICOMSegmentationIO::CreateMetaDataJsonFile(int layer)
  {
    const mitk::LabelSetImage *image = dynamic_cast<const mitk::LabelSetImage *>(this->GetInput());

    const std::string output;
    dcmqi::JSONSegmentationMetaInformationHandler handler;

 
    // 1. Metadata attributes that will be listed in the resulting DICOM SEG object
    std::string contentCreatorName;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0070, 0x0084).c_str(),
      contentCreatorName))
      contentCreatorName = "MITK";
    handler.setContentCreatorName(contentCreatorName);

    std::string clinicalTrailSeriesId;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0071).c_str(),
      clinicalTrailSeriesId))
      clinicalTrailSeriesId = "Session 1";
    handler.setClinicalTrialSeriesID(clinicalTrailSeriesId);

    std::string clinicalTrialTimePointID;
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0050).c_str(),
      clinicalTrialTimePointID))
      clinicalTrialTimePointID = "0";
    handler.setClinicalTrialTimePointID(clinicalTrialTimePointID);

    std::string clinicalTrialCoordinatingCenterName = "";
    if (!image->GetPropertyList()->GetStringProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0060).c_str(),
      clinicalTrialCoordinatingCenterName))
      clinicalTrialCoordinatingCenterName = "Unknown";
    handler.setClinicalTrialCoordinatingCenterName(clinicalTrialCoordinatingCenterName);

    std::string seriesDescription;
    if (!image->GetPropertyList()->GetStringProperty("name", seriesDescription))
      seriesDescription = "MITK Segmentation";
    handler.setSeriesDescription(seriesDescription);

    handler.setSeriesNumber("0" + std::to_string(layer));
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
        TemporoSpatialStringProperty *segmentNumberProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentLabelProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_LABEL_PATH()).c_str()));

        TemporoSpatialStringProperty *algorithmTypeProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentCategoryCodeValueProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentCategoryCodeSchemeProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentCategoryCodeMeaningProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentTypeCodeValueProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentTypeCodeSchemeProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentTypeCodeMeaningProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentModifierCodeValueProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentModifierCodeSchemeProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()).c_str()));

        TemporoSpatialStringProperty *segmentModifierCodeMeaningProp = dynamic_cast<mitk::TemporoSpatialStringProperty *>(label->GetProperty(
          mitk::DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()).c_str()));

        dcmqi::SegmentAttributes *segmentAttribute = nullptr;

        if (segmentNumberProp->GetValue() == "")
        {
          MITK_ERROR << "Something went wrong with the label ID.";
        }
        else
        {
          int labelId = std::stoi(segmentNumberProp->GetValue());
          segmentAttribute = handler.createAndGetNewSegment(labelId);
        }
        if (segmentAttribute != nullptr)
        {
          segmentAttribute->setSegmentDescription(segmentLabelProp->GetValueAsString());
          segmentAttribute->setSegmentAlgorithmType(algorithmTypeProp->GetValueAsString());
          segmentAttribute->setSegmentAlgorithmName("MITK Segmentation");
          if (segmentCategoryCodeValueProp != nullptr && segmentCategoryCodeSchemeProp != nullptr &&
            segmentCategoryCodeMeaningProp != nullptr)
            segmentAttribute->setSegmentedPropertyCategoryCodeSequence(
              segmentCategoryCodeValueProp->GetValueAsString(),
              segmentCategoryCodeSchemeProp->GetValueAsString(),
              segmentCategoryCodeMeaningProp->GetValueAsString());
          else
            // some default values
            segmentAttribute->setSegmentedPropertyCategoryCodeSequence(
              "M-01000", "SRT", "Morphologically Altered Structure");

          if (segmentTypeCodeValueProp != nullptr && segmentTypeCodeSchemeProp != nullptr &&
            segmentTypeCodeMeaningProp != nullptr)
          {
            segmentAttribute->setSegmentedPropertyTypeCodeSequence(segmentTypeCodeValueProp->GetValueAsString(),
              segmentTypeCodeSchemeProp->GetValueAsString(),
              segmentTypeCodeMeaningProp->GetValueAsString());
            handler.setBodyPartExamined(segmentTypeCodeMeaningProp->GetValueAsString());
          }
          else
          {
            // some default values
            segmentAttribute->setSegmentedPropertyTypeCodeSequence("M-03000", "SRT", "Mass");
            handler.setBodyPartExamined("Mass");
          }
          if (segmentModifierCodeValueProp != nullptr && segmentModifierCodeSchemeProp != nullptr &&
            segmentModifierCodeMeaningProp != nullptr)
            segmentAttribute->setSegmentedPropertyTypeModifierCodeSequence(
              segmentModifierCodeValueProp->GetValueAsString(),
              segmentModifierCodeSchemeProp->GetValueAsString(),
              segmentModifierCodeMeaningProp->GetValueAsString());

          Color color = label->GetColor();
          segmentAttribute->setRecommendedDisplayRGBValue(color[0] * 255, color[1] * 255, color[2] * 255);
        }
      }
    }
    return handler.getJSONOutputAsString();
  }

  void mitk::DICOMSegmentationIO::SetLabelProperties(mitk::Label *label, dcmqi::SegmentAttributes *segmentAttribute)
  {
    // Segment Number:Identification number of the segment.The value of Segment Number(0062, 0004) shall be unique
    // within the Segmentation instance in which it is created
    label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()).c_str(),
      TemporoSpatialStringProperty::New(std::to_string(label->GetValue())));

    // Segment Label: User-defined label identifying this segment.
    label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_LABEL_PATH()).c_str(),
      TemporoSpatialStringProperty::New(label->GetName()));

    // Segment Algorithm Type: Type of algorithm used to generate the segment.
    if (!segmentAttribute->getSegmentAlgorithmType().empty())
      label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(segmentAttribute->getSegmentAlgorithmType()));

    // Add Segmented Property Category Code Sequence tags
    auto categoryCodeSequence = segmentAttribute->getSegmentedPropertyCategoryCodeSequence();
    if (categoryCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      categoryCodeSequence->getCodeValue(codeValue);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeValue.c_str()));

      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      categoryCodeSequence->getCodingSchemeDesignator(codeScheme);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeScheme.c_str()));

      OFString codeMeaning; // (0008,0104) Code Meaning
      categoryCodeSequence->getCodeMeaning(codeMeaning);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeMeaning.c_str()));
    }

    // Add Segmented Property Type Code Sequence tags
    auto typeCodeSequence = segmentAttribute->getSegmentedPropertyTypeCodeSequence();
    if (typeCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      typeCodeSequence->getCodeValue(codeValue);
      label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeValue.c_str()));

      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      typeCodeSequence->getCodingSchemeDesignator(codeScheme);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeScheme.c_str()));

      OFString codeMeaning; // (0008,0104) Code Meaning
      typeCodeSequence->getCodeMeaning(codeMeaning);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeMeaning.c_str()));
    }

    // Add Segmented Property Type Modifier Code Sequence tags
    auto modifierCodeSequence = segmentAttribute->getSegmentedPropertyTypeModifierCodeSequence();
    if (modifierCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      modifierCodeSequence->getCodeValue(codeValue);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeValue.c_str()));

      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      modifierCodeSequence->getCodingSchemeDesignator(codeScheme);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeScheme.c_str()));

      OFString codeMeaning; // (0008,0104) Code Meaning
      modifierCodeSequence->getCodeMeaning(codeMeaning);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeMeaning.c_str()));
    }

    // Add Atomic RegionSequence tags
    auto atomicRegionSequence = segmentAttribute->getAnatomicRegionSequence();
    if (atomicRegionSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      atomicRegionSequence->getCodeValue(codeValue);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeValue.c_str()));

      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      atomicRegionSequence->getCodingSchemeDesignator(codeScheme);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeScheme.c_str()));

      OFString codeMeaning; // (0008,0104) Code Meaning
      atomicRegionSequence->getCodeMeaning(codeMeaning);
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::ANATOMIC_REGION_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(codeMeaning.c_str()));
    }
  }

  DICOMSegmentationIO *DICOMSegmentationIO::IOClone() const { return new DICOMSegmentationIO(*this); }
} // namespace

#endif //__mitkDICOMSegmentationIO__cpp
