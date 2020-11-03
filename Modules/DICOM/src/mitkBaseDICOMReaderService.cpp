/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBaseDICOMReaderService.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkImage.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
#include "legacy/mitkDicomSeriesReader.h"
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkLocaleSwitch.h>
#include "mitkIPropertyProvider.h"
#include "mitkPropertyNameHelper.h"
#include "mitkPropertyKeyPath.h"
#include "mitkDICOMIOMetaInformationPropertyConstants.h"

#include <iostream>


#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

namespace mitk
{

  BaseDICOMReaderService::BaseDICOMReaderService(const std::string& description)
    : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), description)
{
}

BaseDICOMReaderService::BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description)
  : AbstractFileReader(customType, description)
{
}

void BaseDICOMReaderService::SetOnlyRegardOwnSeries(bool regard)
{
  m_OnlyRegardOwnSeries = regard;
}

bool BaseDICOMReaderService::GetOnlyRegardOwnSeries() const
{
  return m_OnlyRegardOwnSeries;
}


std::vector<itk::SmartPointer<BaseData> > BaseDICOMReaderService::DoRead()
{
  std::vector<BaseData::Pointer> result;


  const std::string fileName = this->GetLocalFileName();
  //special handling of Philips 3D US DICOM.
  //Copied from DICOMSeriesReaderService
  if (DicomSeriesReader::IsPhilips3DDicom(fileName))
  {
      MITK_INFO << "it is a Philips3D US Dicom file" << std::endl;
      mitk::LocaleSwitch localeSwitch("C");
      std::locale previousCppLocale(std::cin.getloc());
      std::locale l("C");
      std::cin.imbue(l);

      DataNode::Pointer node = DataNode::New();
      mitk::DicomSeriesReader::StringContainer stringvec;
      stringvec.push_back(fileName);
      if (DicomSeriesReader::LoadDicomSeries(stringvec, *node))
      {
          BaseData::Pointer data = node->GetData();
          StringProperty::Pointer nameProp = StringProperty::New(itksys::SystemTools::GetFilenameName(fileName));
          data->GetPropertyList()->SetProperty("name", nameProp);
          result.push_back(data);
      }
      std::cin.imbue(previousCppLocale);
      return result;
  }

  //Normal DICOM handling (It wasn't a Philips 3D US)
  mitk::StringList relevantFiles = this->GetDICOMFilesInSameDirectory();

  if (relevantFiles.empty())
  {
      MITK_INFO << "DICOMReader service found no relevant files in specified location. No data is loaded. Location: "<<fileName;
  }
  else
  {
    bool pathIsDirectory = itksys::SystemTools::FileIsDirectory(fileName);

    if (!pathIsDirectory && m_OnlyRegardOwnSeries)
    {
      relevantFiles = mitk::FilterDICOMFilesForSameSeries(fileName, relevantFiles);
    }

    mitk::DICOMFileReader::Pointer reader = this->GetReader(relevantFiles);

      if(reader.IsNull())
      {
          MITK_INFO << "DICOMReader service found no suitable reader configuration for relevant files.";
      }
      else
      {
        if (!pathIsDirectory)
        { //we ensure that we only load the relevant image block files
          const auto nrOfOutputs = reader->GetNumberOfOutputs();
          for (unsigned int outputIndex = 0; outputIndex < nrOfOutputs; ++outputIndex)
          {
            const auto frameList = reader->GetOutput(outputIndex).GetImageFrameList();

            auto finding = std::find_if(frameList.begin(), frameList.end(), [&](const DICOMImageFrameInfo::Pointer& frame) { return frame->Filename == fileName; });

            if (finding != frameList.end())
            { //we have the block containing the fileName -> these are the realy relevant files.
              relevantFiles.resize(frameList.size());
              std::transform(frameList.begin(), frameList.end(), relevantFiles.begin(), [](const DICOMImageFrameInfo::Pointer& frame) { return frame->Filename; });
              break;
            }
          }
        }
          const unsigned int ntotalfiles = relevantFiles.size();

          for( unsigned int i=0; i< ntotalfiles; i++)
          {
            m_ReadFiles.push_back( relevantFiles.at(i) );
          }

          reader->SetAdditionalTagsOfInterest(mitk::GetCurrentDICOMTagsOfInterest());
          reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);
          reader->SetInputFiles(relevantFiles);

          mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
          scanner->AddTagPaths(reader->GetTagsOfInterest());
          scanner->SetInputFiles(relevantFiles);
          scanner->Scan();

          reader->SetTagCache(scanner->GetScanCache());
          reader->AnalyzeInputFiles();
          reader->LoadImages();

          for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
          {
            const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
            mitk::BaseData::Pointer data = desc.GetMitkImage().GetPointer();

            std::string nodeName = GenerateNameFromDICOMProperties(&desc);

            StringProperty::Pointer nameProp = StringProperty::New(nodeName);
            data->SetProperty("name", nameProp);

            data->SetProperty(PropertyKeyPathToPropertyName(DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION()), StringProperty::New(reader->GetConfigurationLabel()));

            result.push_back(data);
          }
      }
  }

  return result;
}

StringList BaseDICOMReaderService::GetDICOMFilesInSameDirectory() const
{
  std::string fileName = this->GetLocalFileName();

  mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(fileName);

  return relevantFiles;
}

IFileReader::ConfidenceLevel BaseDICOMReaderService::GetConfidenceLevel() const
{
  IFileReader::ConfidenceLevel abstractConfidence = AbstractFileReader::GetConfidenceLevel();

  if (Unsupported == abstractConfidence)
  {
    if (itksys::SystemTools::FileIsDirectory(this->GetInputLocation().c_str()))
    {
      // In principle we support dicom directories
      return Supported;
    }
  }

  return abstractConfidence;
}

std::string GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider)
{
  std::string nodeName = mitk::DataNode::NO_NAME_VALUE();

  auto studyProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x1030).c_str());
  if (studyProp.IsNotNull())
  {
    nodeName = studyProp->GetValueAsString();
  }

  auto seriesProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str());

  if (seriesProp.IsNotNull())
  {
    if (studyProp.IsNotNull())
    {
      nodeName += " / ";
    }
    else
    {
      nodeName = "";

    }
    nodeName += seriesProp->GetValueAsString();
  }

  return nodeName;
};

}
