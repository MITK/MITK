/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationIO.h>

#include <mitkDICOMSegIOMimeTypes.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMIOHelper.h>
#include <mitkDICOMProperty.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkLocaleSwitch.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>


// itk
#include <itkThresholdImageFilter.h>

// dcmqi
#include <dcmqi/Itk2DicomConverter.h>
#include <dcmqi/Dicom2ItkConverterBase.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmfg/fgderimg.h>
#include <dcmtk/dcmfg/fginterface.h>
#include <dcmtk/dcmfg/fgtypes.h>
#include <dcmtk/dcmseg/segdoc.h>

namespace
{
  // Read SegmentsOverlap (0062,0013) and decide if the reader must assume
  // overlapping segments. Used only by the binary read branch: each segment
  // image arrives separately and the reader has to choose between one shared
  // MITK group (non-overlap) and one group per segment (overlap). The labelmap
  // (Sup 243) branch ignores the flag because labelmap encoding already
  // forbids overlap within a single SEG file.
  bool ShouldAssumeOverlappingSegments(DcmDataset &dataSet)
  {
    OFString overlapValue;
    if (dataSet.findAndGetOFString(DCM_SegmentsOverlap, overlapValue).bad())
      return true;
    // DCM permits NO, YES, UNDEFINED; lower/mixed case forms are tolerated
    // here to stay robust against non-compliant producers.
    return "NO" != overlapValue && "no" != overlapValue && "No" != overlapValue;
  }

  // Resolve a display name for a DICOM SEG segment. Mirrors the historical
  // fallback chain: SegmentLabel -> SegmentedPropertyType code meaning (with
  // optional modifier) -> string form of the numeric segment id.
  OFString DeriveLabelName(dcmqi::SegmentAttributes &segmentAttribute)
  {
    OFString labelName = segmentAttribute.getSegmentLabel();
    if (!labelName.empty())
      return labelName;

    if (segmentAttribute.getSegmentedPropertyTypeCodeSequence() != nullptr)
    {
      segmentAttribute.getSegmentedPropertyTypeCodeSequence()->getCodeMeaning(labelName);
      if (segmentAttribute.getSegmentedPropertyTypeModifierCodeSequence() != nullptr)
      {
        OFString modifier;
        segmentAttribute.getSegmentedPropertyTypeModifierCodeSequence()->getCodeMeaning(modifier);
        labelName.append(" (").append(modifier).append(")");
      }
      return labelName;
    }

    return OFString(std::to_string(segmentAttribute.getLabelID()).c_str());
  }

  // One source-image reference seen on one SEG frame. Frame index is the
  // SEG's own frame numbering (0-based) and is later used as the slice
  // index in the TemporoSpatialStringProperty handed to the relation rule.
  struct SegSourceFrameRef
  {
    Uint32 frameIndex;
    std::string sopInstanceUID;
    std::string sopClassUID;
  };

  // A set of per-frame source references that share one source series.
  // The DICOM SEG IOD's top-level ReferencedSeriesSequence has exactly one
  // item per source series, so this 1:1 corresponds to one connect call on
  // the relation rule.
  struct SegSourceSeriesGroup
  {
    std::string seriesInstanceUID;
    std::vector<SegSourceFrameRef> frames;
  };

