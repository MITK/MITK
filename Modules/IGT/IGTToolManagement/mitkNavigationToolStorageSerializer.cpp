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
#include "Poco/Zip/Compress.h"

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkNavigationToolWriter.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkSceneIO.h>
#include <mitkStandardFileLocations.h>

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
  std::string DataStorageFileName = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "\\" + myToolWriter->GetFileWithoutPath(filename) + ".storage";
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mySceneIO->SaveScene(saveStorage->GetAll(),saveStorage,DataStorageFileName);

  //then put the storage and the toolfiles into a zip-archive
  std::ofstream file( filename.c_str(), std::ios::binary | std::ios::out);
  if (!file.good())
    {
    m_ErrorMessage = "Could not open a zip file for writing: '" + filename + "'";
    return false;
    }
  else
    {
    Poco::Zip::Compress zipper( file, true );
    zipper.addFile(DataStorageFileName,myToolWriter->GetFileWithoutPath(DataStorageFileName));
    for (int i=0; i<storage->GetToolCount();i++)
      {
      //check if filename already exits
      for (int j=0; j<i; j++) if (myToolWriter->GetFileWithoutPath(storage->GetTool(i)->GetCalibrationFile()) == myToolWriter->GetFileWithoutPath(storage->GetTool(j)->GetCalibrationFile())) break;
      zipper.addFile(storage->GetTool(i)->GetCalibrationFile(),myToolWriter->GetFileWithoutPath(storage->GetTool(i)->GetCalibrationFile()));
      }
    zipper.close();
    }

  return true;
  }