/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkLabelSetImageWriter__cpp
#define __mitkLabelSetImageWriter__cpp

#include "mitkLegacyLabelSetImageIO.h"
#include "mitkBasePropertySerializer.h"
#include "mitkMultilabelIOMimeTypes.h"
#include "mitkImageAccessByItk.h"
#include "mitkMultiLabelIOHelper.h"
#include "mitkLabelSetImageConverter.h"
#include <mitkLocaleSwitch.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkIPropertyPersistence.h>
#include <mitkCoreServices.h>
#include <mitkItkImageIO.h>
#include <mitkUIDManipulator.h>

// itk
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

#include <tinyxml2.h>


namespace mitk
{

  const constexpr char* const OPTION_NAME_MULTI_LAYER = "Multi layer handling";
  const constexpr char* const OPTION_NAME_MULTI_LAYER_ADAPT = "Adapt label values";
  const constexpr char* const OPTION_NAME_MULTI_LAYER_SPLIT = "Split layers";

  LegacyLabelSetImageIO::LegacyLabelSetImageIO()
    : AbstractFileReader(MitkMultilabelIOMimeTypes::LEGACYLABELSET_MIMETYPE(), "MITK LabelSetImage (legacy)")
  {
    this->InitializeDefaultMetaDataKeys();
    AbstractFileReader::SetRanking(10);

    IFileIO::Options options;
    std::vector<std::string> multiLayerStrategy;
    multiLayerStrategy.push_back(OPTION_NAME_MULTI_LAYER_ADAPT);
    multiLayerStrategy.push_back(OPTION_NAME_MULTI_LAYER_SPLIT);
    options[OPTION_NAME_MULTI_LAYER] = multiLayerStrategy;
    this->SetDefaultOptions(options);

    this->RegisterService();
  }


  IFileIO::ConfidenceLevel LegacyLabelSetImageIO::GetConfidenceLevel() const
  {
    if (AbstractFileReader::GetConfidenceLevel() == Unsupported)
      return Unsupported;
    const std::string fileName = this->GetLocalFileName();
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(fileName);
    io->ReadImageInformation();

    itk::MetaDataDictionary imgMetaDataDictionary = io->GetMetaDataDictionary();
    std::string value("");
    itk::ExposeMetaData<std::string>(imgMetaDataDictionary, "modality", value);
    if (value.compare("org.mitk.image.multilabel") == 0)
    {
      return Supported;
    }
    else
      return Unsupported;
  }

  std::vector<mitk::LabelSet::Pointer> ExtractLabelSetsFromMetaData(const itk::MetaDataDictionary& dictionary)
  {
    std::vector<mitk::LabelSet::Pointer> result;

    // get labels and add them as properties to the image
    char keybuffer[256];

    unsigned int numberOfLayers = MultiLabelIOHelper::GetIntByKey(dictionary, "layers");
    std::string _xmlStr;
    mitk::Label::Pointer label;

    for (unsigned int layerIdx = 0; layerIdx < numberOfLayers; layerIdx++)
    {
      sprintf(keybuffer, "layer_%03u", layerIdx);
      int numberOfLabels = MultiLabelIOHelper::GetIntByKey(dictionary, keybuffer);

      mitk::LabelSet::Pointer labelSet = mitk::LabelSet::New();

      for (int labelIdx = 0; labelIdx < numberOfLabels; labelIdx++)
      {
        tinyxml2::XMLDocument doc;
        sprintf(keybuffer, "label_%03u_%05d", layerIdx, labelIdx);
        _xmlStr = MultiLabelIOHelper::GetStringByKey(dictionary, keybuffer);
        doc.Parse(_xmlStr.c_str(), _xmlStr.size());

        auto* labelElem = doc.FirstChildElement("Label");
        if (labelElem == nullptr)
          mitkThrow() << "Error parsing NRRD header for mitk::LabelSetImage IO";

        label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(labelElem);

        if (label->GetValue() != mitk::LabelSetImage::UnlabeledValue)
        {
          labelSet->AddLabel(label);
          labelSet->SetLayer(layerIdx);
        }
        else
        {
          MITK_INFO << "Multi label image contains a label specification for unlabeled pixels. This legacy information is ignored.";
        }
      }
      result.push_back(labelSet);
    }

    return result;
  }