  // Frames that carry no derivation reference are silently skipped: the
  // SEG IOD makes the Derivation Image FG type 1C, so absence is DICOM-legal
  // and simply means "no source image is recorded for that frame."
  // Parameter is non-const because DCMTK's getFunctionalGroups and
  // getNumberOfFrames have no const overload.
  std::vector<SegSourceFrameRef> CollectPerFrameSourceRefs(DcmSegmentation& segDoc)
  {
    std::vector<SegSourceFrameRef> result;
    FGInterface& fgInterface = segDoc.getFunctionalGroups();
    const size_t numFrames = segDoc.getNumberOfFrames();

    for (size_t f = 0; f < numFrames; ++f)
    {
      OFBool isPerFrame = OFFalse;
      auto* fg = fgInterface.get(static_cast<Uint32>(f), DcmFGTypes::EFG_DERIVATIONIMAGE, isPerFrame);
      auto* derImg = OFstatic_cast(FGDerivationImage*, fg);
      if (derImg == nullptr)
        continue;

      OFVector<DerivationImageItem*>& derItems = derImg->getDerivationImageItems();
      for (auto* derItem : derItems)
      {
        if (derItem == nullptr)
          continue;
        OFVector<SourceImageItem*>& srcItems = derItem->getSourceImageItems();
        for (auto* srcItem : srcItems)
        {
          if (srcItem == nullptr)
            continue;
          OFString sopInstance;
          OFString sopClass;
          // ImageSOPInstanceReferenceMacro inherits the two getters from
          // SOPInstanceReferenceMacro; the macro is held by value on the
          // SourceImageItem.
          srcItem->getImageSOPInstanceReference().getReferencedSOPInstanceUID(sopInstance);
          srcItem->getImageSOPInstanceReference().getReferencedSOPClassUID(sopClass);
          if (sopInstance.empty())
            continue;
          result.push_back({static_cast<Uint32>(f), sopInstance.c_str(), sopClass.c_str()});
        }
      }
    }

    return result;
  }

  // An empty result means the SEG has no top-level series references at
  // all (DICOM-legal type 1C absence) and the per-series grouping below
  // degenerates to "no relations." Parameter is non-const because DCMTK's
  // findAndGet* are not const-overloaded.
  std::map<std::string, std::set<std::string>>
    CollectSourceInstancesBySeries(DcmDataset& dataset)
  {
    std::map<std::string, std::set<std::string>> result;
    DcmSequenceOfItems* refSeriesSeq = nullptr;
    if (dataset.findAndGetSequence(DCM_ReferencedSeriesSequence, refSeriesSeq).bad()
        || refSeriesSeq == nullptr)
      return result;

    for (unsigned long i = 0; i < refSeriesSeq->card(); ++i)
    {
      DcmItem* item = refSeriesSeq->getItem(i);
      if (item == nullptr)
        continue;

      OFString seriesUID;
      if (item->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID).bad() || seriesUID.empty())
        continue;

      auto& instanceSet = result[seriesUID.c_str()];

      DcmSequenceOfItems* refInstSeq = nullptr;
      if (item->findAndGetSequence(DCM_ReferencedInstanceSequence, refInstSeq).bad()
          || refInstSeq == nullptr)
        continue;

      for (unsigned long j = 0; j < refInstSeq->card(); ++j)
      {
        DcmItem* instItem = refInstSeq->getItem(j);
        if (instItem == nullptr)
          continue;
        OFString sopInstance;
        if (instItem->findAndGetOFString(DCM_ReferencedSOPInstanceUID, sopInstance).good()
            && !sopInstance.empty())
        {
          instanceSet.insert(sopInstance.c_str());
        }
      }
    }

