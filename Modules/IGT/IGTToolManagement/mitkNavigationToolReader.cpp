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

//mitk headers
#include "mitkNavigationToolReader.h"
#include "mitkTrackingTypes.h"
#include <mitkStandardFileLocations.h>
#include <mitkSceneIO.h>



mitk::NavigationToolReader::NavigationToolReader(mitk::DataStorage::Pointer dataStorage)
  {
  m_DataStorage = dataStorage;
  }

mitk::NavigationToolReader::~NavigationToolReader()
  {

  }

mitk::NavigationTool::Pointer mitk::NavigationToolReader::DoRead(std::string filename)
  {
  //decompress all files into a temporary directory
  std::ifstream file( filename.c_str(), std::ios::binary );
  if (!file.good())
    {
    m_ErrorMessage = "Cannot open '" + filename + "' for reading";
    return NULL;
    }

  std::string tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory();
  Poco::Zip::Decompress unzipper( file, Poco::Path( tempDirectory ) );
  unzipper.decompressAllFiles();
  
  //use SceneSerialization to load the DataStorage
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer loadedStorage = mySceneIO->LoadScene(tempDirectory + "\\" + GetFileWithoutPath(filename) + ".storage");

  if (loadedStorage->GetAll()->size()==0 || loadedStorage.IsNull())
    {
    m_ErrorMessage = "Invalid file: cannot parse tool data.";
    return NULL;
    }
  
  //convert the DataStorage back to a NavigationTool-Object
  mitk::DataTreeNode::Pointer myNode = loadedStorage->GetAll()->ElementAt(0);
  mitk::NavigationTool::Pointer returnValue = ConvertDataTreeNodeToNavigationTool(myNode);
  
  //delete the data-storage file which is not needed any more. The toolfile must be left in the temporary directory becauses it is linked in the datatreenode of the tool
  //TODO!

  return returnValue;
  }

mitk::NavigationTool::Pointer mitk::NavigationToolReader::ConvertDataTreeNodeToNavigationTool(mitk::DataTreeNode::Pointer node)
  {
  mitk::NavigationTool::Pointer returnValue = mitk::NavigationTool::New();
  
  //DateTreeNode with Name and Surface
  mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
  newNode->SetName(node->GetName());
  newNode->SetData(node->GetData());
  m_DataStorage->Add(newNode);
  returnValue->SetDataTreeNode(newNode);

  //Identifier
  std::string identifier;
  node->GetStringProperty("identifier",identifier);
  returnValue->SetIdentifier(identifier);
  
  //Serial Number
  std::string serial;
  node->GetStringProperty("serial number",serial);
  returnValue->SetSerialNumber(serial);

  //Tracking Device
  int device_type;
  node->GetIntProperty("tracking device type",device_type);
  returnValue->SetTrackingDeviceType(static_cast<mitk::TrackingDeviceType>(device_type));

  //Tool Type
  int type;
  node->GetIntProperty("tracking tool type",type);
  returnValue->SetType(static_cast<mitk::NavigationTool::NavigationToolType>(type));

  //Calibration File Name
  std::string calibration_filename;
  node->GetStringProperty("toolfileName",calibration_filename);
  std::string calibration_filename_with_path = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + "\\" + calibration_filename;
  returnValue->SetCalibrationFile(calibration_filename_with_path);
  
  return returnValue;
  }

std::string mitk::NavigationToolReader::GetFileWithoutPath(std::string FileWithPath)
  {
  std::string returnValue = "";
  returnValue = FileWithPath.substr(FileWithPath.rfind("/")+1, FileWithPath.length());
  //dirty hack: Windows path seperators
  if (returnValue.size() == FileWithPath.size()) returnValue = FileWithPath.substr(FileWithPath.rfind("\\")+1, FileWithPath.length());
  return returnValue;
  }