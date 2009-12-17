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

#include "mitkNavigationToolStorageDeserializer.h"
#include <mitkSceneIO.h>
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
  mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New(m_DataStorage);
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer readStorage = mySceneIO->LoadScene(filename);
  if (readStorage.IsNull()) {m_ErrorMessage = "Error: invalid filename!"; return NULL;}
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

  return returnValue;
  }