    return result;
  }

  // Invert the per-frame refs against the series map. Frames whose source
  // SOPInstanceUID matches no known series are warned about and dropped
  // from any group: this indicates a malformed SEG, not a reader-side
  // failure mode, so the seg still loads but the unresolved frame is not
  // attributed to a relation.
  std::vector<SegSourceSeriesGroup> GroupPerFrameRefsBySeries(
    const std::vector<SegSourceFrameRef>& frameRefs,
    const std::map<std::string, std::set<std::string>>& seriesToInstances)
  {
    std::vector<SegSourceSeriesGroup> groups;
    if (seriesToInstances.empty())
      return groups;

    std::map<std::string, size_t> seriesToGroupIndex;
    for (const auto& frameRef : frameRefs)
    {
      const std::string* matchedSeries = nullptr;
      for (const auto& [seriesUID, instanceSet] : seriesToInstances)
      {
        if (instanceSet.count(frameRef.sopInstanceUID) > 0)
        {
          matchedSeries = &seriesUID;
          break;
        }
      }

      if (matchedSeries == nullptr)
      {
        MITK_WARN << "DICOM SEG references source SOP Instance " << frameRef.sopInstanceUID
                  << " on frame " << frameRef.frameIndex
                  << " but the SEG's ReferencedSeriesSequence does not declare it; "
                  << "skipping frame from source-image relation population.";
        continue;
      }

      auto [it, inserted] = seriesToGroupIndex.try_emplace(*matchedSeries, groups.size());
      if (inserted)
      {
        SegSourceSeriesGroup g;
        g.seriesInstanceUID = *matchedSeries;
        groups.push_back(std::move(g));
      }
      groups[it->second].frames.push_back(frameRef);
    }

    return groups;
  }

  // The PropertyList is an ad-hoc IPropertyProvider built only to feed
  // the rule's instance Connect overload; it has no lifetime beyond this
  // scope.
  void PopulateSourceImageRelations(mitk::MultiLabelSegmentation& seg,
                                    const std::vector<SegSourceSeriesGroup>& groups)
  {
    if (groups.empty())
      return;

    auto rule = mitk::SegSourceImageRelationRule::New();

    for (const auto& group : groups)
    {
      auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
      auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
      for (const auto& frameRef : group.frames)
      {
        // Single time step in the SEG IOD; frame -> slice index 1:1 as
        // described on SegSourceFrameRef.
        perSliceInstance->SetValue(0, frameRef.frameIndex, frameRef.sopInstanceUID);
        perSliceClass->SetValue(0, frameRef.frameIndex, frameRef.sopClassUID);
      }

      auto provider = mitk::PropertyList::New();
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0018).c_str(),
                            perSliceInstance);
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0016).c_str(),
                            perSliceClass);
      provider->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str(),
                            mitk::TemporoSpatialStringProperty::New(group.seriesInstanceUID));

      rule->Connect(&seg, provider.GetPointer());
    }
  }
}

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

        if (result == nullptr)
          mitkThrow() << "dcmqi failed to convert the segmentation to DICOM SEG for group " << layer << ".";

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

      //=============================== dcmqi part ====================================
      // getConverter() is a SOP-Class-specific factory: UID_SegmentationStorage
      // returns the binary converter, UID_LabelMapSegmentationStorage (Sup 243)
      // returns the labelmap converter. The two paths build the
      // MultiLabelSegmentation differently (per-segment vs single labelmap
      // image), so dispatch happens once here and the helpers diverge from
      // there.
      std::unique_ptr<dcmqi::Dicom2ItkConverterBase> converter(
        dcmqi::Dicom2ItkConverter::getConverter(dataSet));
      if (converter == nullptr)
        mitkThrow() << "Unsupported DICOM SEG SOP Class; cannot read.";

      std::string metaInfoString;
      auto convertCondition = converter->dcmSegmentation2itkimage(dataSet, metaInfoString, false);
      if (convertCondition.bad())
        mitkThrow() << "dcmqi failed to convert DICOM SEG: "
                    << convertCondition.text();

      dcmqi::JSONSegmentationMetaInformationHandler metaInfo(metaInfoString.c_str());
      metaInfo.read();

      MITK_INFO << "Input " << metaInfo.getJSONOutputAsString();
      //===============================================================================

      if (converter->isLabelmap())
      {
        labelSetImage = this->ReadLabelmapSegmentation(*converter, metaInfo);
      }
      else
      {
        labelSetImage = this->ReadBinarySegmentation(
          *converter, metaInfo, ShouldAssumeOverlappingSegments(*dataSet));
      }

      if (labelSetImage.IsNull())
        mitkThrow() << "No valid segments found in DICOM SEG file.";

      labelSetImage->SetAllLabelsVisible(true);

      if (labelSetImage->GetTotalNumberOfLabels() > 0)
      {
        labelSetImage->SetActiveLabel(labelSetImage->GetAllLabelValues().front());
      }

      // Add some general DICOM Segmentation properties
      mitk::IDICOMTagsOfInterest *toiSrv = DICOMIOHelper::GetTagsOfInterestService();
      if (toiSrv == nullptr)
        mitkThrow() << "DICOM tags-of-interest service is not available.";

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

      // Populate after SetProperties so the per-slice TemporoSpatialString
      // properties this writes (DICOM.0008.2112.[0].0008.1155 and friends)
      // cannot be flattened by the tag-of-interest scanner. Parse from the
      // already-loaded dataset rather than re-reading the file: dcmqi above
      // does not take ownership of dcmFileFormat's dataset and dcmFileFormat
      // outlives this block, so loadDataset avoids a second disk read.
      DcmSegmentation* segDocRaw = nullptr;
      OFCondition loadSegCond = DcmSegmentation::loadDataset(*dataSet, segDocRaw);
      std::unique_ptr<DcmSegmentation> segDoc(segDocRaw);
      if (loadSegCond.bad() || segDoc == nullptr)
        mitkThrow() << "Failed to parse DICOM SEG via DcmSegmentation::loadDataset: "
                    << loadSegCond.text();

      const auto frameRefs = CollectPerFrameSourceRefs(*segDoc);
      const auto seriesToInstances = CollectSourceInstancesBySeries(*dataSet);
      const auto sourceSeriesGroups = GroupPerFrameRefsBySeries(frameRefs, seriesToInstances);
      PopulateSourceImageRelations(*labelSetImage, sourceSeriesGroups);
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

  mitk::MultiLabelSegmentation::Pointer DICOMSegmentationIO::ReadBinarySegmentation(
    dcmqi::Dicom2ItkConverterBase &converter,
    dcmqi::JSONSegmentationMetaInformationHandler &metaInfo,
    bool assumeOverlappingSegments)
  {
    std::vector<itkInternalImageType::Pointer> segItkImages;
    auto image = converter.begin16Bit();
    while (image.IsNotNull())
    {
      segItkImages.emplace_back(image);
      image = converter.next16Bit();
    }

    if (segItkImages.empty())
      mitkThrow() << "DICOM SEG converted successfully but yielded no "
                  << "segment images; cannot construct a MultiLabelSegmentation.";

    MultiLabelSegmentation::Pointer labelSetImage;

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
      if (segmentIter == metaInfo.segmentsAttributesMappingList.end())
        mitkThrow() << "Segment metadata list has fewer entries than segment images.";

      const auto &segmentMap = (*segmentIter);
      if (segmentMap.empty())
        mitkThrow() << "Segment metadata entry is empty for segment image.";

      dcmqi::SegmentAttributes *segmentAttribute = segmentMap.begin()->second;
      if (segmentAttribute == nullptr)
        mitkThrow() << "Segment attributes are null for segment image.";

      const OFString labelName = DeriveLabelName(*segmentAttribute);

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

    return labelSetImage;
  }

  mitk::MultiLabelSegmentation::Pointer DICOMSegmentationIO::ReadLabelmapSegmentation(
    dcmqi::Dicom2ItkConverterBase &converter,
    dcmqi::JSONSegmentationMetaInformationHandler &metaInfo)
  {
    const Uint8 bytesPerPixel = converter.bytesPerPixel();
    if (bytesPerPixel != 1 && bytesPerPixel != 2)
      mitkThrow() << "Unsupported labelmap pixel size: bytesPerPixel="
                  << static_cast<unsigned>(bytesPerPixel)
                  << " (expected 1 or 2 per dcmqi's Sup 243 converter).";

    // dcmqi's labelmap converter emits exactly one image carrying every
    // segment number as its pixel values. The multi-image guard surfaces
    // a future dcmqi change explicitly instead of silently dropping all
    // but the first image.
    Image::Pointer mitkSegImage;
    int imageCount = 0;
    if (bytesPerPixel == 2)
    {
      auto image = converter.begin16Bit();
      while (image.IsNotNull())
      {
        if (++imageCount > 1)
          mitkThrow() << "Labelmap DICOM SEG converter yielded more than one "
                      << "image; Sup 243 expects exactly one per SEG.";
        typedef itk::CastImageFilter<itkInternalImageType, itkInputImageType> CastFilter;
        auto castFilter = CastFilter::New();
        castFilter->SetInput(image);
        castFilter->Update();
        CastToMitkImage(castFilter->GetOutput(), mitkSegImage);
        image = converter.next16Bit();
      }
    }
    else // bytesPerPixel == 1
    {
      typedef itk::Image<unsigned char, 3> Uint8ImageType;
      auto image = converter.begin8Bit();
      while (image.IsNotNull())
      {
        if (++imageCount > 1)
          mitkThrow() << "Labelmap DICOM SEG converter yielded more than one "
                      << "image; Sup 243 expects exactly one per SEG.";
        typedef itk::CastImageFilter<Uint8ImageType, itkInputImageType> CastFilter;
        auto castFilter = CastFilter::New();
        castFilter->SetInput(image);
        castFilter->Update();
        CastToMitkImage(castFilter->GetOutput(), mitkSegImage);
        image = converter.next8Bit();
      }
    }

    if (imageCount == 0)
      mitkThrow() << "Labelmap DICOM SEG converted successfully but yielded "
                  << "no image; cannot construct a MultiLabelSegmentation.";

    if (metaInfo.segmentsAttributesMappingList.empty())
      mitkThrow() << "Labelmap DICOM SEG yielded no segment attribute entries.";

    // The SEG's Segment Sequence is authoritative for what labels exist;
    // pixel values are content. Sup 243 may declare a Background segment
    // (number 0); MITK treats pixel value 0 as UNLABELED, so background
    // entries are dropped here to avoid colliding with foreground handling.
    const auto &segmentMap = metaInfo.segmentsAttributesMappingList.front();
    std::map<MultiLabelSegmentation::LabelValueType, dcmqi::SegmentAttributes *> attributesByLabelValue;
    for (const auto &segmentEntry : segmentMap)
    {
      dcmqi::SegmentAttributes *segmentAttribute = segmentEntry.second;
      if (segmentAttribute == nullptr)
        continue;
      const auto labelValue =
        static_cast<MultiLabelSegmentation::LabelValueType>(segmentAttribute->getLabelID());
      if (labelValue == MultiLabelSegmentation::UNLABELED_VALUE)
        continue;
      attributesByLabelValue.emplace(labelValue, segmentAttribute);
    }

    // The labelmap image's pixel values are the segment numbers, so
    // InitializeByLabeledImage builds one MITK group with one auto-created
    // Label per distinct value. The labels are then re-driven from metadata
    // below; auto-creation is only used to bind label values to pixel-grid
    // content.
    auto labelSetImage = MultiLabelSegmentation::New();
    labelSetImage->InitializeByLabeledImage(mitkSegImage);

    // Sup 243 requires every non-zero pixel value to be described by a
    // Segment Sequence entry. Mismatch in either direction is a
    // non-conformant SEG and throws, matching the binary branch's
    // hard-failure contract (no silent drops).
    for (const auto labelValue : labelSetImage->GetAllLabelValues())
    {
      if (labelValue == MultiLabelSegmentation::UNLABELED_VALUE)
        continue;
      if (attributesByLabelValue.find(labelValue) == attributesByLabelValue.end())
        mitkThrow() << "Labelmap DICOM SEG contains pixel value "
                    << static_cast<unsigned>(labelValue)
                    << " but no matching Segment Sequence entry; SEG is non-conformant.";
    }

    // Drive label naming, colour and DICOM property metadata from
    // segmentsAttributesMappingList. SetLabelProperties also stamps the
    // empty tracking-ID/UID sentinel that suppresses MITK's auto-UID
    // generation, so running it on every metadata-described label keeps
    // round-trip integrity.
    for (const auto &[labelValue, segmentAttribute] : attributesByLabelValue)
    {
      Label *label = labelSetImage->GetLabel(labelValue);
      if (label == nullptr)
        mitkThrow() << "Labelmap DICOM SEG metadata declares segment "
                    << static_cast<unsigned>(labelValue)
                    << " but no pixel with that value is present; SEG is non-conformant.";

      const OFString labelName = DeriveLabelName(*segmentAttribute);
      label->SetName(labelName.c_str());

      if (segmentAttribute->getRecommendedDisplayRGBValue() != nullptr)
      {
        const float rgb[3] = {
          segmentAttribute->getRecommendedDisplayRGBValue()[0] / 255.0f,
          segmentAttribute->getRecommendedDisplayRGBValue()[1] / 255.0f,
          segmentAttribute->getRecommendedDisplayRGBValue()[2] / 255.0f
        };
        label->SetColor(Color(rgb));
      }

      this->SetLabelProperties(label, segmentAttribute);
    }

    return labelSetImage;
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

      label->SetSegmentedPropertyCategory(DICOMCodeSequence(codeValue, codeScheme, codeMeaning));
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

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeMeaning);

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

      DICOMCodeSequenceWithModifiers code = DICOMCodeSequenceWithModifiers(codeValue, codeScheme, codeMeaning);
      // Add Anatomic Region Modifier Code Sequence tags
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
