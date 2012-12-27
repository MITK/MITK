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
#include "Poco/Zip/Decompress.h"
#include "Poco/Path.h"

//mitk headers
#include "mitkNavigationToolReader.h"
#include "mitkTrackingTypes.h"
#include <mitkStandardFileLocations.h>
#include <mitkSceneIO.h>



mitk::NavigationToolReader::NavigationToolReader()
  {

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

  std::string tempDirectory = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory() + Poco::Path::separator() + "toolFilesByNavigationToolReader" + Poco::Path::separator() + GetFileWithoutPath(filename);
  Poco::Zip::Decompress unzipper( file, Poco::Path( tempDirectory ) );
  unzipper.decompressAllFiles();

  //use SceneSerialization to load the DataStorage
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer loadedStorage = mySceneIO->LoadScene(tempDirectory + Poco::Path::separator() + GetFileWithoutPath(filename) + ".storage");

  if (loadedStorage->GetAll()->size()==0 || loadedStorage.IsNull())
    {
    m_ErrorMessage = "Invalid file: cannot parse tool data.";
    return NULL;
    }

  //convert the DataStorage back to a NavigationTool-Object
  mitk::DataNode::Pointer myNode = loadedStorage->GetAll()->ElementAt(0);
  mitk::NavigationTool::Pointer returnValue = ConvertDataNodeToNavigationTool(myNode, tempDirectory);

  //delete the data-storage file which is not needed any more. The toolfile must be left in the temporary directory becauses it is linked in the datatreenode of the tool
  std::remove((std::string(tempDirectory + Poco::Path::separator() + GetFileWithoutPath(filename) + ".storage")).c_str());

  return returnValue;
  }

mitk::NavigationTool::Pointer mitk::NavigationToolReader::ConvertDataNodeToNavigationTool(mitk::DataNode::Pointer node, std::string toolPath)
  {
  mitk::NavigationTool::Pointer returnValue = mitk::NavigationTool::New();

  //DateTreeNode with Name and Surface
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(node->GetName());
  newNode->SetData(node->GetData());
  returnValue->SetDataNode(newNode);

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
  if (calibration_filename=="none")
    {
    returnValue->SetCalibrationFile("none");
    }
  else
    {
    std::string calibration_filename_with_path = toolPath + Poco::Path::separator() + calibration_filename;
    returnValue->SetCalibrationFile(calibration_filename_with_path);
    }

  //Tool Landmarks
  mitk::PointSet::Pointer ToolRegLandmarks = mitk::PointSet::New();
  mitk::PointSet::Pointer ToolCalLandmarks = mitk::PointSet::New();
  std::string RegLandmarksString;
  std::string CalLandmarksString;
  node->GetStringProperty("ToolRegistrationLandmarks",RegLandmarksString);
  node->GetStringProperty("ToolCalibrationLandmarks",CalLandmarksString);
  ToolRegLandmarks = ConvertStringToPointSet(RegLandmarksString);
  ToolCalLandmarks = ConvertStringToPointSet(CalLandmarksString);
  returnValue->SetToolRegistrationLandmarks(ToolRegLandmarks);
  returnValue->SetToolCalibrationLandmarks(ToolCalLandmarks);

  //Tool Tip
  std::string toolTipPositionString;
  std::string toolTipOrientationString;
  node->GetStringProperty("ToolTipPosition",toolTipPositionString);
  node->GetStringProperty("ToolTipOrientation",toolTipOrientationString);
  returnValue->SetToolTipPosition(ConvertStringToPoint(toolTipPositionString));
  returnValue->SetToolTipOrientation(ConvertStringToQuaternion(toolTipOrientationString));

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

mitk::PointSet::Pointer mitk::NavigationToolReader::ConvertStringToPointSet(std::string string)
  {
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  std::string pointSeperator = "|";
  std::string valueSeperator = ";";
  std::vector<std::string> points;
  split(string,pointSeperator,points);
  for(int i=0; i<points.size(); i++)
    {
    std::vector<std::string> values;
    split(points.at(i),valueSeperator,values);
    if (values.size() == 4)
      {
      double index = atof(values.at(0).c_str());
      mitk::Point3D point;
      point[0] = atof(values.at(1).c_str());
      point[1] = atof(values.at(2).c_str());
      point[2] = atof(values.at(3).c_str());
      returnValue->SetPoint(index,point);
      }
    }
  return returnValue;
  }
mitk::Point3D mitk::NavigationToolReader::ConvertStringToPoint(std::string string)
{
std::string valueSeperator = ";";
std::vector<std::string> values;
split(string,valueSeperator,values);
mitk::Point3D point;
if (values.size() == 3)
    {
    point[0] = atof(values.at(0).c_str());
    point[1] = atof(values.at(1).c_str());
    point[2] = atof(values.at(2).c_str());
    }
return point;
}

mitk::Quaternion mitk::NavigationToolReader::ConvertStringToQuaternion(std::string string)
{
std::string valueSeperator = ";";
std::vector<std::string> values;
split(string,valueSeperator,values);
mitk::Quaternion quat = mitk::Quaternion(0,0,0,1);
if (values.size() == 4)
    {
    quat = mitk::Quaternion(atof(values.at(0).c_str()),
                            atof(values.at(1).c_str()),
                            atof(values.at(2).c_str()),
                            atof(values.at(3).c_str()));
    }
return quat;

}

void mitk::NavigationToolReader::split(std::string& text, std::string& separators, std::vector<std::string>& words)
  {
  int n = text.length();
  int start, stop;

  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
  {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop+1);
    }
  }
