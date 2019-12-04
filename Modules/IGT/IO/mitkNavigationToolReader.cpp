/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//Poco headers
#include <Poco/Zip/Decompress.h>
#include <Poco/Path.h>

//mitk headers
#include "mitkNavigationToolReader.h"
#include "mitkTrackingTypes.h"
#include <mitkIOUtil.h>
#include <mitkSceneIO.h>

//All Tracking devices, which should be available by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

mitk::NavigationToolReader::NavigationToolReader()
{
  m_ToolfilePath = mitk::IOUtil::GetTempPath() + Poco::Path::separator() + "IGT_Toolfiles" + Poco::Path::separator();
}

mitk::NavigationToolReader::~NavigationToolReader()
{
}

mitk::NavigationTool::Pointer mitk::NavigationToolReader::DoRead(std::string filename)
{
  //decompress all files into a temporary directory
  std::ifstream file(filename.c_str(), std::ios::binary);
  if (!file.good())
  {
    m_ErrorMessage = "Cannot open '" + filename + "' for reading";
    return nullptr;
  }

  std::string tempDirectory = m_ToolfilePath + GetFileWithoutPath(filename);
  Poco::Zip::Decompress unzipper(file, Poco::Path(tempDirectory));
  unzipper.decompressAllFiles();

  //use SceneSerialization to load the DataStorage
  mitk::SceneIO::Pointer mySceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer loadedStorage = mySceneIO->LoadScene(tempDirectory + Poco::Path::separator() + GetFileWithoutPath(filename) + ".storage");

  if (loadedStorage->GetAll()->size() == 0 || loadedStorage.IsNull())
  {
    m_ErrorMessage = "Invalid file: cannot parse tool data.";
    return nullptr;
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
  returnValue->SetDataNode(node);

  //Identifier
  std::string identifier;
  node->GetStringProperty("identifier", identifier);
  returnValue->SetIdentifier(identifier);

  node->RemoveProperty("identifier");

  //Serial Number
  std::string serial;
  node->GetStringProperty("serial number", serial);
  returnValue->SetSerialNumber(serial);

  node->RemoveProperty("serial number");

  //Tracking Device
  mitk::TrackingDeviceType device_type;
  node->GetStringProperty("tracking device type", device_type);

  //For backward compability with old tool stroages (before 12/2015 device_type was an int value, now it is string)
  if (device_type.size() == 0)
  {
    /*
    This was the old enum. Numbers inserted for better readibility. Don't delete this if-case to allow loading of ols storages...
    enum TrackingDeviceType
    {
    0 NDIPolaris,                 ///< Polaris: optical Tracker from NDI
    1 NDIAurora,                  ///< Aurora: electromagnetic Tracker from NDI
    2 ClaronMicron,               ///< Micron Tracker: optical Tracker from Claron
    3 IntuitiveDaVinci,           ///< Intuitive Surgical: DaVinci Telemanipulator API Interface
    4 AscensionMicroBird,         ///< Ascension microBird / PCIBird family
    5 VirtualTracker,             ///< Virtual Tracking device class that produces random tracking coordinates
    6 TrackingSystemNotSpecified, ///< entry for not specified or initialized tracking system
    7 TrackingSystemInvalid,      ///< entry for invalid state (mainly for testing)
    8 NPOptitrack,                          ///< NaturalPoint: Optitrack optical Tracking System
    9 OpenIGTLinkTrackingDeviceConnection   ///< Device which is connected via open igt link
    };
    */
    int device_type_old;
    node->GetIntProperty("tracking device type", device_type_old);
    switch (device_type_old)
    {
    case 0:device_type = mitk::NDIPolarisTypeInformation::GetTrackingDeviceName(); break;
    case 1:device_type = mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(); break;
    case 2:device_type = mitk::MicronTrackerTypeInformation::GetTrackingDeviceName(); break;
    case 3:device_type = "IntuitiveDaVinci"; break;
    case 4:device_type = "AscensionMicroBird"; break;
    case 5:device_type = mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(); break;
    case 6:device_type = mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(); break;
    case 7:device_type = "TrackingSystemInvalid"; break;
    case 8:device_type = mitk::NPOptitrackTrackingTypeInformation::GetTrackingDeviceName(); break;
    case 9:device_type = mitk::OpenIGTLinkTypeInformation::GetTrackingDeviceName(); break;
    default: device_type = mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName(); break; //default... unknown...
    }
  }

  node->RemoveProperty("tracking device type");

  returnValue->SetTrackingDeviceType(static_cast<mitk::TrackingDeviceType>(device_type));

  //Tool Type
  int type;
  node->GetIntProperty("tracking tool type", type);
  returnValue->SetType(static_cast<mitk::NavigationTool::NavigationToolType>(type));

  node->RemoveProperty("tracking tool type");

  //Calibration File Name
  std::string calibration_filename;
  node->GetStringProperty("toolfileName", calibration_filename);
  if (calibration_filename == "none")
  {
    returnValue->SetCalibrationFile("none");
  }
  else
  {
    std::string calibration_filename_with_path = toolPath + Poco::Path::separator() + calibration_filename;
    returnValue->SetCalibrationFile(calibration_filename_with_path);
  }

  node->RemoveProperty("toolfileName");

  //Tool Landmarks
  mitk::PointSet::Pointer ToolRegLandmarks = mitk::PointSet::New();
  mitk::PointSet::Pointer ToolCalLandmarks = mitk::PointSet::New();
  std::string RegLandmarksString;
  std::string CalLandmarksString;
  node->GetStringProperty("ToolRegistrationLandmarks", RegLandmarksString);
  node->GetStringProperty("ToolCalibrationLandmarks", CalLandmarksString);
  ToolRegLandmarks = ConvertStringToPointSet(RegLandmarksString);
  ToolCalLandmarks = ConvertStringToPointSet(CalLandmarksString);
  returnValue->SetToolLandmarks(ToolRegLandmarks);
  returnValue->SetToolControlPoints(ToolCalLandmarks);

  node->RemoveProperty("ToolRegistrationLandmarks");
  node->RemoveProperty("ToolCalibrationLandmarks");

  //Tool Tip
  std::string toolTipPositionString;
  std::string toolTipOrientationString;
  bool positionSet = node->GetStringProperty("ToolTipPosition", toolTipPositionString);
  bool orientationSet = node->GetStringProperty("ToolAxisOrientation", toolTipOrientationString);

  if (positionSet && orientationSet) //only define tooltip if it is set
  {
    returnValue->SetToolTipPosition(ConvertStringToPoint(toolTipPositionString));
    returnValue->SetToolAxisOrientation(ConvertStringToQuaternion(toolTipOrientationString));
  }
  else if (positionSet != orientationSet)
  {
    MITK_WARN << "Tooltip definition incomplete: position and orientation have to be set! Skipping tooltip definition.";
  }

  node->RemoveProperty("ToolTipPosition");
  node->RemoveProperty("ToolAxisOrientation");

  return returnValue;
}

std::string mitk::NavigationToolReader::GetFileWithoutPath(std::string FileWithPath)
{
  Poco::Path myFile(FileWithPath.c_str());
  return myFile.getFileName();
}

mitk::PointSet::Pointer mitk::NavigationToolReader::ConvertStringToPointSet(std::string string)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  std::string pointSeperator = "|";
  std::string valueSeperator = ";";
  std::vector<std::string> points;
  split(string, pointSeperator, points);
  for (unsigned int i = 0; i < points.size(); i++)
  {
    std::vector<std::string> values;
    split(points.at(i), valueSeperator, values);
    if (values.size() == 4)
    {
      double index = atof(values.at(0).c_str());
      mitk::Point3D point;
      point[0] = atof(values.at(1).c_str());
      point[1] = atof(values.at(2).c_str());
      point[2] = atof(values.at(3).c_str());
      returnValue->SetPoint(index, point);
    }
  }
  return returnValue;
}
mitk::Point3D mitk::NavigationToolReader::ConvertStringToPoint(std::string string)
{
  std::string valueSeperator = ";";
  std::vector<std::string> values;
  split(string, valueSeperator, values);
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
  split(string, valueSeperator, values);
  mitk::Quaternion quat = mitk::Quaternion(0, 0, 0, 1);
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
    start = text.find_first_not_of(separators, stop + 1);
  }
}
