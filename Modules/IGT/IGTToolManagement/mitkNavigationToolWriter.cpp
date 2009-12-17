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

#include "mitkNavigationToolWriter.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkProperties.h>
#include <mitkSceneIO.h>

mitk::NavigationToolWriter::NavigationToolWriter()
  {

  }

mitk::NavigationToolWriter::~NavigationToolWriter()
  {

  }

bool mitk::NavigationToolWriter::DoWrite(std::string FileName,mitk::NavigationTool::Pointer Tool)
  {
  //convert whole data to a mitk::DataStorage
  mitk::StandaloneDataStorage::Pointer saveStorage = mitk::StandaloneDataStorage::New();
  mitk::DataTreeNode::Pointer thisTool = ConvertToDataTreeNode(Tool);
  saveStorage->Add(thisTool);

  //use SceneSerialization to save the DataStorage
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mySceneIO->SaveScene(saveStorage->GetAll(),saveStorage,FileName);

  return true;
  }

std::string mitk::NavigationToolWriter::ReadFile(std::string filename)
  {
  std::string returnValue = "";
  std::ifstream in(filename.c_str());
  while(!in.eof()) returnValue += in.get();
  return returnValue;
  }

mitk::DataTreeNode::Pointer mitk::NavigationToolWriter::ConvertToDataTreeNode(mitk::NavigationTool::Pointer Tool)
  {
  mitk::DataTreeNode::Pointer thisTool = mitk::DataTreeNode::New();
  //Name
    thisTool->SetName(Tool->GetDataTreeNode()->GetName().c_str());
  //Identifier
    thisTool->AddProperty("identifier",mitk::StringProperty::New(Tool->GetIdentifier().c_str()));
  //Serial Number
    thisTool->AddProperty("serial number",mitk::StringProperty::New(Tool->GetSerialNumber().c_str()));
  //Tracking Device
    thisTool->AddProperty("tracking device type",mitk::IntProperty::New(Tool->GetTrackingDeviceType()));
  //Tool Type
    thisTool->AddProperty("tracking tool type",mitk::IntProperty::New(Tool->GetType()));
  //Calibration File Name
    thisTool->AddProperty("toolfileName",mitk::StringProperty::New(GetFileWithoutPath(Tool->GetCalibrationFile())));
  //Calibration File
    thisTool->AddProperty("toolfile",mitk::StringProperty::New(ReadFile(Tool->GetCalibrationFile()).c_str()));
  //Surface
    thisTool->SetData(Tool->GetDataTreeNode()->GetData());
  return thisTool;
  }

std::string mitk::NavigationToolWriter::GetFileWithoutPath(std::string FileWithPath)
  {
  std::string returnValue = "";
  returnValue = FileWithPath.substr(FileWithPath.rfind("/"), FileWithPath.length());
  return returnValue;
  }
