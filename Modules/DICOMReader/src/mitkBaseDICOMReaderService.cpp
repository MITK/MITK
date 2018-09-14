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

#include <iostream>


#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

namespace mitk {

  BaseDICOMReaderService::BaseDICOMReaderService(const std::string& description)
    : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), description)
{
}

  BaseDICOMReaderService::BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description)
    : AbstractFileReader(customType, description)
  {
  }

std::vector<itk::SmartPointer<BaseData> > BaseDICOMReaderService::Read()
{
  std::vector<BaseData::Pointer> result;


  std::string fileName = this->GetLocalFileName();
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
  mitk::StringList relevantFiles = this->GetRelevantFiles();

  // check whether directory or file
  // if directory try to find first file within it instead
  // We only support this for a single directory at once
  if (relevantFiles.empty())
  {
    bool pathIsDirectory = itksys::SystemTools::FileIsDirectory(this->GetLocalFileName());
    if (pathIsDirectory)
    {
      itksys::Directory input;
      input.Load(this->GetLocalFileName().c_str());

      std::vector<std::string> files;
      for (unsigned long idx = 0; idx<input.GetNumberOfFiles(); idx++)
      {
        if (!itksys::SystemTools::FileIsDirectory(input.GetFile(idx)))
        {
          std::string fullpath = this->GetLocalFileName() + "/" + std::string(input.GetFile(idx));
          files.push_back(fullpath.c_str());
        }
      }
      relevantFiles = files;
    }
  }

  if (relevantFiles.empty())
  {
      MITK_INFO << "DICOMReader service found no relevant files in specified location. No data is loaded. Location: "<<fileName;
  }
  else
  {
      mitk::DICOMFileReader::Pointer reader = this->GetReader(relevantFiles);

      if(reader.IsNull())
      {
          MITK_INFO << "DICOMReader service found no suitable reader configuration for relevant files.";
      }
      else
      {
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

            result.push_back(data);
          }
      }
  }

  return result;
}

StringList BaseDICOMReaderService::GetRelevantFiles() const
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

  auto studyProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000D).c_str());
  if (studyProp.IsNotNull())
  {
    nodeName = studyProp->GetValueAsString();
  }

  auto seriesProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0020, 0x000E).c_str());

  if (seriesProp.IsNotNull())
  {
    if (studyProp.IsNotNull())
    {
      nodeName += " / ";
    }
    nodeName += seriesProp->GetValueAsString();
  }

  return nodeName;
};

}