  std::vector<BaseData::Pointer> LegacyLabelSetImageIO::DoRead()
  {
    itk::NrrdImageIO::Pointer nrrdImageIO = itk::NrrdImageIO::New();

    std::vector<BaseData::Pointer> result;

    auto rawimage = ItkImageIO::LoadRawMitkImageFromImageIO(nrrdImageIO, this->GetLocalFileName());

    const itk::MetaDataDictionary& dictionary = nrrdImageIO->GetMetaDataDictionary();

    std::vector<Image::Pointer> groupImages = { rawimage };
    if (rawimage->GetChannelDescriptor().GetPixelType().GetPixelType() == itk::IOPixelEnum::VECTOR)
    {
      groupImages = SplitVectorImage(rawimage);
    }

    auto labelsets = ExtractLabelSetsFromMetaData(dictionary);

    if (labelsets.size() != groupImages.size())
    {
      mitkThrow() << "Loaded data is in an invalid state. Number of extracted layer images and labels sets does not match. Found layer images: " << groupImages.size() << "; found labelsets: " << labelsets.size();
    }

    auto props = ItkImageIO::ExtractMetaDataAsPropertyList(nrrdImageIO->GetMetaDataDictionary(), this->GetMimeType()->GetName(), this->m_DefaultMetaDataKeys);

    const Options userOptions = this->GetOptions();

    const auto multiLayerStrategy = userOptions.find(OPTION_NAME_MULTI_LAYER)->second.ToString();

    if (multiLayerStrategy == OPTION_NAME_MULTI_LAYER_SPLIT)
    { //just split layers in different multi label images
      auto labelSetIterator = labelsets.begin();
      for (auto image : groupImages)
      {
        auto output = ConvertImageToLabelSetImage(image);
        output->AddLabelSetToLayer(0, *labelSetIterator);
        output->GetLabelSet(0)->SetLayer(0);

        //meta data handling
        for (auto& [name, prop] : *(props->GetMap()))
        {
          output->SetProperty(name, prop->Clone()); //need to clone to avoid that all outputs pointing to the same prop instances.
        }
        // Handle UID
        //Remark if we split the legacy label set into distinct layer images, the outputs should have new IDs. So we don't get the old one.

        result.push_back(output.GetPointer());
        labelSetIterator++;
      }
    }
    else
    { //Avoid label id collision.
      LabelSetImage::LabelValueType maxValue = LabelSetImage::UnlabeledValue;
      auto imageIterator = groupImages.begin();
      std::vector<mitk::LabelSet::Pointer> adaptedLabelSets;

      for (auto labelset : labelsets)
      {
        const auto setValues = labelset->GetUsedLabelValues();

        //generate mapping table;
        std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping;
        for (auto vIter = setValues.crbegin(); vIter != setValues.crend(); vIter++)
        { //have to use reverse loop because TransferLabelContent (used to adapt content in the same image; see below)
          //would potentially corrupt otherwise the content due to "value collision between old values still present
          //and already adapted values. By going from highest value to lowest, we avoid that.
          if (LabelSetImage::UnlabeledValue != *vIter)
            labelMapping.push_back({ *vIter, *vIter + maxValue });
        }


        if (LabelSetImage::UnlabeledValue != maxValue)
        {
          //adapt labelset
          auto mappedLabelSet = GenerateLabelSetWithMappedValues(labelset, labelMapping);
          adaptedLabelSets.emplace_back(mappedLabelSet);

          //adapt image (it is an inplace operation. the image instance stays the same.
          TransferLabelContent(*imageIterator, *imageIterator, mappedLabelSet, LabelSetImage::UnlabeledValue, LabelSetImage::UnlabeledValue,
            false, labelMapping, MultiLabelSegmentation::MergeStyle::Replace, MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
        }
        else
        {
          adaptedLabelSets.emplace_back(labelset);
        }

        const auto setMaxValue = *(std::max_element(setValues.begin(), setValues.end()));
        maxValue += setMaxValue;
        imageIterator++;
      }

      auto output = ConvertImageVectorToLabelSetImage(groupImages, rawimage->GetTimeGeometry());

      LabelSetImage::GroupIndexType id = 0;
      for (auto labelset : adaptedLabelSets)
      {
        output->AddLabelSetToLayer(id, labelset);
        id++;
      }

      //meta data handling
      for (auto& [name, prop] : *(props->GetMap()))
      {
        output->SetProperty(name, prop->Clone()); //need to clone to avoid that all outputs pointing to the same prop instances.
      }

      // Handle UID
      if (dictionary.HasKey(PROPERTY_KEY_UID))
      {
        itk::MetaDataObject<std::string>::ConstPointer uidData = dynamic_cast<const itk::MetaDataObject<std::string>*>(dictionary.Get(PROPERTY_KEY_UID));
        if (uidData.IsNotNull())
        {
          mitk::UIDManipulator uidManipulator(output);
          uidManipulator.SetUID(uidData->GetMetaDataObjectValue());
        }
      }
      result.push_back(output.GetPointer());
    }

    MITK_INFO << "...finished!";
    return result;
  }

  LegacyLabelSetImageIO *LegacyLabelSetImageIO::Clone() const { return new LegacyLabelSetImageIO(*this); }

  void LegacyLabelSetImageIO::InitializeDefaultMetaDataKeys()
  {
    this->m_DefaultMetaDataKeys.push_back("NRRD.space");
    this->m_DefaultMetaDataKeys.push_back("NRRD.kinds");
    this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TYPE);
    this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS);
    this->m_DefaultMetaDataKeys.push_back("ITK.InputFilterName");
    this->m_DefaultMetaDataKeys.push_back("label.");
    this->m_DefaultMetaDataKeys.push_back("layer.");
    this->m_DefaultMetaDataKeys.push_back("layers");
    this->m_DefaultMetaDataKeys.push_back("modality");
    this->m_DefaultMetaDataKeys.push_back("org.mitk.label.");
    this->m_DefaultMetaDataKeys.push_back("MITK.IO.");
  }

} // namespace

#endif //__mitkLabelSetImageWriter__cpp
