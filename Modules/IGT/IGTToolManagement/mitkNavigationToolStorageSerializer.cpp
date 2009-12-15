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

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkNavigationToolWriter.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>

mitk::NavigationToolStorageSerializer::NavigationToolStorageSerializer()
  {

  }

mitk::NavigationToolStorageSerializer::~NavigationToolStorageSerializer()
  {

  }

bool mitk::NavigationToolStorageSerializer::Serialize(std::string filename, mitk::NavigationToolStorage::Pointer storage)
  {
  //convert whole data to a mitk::DataStorage
  mitk::StandaloneDataStorage::Pointer saveStorage = mitk::StandaloneDataStorage::New();
  mitk::NavigationToolWriter::Pointer myToolWriter = mitk::NavigationToolWriter::New();

  for(int i=0; i<storage->GetToolCount();i++)
    {
    mitk::DataTreeNode::Pointer thisTool = myToolWriter->ConvertToDataTreeNode(storage->GetTool(i));
    saveStorage->Add(thisTool);
    }

  //use SceneSerialization to save the DataStorage
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mySceneIO->SaveScene(saveStorage->GetAll(),saveStorage,filename);

  return true;
  }