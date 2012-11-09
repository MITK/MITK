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

//Poco headers
#include "Poco/Zip/Compress.h"
#include "Poco/Path.h"

//mitk headers
#include "mitkNavigationToolWriter.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkProperties.h>
#include <mitkSceneIO.h>
#include <mitkPointSet.h>
#include <mitkStandardFileLocations.h>

//std headers
#include <stdio.h>

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
  mitk::DataNode::Pointer thisTool = ConvertToDataNode(Tool);
  saveStorage->Add(thisTool);

  //use SceneSerialization to save the DataStorage
  std::string DataStorageFileName = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + GetFileWithoutPath(FileName) + ".storage";
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mySceneIO->SaveScene(saveStorage->GetAll(),saveStorage,DataStorageFileName);

  //now put the DataStorage and the Toolfile in a ZIP-file
  std::ofstream file( FileName.c_str(), std::ios::binary | std::ios::out);
  if (!file.good())
    {
    m_ErrorMessage = "Could not open a zip file for writing: '" + FileName + "'";
    return false;
    }
  else
    {
    Poco::Zip::Compress zipper( file, true );
    zipper.addFile(DataStorageFileName,GetFileWithoutPath(DataStorageFileName));
    if (Tool->GetCalibrationFile()!="none") zipper.addFile(Tool->GetCalibrationFile(),GetFileWithoutPath(Tool->GetCalibrationFile()));
    zipper.close();
    }

  //delete the data storage
  std::remove(DataStorageFileName.c_str());

  return true;
  }

mitk::DataNode::Pointer mitk::NavigationToolWriter::ConvertToDataNode(mitk::NavigationTool::Pointer Tool)
  {
  mitk::DataNode::Pointer thisTool = mitk::DataNode::New();
  //Name
    if (Tool->GetDataNode().IsNull()) thisTool->SetName("none");
    else thisTool->SetName(Tool->GetDataNode()->GetName().c_str());
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
  //Surface
    if (Tool->GetDataNode().IsNotNull()) if (Tool->GetDataNode()->GetData()!=NULL) thisTool->SetData(Tool->GetDataNode()->GetData());

  //Tool Landmarks
    thisTool->AddProperty("ToolRegistrationLandmarks",mitk::StringProperty::New(ConvertPointSetToString(Tool->GetToolRegistrationLandmarks())));
    thisTool->AddProperty("ToolCalibrationLandmarks",mitk::StringProperty::New(ConvertPointSetToString(Tool->GetToolCalibrationLandmarks())));

  //Tool Tip
    thisTool->AddProperty("ToolTipPosition",mitk::StringProperty::New(ConvertPointToString(Tool->GetToolTipPosition())));
    thisTool->AddProperty("ToolTipOrientation",mitk::StringProperty::New(ConvertQuaternionToString(Tool->GetToolTipOrientation())));

  //Material is not needed, to avoid errors in scene serialization we have to do this:
    thisTool->ReplaceProperty("material",NULL);


  return thisTool;
  }

std::string mitk::NavigationToolWriter::GetFileWithoutPath(std::string FileWithPath)
  {
  std::string returnValue = "";
  returnValue = FileWithPath.substr(FileWithPath.rfind("/")+1, FileWithPath.length());
  //dirty hack: Windows path seperators
  if (returnValue.size() == FileWithPath.size()) returnValue = FileWithPath.substr(FileWithPath.rfind("\\")+1, FileWithPath.length());
  return returnValue;
  }

std::string mitk::NavigationToolWriter::ConvertPointSetToString(mitk::PointSet::Pointer pointSet)
  {
  std::stringstream returnValue;
  mitk::PointSet::PointDataIterator it;
  for ( it = pointSet->GetPointSet()->GetPointData()->Begin();it != pointSet->GetPointSet()->GetPointData()->End();it++ )
    {
    mitk::Point3D thisPoint = pointSet->GetPoint(it->Index());
    returnValue << it->Index() << ";" << ConvertPointToString(thisPoint) << "|";
    }
  return returnValue.str();
  }

std::string mitk::NavigationToolWriter::ConvertPointToString(mitk::Point3D point)
{
std::stringstream returnValue;
returnValue << point[0] << ";" << point[1] << ";" << point[2];
return returnValue.str();
}

std::string mitk::NavigationToolWriter::ConvertQuaternionToString(mitk::Quaternion quat)
{
std::stringstream returnValue;
returnValue << quat.x() << ";" << quat.y() << ";" << quat.z() << ";" << quat.r();
return returnValue.str();
}
