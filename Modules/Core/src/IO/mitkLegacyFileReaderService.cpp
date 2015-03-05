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

#include "mitkLegacyFileReaderService.h"

#include <mitkDicomSeriesReader.h>
#include <mitkProgressBar.h>
#include <mitkIOAdapter.h>
#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

mitk::LegacyFileReaderService::LegacyFileReaderService(const mitk::LegacyFileReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::LegacyFileReaderService::LegacyFileReaderService(const std::vector<std::string>& extensions,
                                                       const std::string& category)
  : AbstractFileReader()
{
  this->SetMimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".legacy.");

  CustomMimeType customMimeType;
  customMimeType.SetCategory(category);

  for(std::vector<std::string>::const_iterator iter = extensions.begin(),
      endIter = extensions.end(); iter != endIter; ++iter)
  {
    std::string extension = *iter;
    if (!extension.empty() && extension[0] == '.')
    {
      extension.assign(extension.begin()+1, extension.end());
    }
    customMimeType.AddExtension(extension);
  }
  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  m_ServiceReg = this->RegisterService();
}

mitk::LegacyFileReaderService::~LegacyFileReaderService()
{
}

////////////////////// Reading /////////////////////////

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::LegacyFileReaderService::Read()
{
  std::vector<BaseData::Pointer> result;

  std::list<IOAdapterBase::Pointer> possibleIOAdapter;
  std::list<itk::LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkIOAdapter");

  for( std::list<itk::LightObject::Pointer>::iterator i = allobjects.begin();
       i != allobjects.end();
       ++i)
  {
    IOAdapterBase* io = dynamic_cast<IOAdapterBase*>(i->GetPointer());
    if(io)
    {
      possibleIOAdapter.push_back(io);
    }
    else
    {
      MITK_ERROR << "Error BaseDataIO factory did not return an IOAdapterBase: "
        << (*i)->GetNameOfClass()
        << std::endl;
    }
  }

  const std::string path = this->GetLocalFileName();
  for( std::list<IOAdapterBase::Pointer>::iterator k = possibleIOAdapter.begin();
                                                   k != possibleIOAdapter.end();
                                                   ++k )
  {
    bool canReadFile = (*k)->CanReadFile(path, "", "");       // they could read the file

    if( canReadFile )
    {
      BaseProcess::Pointer ioObject = (*k)->CreateIOProcessObject(path, "", "");
      ioObject->Update();
      int numberOfContents = static_cast<int>(ioObject->GetNumberOfOutputs());

      if (numberOfContents > 0)
      {
        BaseData::Pointer baseData;
        for(int i=0; i<numberOfContents; ++i)
        {
          baseData = dynamic_cast<BaseData*>(ioObject->GetOutputs()[i].GetPointer());
          if (baseData) // this is what's wanted, right?
          {
            result.push_back( baseData );
          }
        }
      }

      break;
    }
  }

  if (result.empty())
  {
    mitkThrow() << "Could not read file '" << path << "'";
  }

  return result;
}

mitk::LegacyFileReaderService* mitk::LegacyFileReaderService::Clone() const
{
  return new LegacyFileReaderService(*this);
}
