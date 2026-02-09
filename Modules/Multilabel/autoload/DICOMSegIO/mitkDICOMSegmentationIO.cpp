/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
#include <dcmqi/Itk2DicomConverter.h>
#include <dcmqi/Dicom2ItkConverter.h>
#include <dcmtk/dcmdata/dcdeftag.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  DICOMSegmentationIO::DICOMSegmentationIO()
    : AbstractFileIO(MultiLabelSegmentation::GetStaticNameOfClass(),
      mitk::MitkDICOMSEGIOMimeTypes::DICOMSEG_MIMETYPE_NAME(),
      "DICOM Segmentation")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  IFileIO::ConfidenceLevel DICOMSegmentationIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;

    // Check if the input file is a segmentation
    const MultiLabelSegmentation *input = dynamic_cast<const MultiLabelSegmentation *>(this->GetInput());

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

    auto input = dynamic_cast<const MultiLabelSegmentation *>(this->GetInput());
    if (input == nullptr)
      mitkThrow() << "Cannot write non-image data";

    // Get DICOM information from referenced image
    vector<std::unique_ptr<DcmDataset>> dcmDatasetsSourceImage;
    std::unique_ptr<DcmFileFormat> readFileFormat = std::make_unique<DcmFileFormat>();
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

      for (const auto &it : lookUpTableMap)
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
      MITK_ERROR << "An error occurred while getting the dicom information: " << e.what() << endl;
      return;
    }

    // Iterate over all layers. For each a dcm file will be generated
    for (unsigned int layer = 0; layer < input->GetNumberOfGroups(); ++layer)
    {
      vector<itkInternalImageType::ConstPointer> segmentations;

      try
      {
        auto mitkLayerImage = input->GetGroupImage(layer);

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
        auto labelSet = input->GetConstLabelsByValue(input->GetLabelValuesByGroup(layer));

        for (const auto& label : labelSet)
        {
          // Threshold over the image with the given label value
          itk::ThresholdImageFilter<itkInternalImageType>::Pointer thresholdFilter =
            itk::ThresholdImageFilter<itkInternalImageType>::New();
          thresholdFilter->SetInput(itkLabelImage);
          thresholdFilter->ThresholdOutside(label->GetValue(), label->GetValue());
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
        vector<DcmItem*> rawVecDataset;
        for (const auto& dcmDataSet : dcmDatasetsSourceImage)
          rawVecDataset.push_back(dcmDataSet.get());

        // Convert itk segmentation images to dicom image
        auto converter = std::make_unique<dcmqi::Itk2DicomConverter>();
        std::unique_ptr<DcmDataset> result(converter->itkimage2dcmSegmentation(rawVecDataset, segmentations, tmpMetaInfoFile, false));

        //We store only one group, thus we can specify the SegmentsOverlap Tag (0062,0013)
        // as NO
        auto condition = result->putAndInsertString(DCM_SegmentsOverlap, "NO");
        if (condition.bad())
        {
          MITK_DEBUG << "unable to set SegmentOverlap tag.";
        }

        // Write dicom file
        DcmFileFormat dcmFileFormat(result.get());

        std::string filePath = path.substr(0, path.find_last_of("."));
        // If there is more than one layer, we have to write more than 1 dicom file
        if (input->GetNumberOfGroups() != 1)
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

  std::vector<BaseData::Pointer> DICOMSegmentationIO::DoRead()
  {
    mitk::LocaleSwitch localeSwitch("C");

    MultiLabelSegmentation::Pointer labelSetImage;
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

      //Get the value of SegmentsOverlap Tag (0062,0013) for this dataset
      OFString overlapValue;
      bool assumeOverlappingSegments = true;
      status = dataSet->findAndGetOFString(DCM_SegmentsOverlap, overlapValue);
      if (status.good())
      {
        assumeOverlappingSegments = "NO" != overlapValue     //DCM allows only NO, YES and UNDEFINED
                                    && "no" != overlapValue  //never the less we add lower and mixed case
                                    && "No" != overlapValue; //version to be more robust with non-compliant DCM files
      }

      //=============================== dcmqi part ====================================
      // Read the DICOM SEG images (segItkImages) and DICOM tags (metaInfo)
      auto converter = std::make_unique<dcmqi::Dicom2ItkConverter>();
      std::string metaInfoString;
      auto convert_condition = converter->dcmSegmentation2itkimage(dataSet, metaInfoString, false);

      std::vector<itkInternalImageType::Pointer> segItkImages;

      if (convert_condition.good())
      {
        auto image = converter->begin();
        while (image.IsNotNull())
        {
          segItkImages.emplace_back(image);
          image = converter->next();
        }
      }

      dcmqi::JSONSegmentationMetaInformationHandler metaInfo(metaInfoString.c_str());
      metaInfo.read();

      MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();
      //===============================================================================

      // Get the label information from segment attributes for each itk image
      vector<map<unsigned, dcmqi::SegmentAttributes *>>::const_iterator segmentIter =
        metaInfo.segmentsAttributesMappingList.begin();

      // For each itk image add a layer to the MultiLabelSegmentation output
      for (auto &segItkImage : segItkImages)
      {
        // Get the labeled image and cast it to mitkImage
        typedef itk::CastImageFilter<itkInternalImageType, itkInputImageType> castItkImageFilterType;
        castItkImageFilterType::Pointer castFilter = castItkImageFilterType::New();
        castFilter->SetInput(segItkImage);
        castFilter->Update();

        Image::Pointer segmentImage;
        CastToMitkImage(castFilter->GetOutput(), segmentImage);

        // Get pixel value of the label
        itkInternalImageType::ValueType segValue = 1;
        typedef itk::ImageRegionIterator<const itkInternalImageType> IteratorType;
        // Iterate over the image to find the pixel value of the label
        IteratorType iter(segItkImage, segItkImage->GetLargestPossibleRegion());
        iter.GoToBegin();
        while (!iter.IsAtEnd())
        {
          itkInputImageType::PixelType value = iter.Get();
          if (value != MultiLabelSegmentation::UNLABELED_VALUE)
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

        OFString labelName = segmentAttribute->getSegmentLabel();

        if (labelName.empty())
        {
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
        }

        float tmp[3] = { 0.0, 0.0, 0.0 };
        if (segmentAttribute->getRecommendedDisplayRGBValue() != nullptr)
        {
          tmp[0] = segmentAttribute->getRecommendedDisplayRGBValue()[0] / 255.0;
          tmp[1] = segmentAttribute->getRecommendedDisplayRGBValue()[1] / 255.0;
          tmp[2] = segmentAttribute->getRecommendedDisplayRGBValue()[2] / 255.0;
        }

        Label::Pointer newLabel = nullptr;
        // If labelSetImage do not exists (first image)
        if (labelSetImage.IsNull())
        {
          // Initialize the labelSetImage with the read image
          labelSetImage = MultiLabelSegmentation::New();
          labelSetImage->InitializeByLabeledImage(segmentImage);
          // Check if the segment image contained labeled pixels. At this point it either contains no (when no labeled pixels where in the image)
          // or one label (as DCMSeg segments only represent one labels). So either generate a new label or used the only existing one.
          newLabel = labelSetImage->GetTotalNumberOfLabels() > 0 ? labelSetImage->GetLabels().front() : Label::New();
          newLabel->SetName(labelName.c_str());
          newLabel->SetColor(Color(tmp));
          newLabel->SetValue(segValue);
        }
        else
        {
          MultiLabelSegmentation::GroupIndexType groupID = 0;
          if (assumeOverlappingSegments)
          {
            // Add a new group because we have to expect every label to be overlapping
            // the label content is directly transferred here.
            groupID = labelSetImage->AddGroup(segmentImage);
          }

          // Add the new label
          newLabel = Label::New();
          newLabel->SetName(labelName.c_str());
          newLabel->SetColor(Color(tmp));
          newLabel->SetValue(segValue);
          labelSetImage->AddLabel(newLabel, groupID, true, true);

          if (!assumeOverlappingSegments)
          {
            //if we know the labels are non overlapping we can put everything in one image
            //the label content has to be transferred, as no new group was added.
            mitk::TransferLabelContent(segmentImage, labelSetImage->GetGroupImage(groupID),
              labelSetImage->GetConstLabelsByValue(labelSetImage->GetLabelValuesByGroup(groupID)),
              mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, {{segValue,newLabel->GetValue()}});
          }

        }

        // Add some more label properties
        this->SetLabelProperties(newLabel, segmentAttribute);
        ++segmentIter;
      }

      labelSetImage->SetAllLabelsVisible(true);

      if (labelSetImage->GetTotalNumberOfLabels() > 0)
      {
        labelSetImage->SetActiveLabel(labelSetImage->GetAllLabelValues().front());
      }

      // Add some general DICOM Segmentation properties
      mitk::IDICOMTagsOfInterest *toiSrv = DICOMIOHelper::GetTagsOfInterestService();
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

      auto findings = DICOMIOHelper::ExtractPathsOfInterest(tagsOfInterestList, frames);
      DICOMIOHelper::SetProperties(labelSetImage, findings);
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
    const mitk::MultiLabelSegmentation *image = dynamic_cast<const mitk::MultiLabelSegmentation *>(this->GetInput());

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

    auto labelSet = image->GetConstLabelsByValue(image->GetLabelValuesByGroup(layer));

    unsigned int segmentNumber = 0;

    for (const auto& label : labelSet)
    {
      ++segmentNumber;
      if (label != nullptr)
      {
        auto segmentAttribute = handler.createOrGetSegment(segmentNumber, label->GetValue());

        if (segmentAttribute != nullptr)
        {
          segmentAttribute->setSegmentLabel(label->GetName());
          segmentAttribute->setSegmentDescription(label->GetDescription());
          std::string algorithmType = label->GetAlgorithmTypeStr();
          if (algorithmType.empty())
            algorithmType = "MANUAL"; //DICOM always needs a type. If undefined we default to "MANUAL"
          segmentAttribute->setSegmentAlgorithmType(algorithmType);
          segmentAttribute->setSegmentAlgorithmName(label->GetAlgorithmName());

          if (label->GetAnatomicRegionCount()>0)
          { //Anatomic region
            auto anatomicRegion = label->GetAnatomicRegion(0); //currently DCMQI only supports one anatomic region code

            if (!anatomicRegion.GetValue().empty() && !anatomicRegion.GetScheme().empty() && !anatomicRegion.GetMeaning().empty())
            {
              segmentAttribute->setAnatomicRegionSequence(anatomicRegion.GetValue(),
                anatomicRegion.GetScheme(), anatomicRegion.GetMeaning());
              handler.setBodyPartExamined(anatomicRegion.GetMeaning());
            }
            else
            {
              // some default values
              segmentAttribute->setSegmentedPropertyTypeCodeSequence("M-03000", "SRT", "Mass");
              handler.setBodyPartExamined("Mass");
            }

            if (anatomicRegion.GetModifierCount() > 0)
            {
              //Anatomic Region Modifier (DCMQI only supports one modifier)
              auto arMod = anatomicRegion.GetModifier(0);

              if (!arMod.GetValue().empty() && !arMod.GetScheme().empty() && !arMod.GetMeaning().empty())
              {
                segmentAttribute->setAnatomicRegionModifierSequence(arMod.GetValue(),
                  arMod.GetScheme(), arMod.GetMeaning());
              }
            }

          }

          { //Segment category
            auto segCat = label->GetSegmentedPropertyCategory();

            if (segCat.has_value() && !segCat->GetValue().empty() && !segCat->GetScheme().empty() && !segCat->GetMeaning().empty())
            {
              segmentAttribute->setSegmentedPropertyCategoryCodeSequence(segCat->GetValue(),
                segCat->GetScheme(), segCat->GetMeaning());
              handler.setBodyPartExamined(segCat->GetMeaning());
            }
            else
            {
              // some default values
              segmentAttribute->setSegmentedPropertyCategoryCodeSequence(
                "M-01000", "SRT", "Morphologically Altered Structure");
            }
          }

          {
            auto segType = label->GetSegmentedPropertyType();

            if (segType.has_value() && !segType->GetValue().empty() && !segType->GetScheme().empty() && !segType->GetMeaning().empty())
            {
              segmentAttribute->setSegmentedPropertyTypeCodeSequence(segType->GetValue(),
                segType->GetScheme(), segType->GetMeaning());
            }
            else
            {
              // some default values
              segmentAttribute->setSegmentedPropertyTypeCodeSequence("M-03000", "SRT", "Mass");
            }

            if (segType.has_value() && segType->GetModifierCount() > 0)
            {
              //Segment Type Modifier (DCMQI only supports one modifier
              auto segTypeMod = segType->GetModifier(0);

              if (!segTypeMod.GetValue().empty() && !segTypeMod.GetScheme().empty() && !segTypeMod.GetMeaning().empty())
              {
                segmentAttribute->setSegmentedPropertyTypeModifierCodeSequence(segTypeMod.GetValue(),
                  segTypeMod.GetScheme(), segTypeMod.GetMeaning());
              }
            }
          }

          Color color = label->GetColor();
          segmentAttribute->setRecommendedDisplayRGBValue(color[0] * 255, color[1] * 255, color[2] * 255);

          if (!label->GetTrackingID().empty())
            segmentAttribute->setTrackingIdentifier(label->GetTrackingID());
          if (!label->GetTrackingUID().empty())
            segmentAttribute->setTrackingUniqueIdentifier(label->GetTrackingUID());
        }
      }
    }
    return handler.getJSONOutputAsString();
  }

  void mitk::DICOMSegmentationIO::SetLabelProperties(mitk::Label *label, dcmqi::SegmentAttributes *segmentAttribute)
  {
    // Segment Algorithm Type: Type of algorithm used to generate the segment.
    label->SetAlgorithmTypeStr(segmentAttribute->getSegmentAlgorithmType());
    label->SetAlgorithmName(segmentAttribute->getSegmentAlgorithmName());

    // Add Segmented Property Category Code Sequence tags
    auto categoryCodeSequence = segmentAttribute->getSegmentedPropertyCategoryCodeSequence();
    if (categoryCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      categoryCodeSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      categoryCodeSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      categoryCodeSequence->getCodeMeaning(codeMeaning);

      label->SetSegmentedPropertyCategory(DICOMCodeSequence(codeValue, codeScheme, codeValue));
    }

    // Add Segmented Property Type Code Sequence tags
    auto typeCodeSequence = segmentAttribute->getSegmentedPropertyTypeCodeSequence();
    if (typeCodeSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      typeCodeSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      typeCodeSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      typeCodeSequence->getCodeMeaning(codeMeaning);

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeValue);

      // Add Segmented Property Type Modifier Code Sequence tags
      auto modifierCodeSequence = segmentAttribute->getSegmentedPropertyTypeModifierCodeSequence();
      if (modifierCodeSequence != nullptr)
      {
        OFString modValue; // (0008,0100) Code Value
        modifierCodeSequence->getCodeValue(modValue);
        OFString modScheme; // (0008,0102) Coding Scheme Designator
        modifierCodeSequence->getCodingSchemeDesignator(modScheme);
        OFString modMeaning; // (0008,0104) Code Meaning
        modifierCodeSequence->getCodeMeaning(modMeaning);
        code.AddModifier(DICOMCodeSequence(modValue, modScheme, modMeaning));
      }

      label->SetSegmentedPropertyType(code);
    }

    // Add Anatomic RegionSequence tags
    auto atomicRegionSequence = segmentAttribute->getAnatomicRegionSequence();
    if (atomicRegionSequence != nullptr)
    {
      OFString codeValue; // (0008,0100) Code Value
      atomicRegionSequence->getCodeValue(codeValue);
      OFString codeScheme; // (0008,0102) Coding Scheme Designator
      atomicRegionSequence->getCodingSchemeDesignator(codeScheme);
      OFString codeMeaning; // (0008,0104) Code Meaning
      atomicRegionSequence->getCodeMeaning(codeMeaning);

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeValue);
      // Add Segmented Property Type Modifier Code Sequence tags
      auto modifierCodeSequence = segmentAttribute->getAnatomicRegionModifierSequence();
      if (modifierCodeSequence != nullptr)
      {
        OFString modValue; // (0008,0100) Code Value
        modifierCodeSequence->getCodeValue(modValue);
        OFString modScheme; // (0008,0102) Coding Scheme Designator
        modifierCodeSequence->getCodingSchemeDesignator(modScheme);
        OFString modMeaning; // (0008,0104) Code Meaning
        modifierCodeSequence->getCodeMeaning(modMeaning);
        code.AddModifier(DICOMCodeSequence(modValue, modScheme, modMeaning));
      }

      label->SetAnatomicRegion(code);
    }

    //we always set the tracking information (even if it is an empty string),
    //as MITK label would otherwise derive an tracking ID automatically
    label->SetTrackingID(segmentAttribute->getTrackingIdentifier());
    label->SetTrackingUID(segmentAttribute->getTrackingUniqueIdentifier());
  }

  DICOMSegmentationIO *DICOMSegmentationIO::IOClone() const { return new DICOMSegmentationIO(*this); }
} // namespace

#endif //__mitkDICOMSegmentationIO__cpp
