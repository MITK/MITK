/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "mitkBaseDataIOFactory.h"
#include "mitkBaseProcess.h"
#include "mitkIOAdapter.h"
#include "mitkConfig.h"

namespace mitk
{

std::vector<BaseData::Pointer> BaseDataIO::LoadBaseDataFromFile(const std::string path, const std::string filePrefix, const std::string filePattern, bool series)
{

  // factories are instantiated in mitk::CoreObjectFactory and other, potentially MITK external places

  std::vector<BaseData::Pointer> result;

  std::list<IOAdapterBase::Pointer> possibleIOAdapter;
  std::list<LightObject::Pointer> allobjects = itk::ObjectFactoryBase::CreateAllInstance("mitkIOAdapter");

  for( std::list<LightObject::Pointer>::iterator i = allobjects.begin();
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
      std::cerr << "Error BaseDataIO factory did not return an IOAdapterBase: "
        << (*i)->GetNameOfClass() 
        << std::endl;
    }
  }

  for( std::list<IOAdapterBase::Pointer>::iterator k = possibleIOAdapter.begin();
                                                   k != possibleIOAdapter.end(); 
                                                   ++k )
  { 
    bool canReadFile = false;
    
    if ( series )
      canReadFile = (*k)->CanReadFile(filePrefix, filePrefix, filePattern); // we have to provide a filename without extension here to 
    else                                                                    // prevent the "normal" (non-series) readers to report that
      canReadFile = (*k)->CanReadFile(path, filePrefix, filePattern);       // they could read the file
    
    if( canReadFile )
    {
      BaseProcess::Pointer ioObject = (*k)->CreateIOProcessObject(path, filePrefix, filePattern);
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

  return result;
}

} // end namespace itk
