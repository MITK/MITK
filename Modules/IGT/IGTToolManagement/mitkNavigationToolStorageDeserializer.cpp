/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//Poco headers
#include "Poco/Zip/Decompress.h"

#include "mitkNavigationToolStorageDeserializer.h"
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>
#include "mitkNavigationToolReader.h"

mitk::NavigationToolStorageDeserializer::NavigationToolStorageDeserializer(mitk::DataStorage::Pointer dataStorage)
  {
  m_DataStorage = dataStorage;
  }

mitk::NavigationToolStorageDeserializer::~NavigationToolStorageDeserializer()
  {

  }

mitk::NavigationToolStorage::Pointer mitk::NavigationToolStorageDeserializer::Deserialize(std::string filename)
  {
  //decomress zip file into temporary directory
  std::ifstream file( filename.c_str(), std::ios::binary );
  if (!file.good())
    {
    m_ErrorMessage = "Cannot open '" + filename + "' for reading";
    return NULL;
    }

  std::string tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory();
  Poco::Zip::Decompress unzipper( file, Poco::Path( tempDirectory ) );
  unzipper.decompressAllFiles();


  //create DataTreeNodes using the decomressed storage
  mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New(m_DataStorage);
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer readStorage = mySceneIO->LoadScene(tempDirectory + "\\" + myReader->GetFileWithoutPath(filename) + ".storage");
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();

  for(int i=0; i<readStorage->GetAll()->Size(); i++)
    {
    mitk::NavigationTool::Pointer newTool = myReader->ConvertDataTreeNodeToNavigationTool(readStorage->GetAll()->ElementAt(i));
    if (!returnValue->AddTool(newTool))
      {
      m_ErrorMessage = "Error can't parse data storage!";
      return NULL;
      }
    }

  //delete decompressed storage which is not needed any more
  //TODO!

  return returnValue;
  }