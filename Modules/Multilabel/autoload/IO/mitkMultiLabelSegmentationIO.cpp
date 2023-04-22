/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationIO.h"
#include "mitkBasePropertySerializer.h"
#include "mitkIOMimeTypes.h"
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

  const constexpr char* const MULTILABEL_SEGMENTATION_MODALITY_KEY = "modality";
  const constexpr char* const MULTILABEL_SEGMENTATION_MODALITY_VALUE = "org.mitk.multilabel.segmentation";
  const constexpr char* const MULTILABEL_SEGMENTATION_VERSION_KEY = "org.mitk.multilabel.segmentation.version";
  const constexpr int MULTILABEL_SEGMENTATION_VERSION_VALUE = 1;
  const constexpr char* const MULTILABEL_SEGMENTATION_LABELS_INFO_KEY = "org.mitk.multilabel.segmentation.labelgroups";
  const constexpr char* const MULTILABEL_SEGMENTATION_UNLABELEDLABEL_LOCK_KEY = "org.mitk.multilabel.segmentation.unlabeledlabellock";

  MultiLabelSegmentationIO::MultiLabelSegmentationIO()
    : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(), IOMimeTypes::NRRD_MIMETYPE(), "MITK Multilabel Segmentation")
  {
    this->InitializeDefaultMetaDataKeys();
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  IFileIO::ConfidenceLevel MultiLabelSegmentationIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;
    const auto *input = static_cast<const LabelSetImage *>(this->GetInput());
    if (input)
      return Supported;
    else
      return Unsupported;
  }

  void MultiLabelSegmentationIO::Write()
  {
    ValidateOutputLocation();

    auto input = dynamic_cast<const LabelSetImage *>(this->GetInput());

    mitk::LocaleSwitch localeSwitch("C");

    mitk::Image::Pointer inputVector = mitk::ConvertLabelSetImageToImage(input);

    // image write
    if (inputVector.IsNull())
    {
      mitkThrow() << "Cannot write non-image data";
    }

    itk::NrrdImageIO::Pointer nrrdImageIo = itk::NrrdImageIO::New();

    ItkImageIO::PreparImageIOToWriteImage(nrrdImageIo, inputVector);

    LocalFile localFile(this);
    const std::string path = localFile.GetFileName();

    MITK_INFO << "Writing image: " << path << std::endl;

    try
    {
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(MULTILABEL_SEGMENTATION_MODALITY_KEY), std::string(MULTILABEL_SEGMENTATION_MODALITY_VALUE));

      //nrrd does only support string meta information. So we have to convert before.
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(MULTILABEL_SEGMENTATION_VERSION_KEY), std::to_string(MULTILABEL_SEGMENTATION_VERSION_VALUE));

      auto json = MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(input);
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(MULTILABEL_SEGMENTATION_LABELS_INFO_KEY), json.dump());
      // end label set specific meta data

      //nrrd does only support string meta information. So we have to convert before.
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(MULTILABEL_SEGMENTATION_UNLABELEDLABEL_LOCK_KEY), std::to_string(input->GetUnlabeledLabelLock()));

      // Handle properties
      ItkImageIO::SavePropertyListAsMetaData(nrrdImageIo->GetMetaDataDictionary(), input->GetPropertyList(), this->GetMimeType()->GetName());

      // Handle UID
      itk::EncapsulateMetaData<std::string>(nrrdImageIo->GetMetaDataDictionary(), PROPERTY_KEY_UID, input->GetUID());

      // use compression if available
      nrrdImageIo->UseCompressionOn();
      nrrdImageIo->SetFileName(path);

      ImageReadAccessor imageAccess(inputVector);
      nrrdImageIo->Write(imageAccess.GetData());
    }
    catch (const std::exception &e)
    {
      mitkThrow() << e.what();
    }
  }

  IFileIO::ConfidenceLevel MultiLabelSegmentationIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;
    const std::string fileName = this->GetLocalFileName();
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(fileName);
    io->ReadImageInformation();

    itk::MetaDataDictionary imgMetaDataDictionary = io->GetMetaDataDictionary();
    std::string value("");
    itk::ExposeMetaData<std::string>(imgMetaDataDictionary, "modality", value);
    if (value.compare(MULTILABEL_SEGMENTATION_MODALITY_VALUE) == 0)
    {
      return Supported;
    }
    else
      return Unsupported;
  }

  std::vector<BaseData::Pointer> MultiLabelSegmentationIO::DoRead()
  {
    itk::NrrdImageIO::Pointer nrrdImageIO = itk::NrrdImageIO::New();

    std::vector<BaseData::Pointer> result;

    auto rawimage = ItkImageIO::LoadRawMitkImageFromImageIO(nrrdImageIO, this->GetLocalFileName());

    const itk::MetaDataDictionary& dictionary = nrrdImageIO->GetMetaDataDictionary();

    //check version
    auto version = MultiLabelIOHelper::GetIntByKey(dictionary, MULTILABEL_SEGMENTATION_VERSION_KEY);
    if (version > MULTILABEL_SEGMENTATION_VERSION_VALUE)
    {
      mitkThrow() << "Data to read has unsupported version. Software is to old to ensure correct reading. Please use a compatible version of MITK or store data in another format. Version of data: " << version << "; Supported versions up to: "<<MULTILABEL_SEGMENTATION_VERSION_VALUE;
    }

    //generate multi label images
    auto output = ConvertImageToLabelSetImage(rawimage);

    //get label set definitions
    auto jsonStr = MultiLabelIOHelper::GetStringByKey(dictionary, MULTILABEL_SEGMENTATION_LABELS_INFO_KEY);
    nlohmann::json jlabelsets = nlohmann::json::parse(jsonStr);
    std::vector<mitk::LabelSet::Pointer> labelsets = MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(jlabelsets);

    if (labelsets.size() != output->GetNumberOfLayers())
    {
      mitkThrow() << "Loaded data is in an invalid state. Number of extracted layer images and labels sets does not match. Found layer images: " << output->GetNumberOfLayers() << "; found labelsets: " << labelsets.size();
    }

    LabelSetImage::GroupIndexType id = 0;
    for (auto labelset : labelsets)
    {
      output->AddLabelSetToLayer(id, labelset);
      id++;
    }

    bool unlabeledLock = MultiLabelIOHelper::GetIntByKey(dictionary, MULTILABEL_SEGMENTATION_UNLABELEDLABEL_LOCK_KEY) != 0;
    output->SetUnlabeledLabelLock(unlabeledLock);

    //meta data handling
    auto props = ItkImageIO::ExtractMetaDataAsPropertyList(nrrdImageIO->GetMetaDataDictionary(), this->GetMimeType()->GetName(), this->m_DefaultMetaDataKeys);
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

    MITK_INFO << "...finished!";
    return result;
  }

  MultiLabelSegmentationIO *MultiLabelSegmentationIO::IOClone() const { return new MultiLabelSegmentationIO(*this); }

  void MultiLabelSegmentationIO::InitializeDefaultMetaDataKeys()
  {
    this->m_DefaultMetaDataKeys.push_back("NRRD.space");
    this->m_DefaultMetaDataKeys.push_back("NRRD.kinds");
    this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TYPE);
    this->m_DefaultMetaDataKeys.push_back(PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS);
    this->m_DefaultMetaDataKeys.push_back("ITK.InputFilterName");
    this->m_DefaultMetaDataKeys.push_back("org.mitk.multilabel.");
    this->m_DefaultMetaDataKeys.push_back("MITK.IO.");
    this->m_DefaultMetaDataKeys.push_back(MULTILABEL_SEGMENTATION_MODALITY_KEY);
  }

} // namespace
