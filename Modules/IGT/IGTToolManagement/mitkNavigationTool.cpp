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

#include "mitkNavigationTool.h"
#include "Poco/File.h"

mitk::NavigationTool::NavigationTool() : m_Type(mitk::NavigationTool::Unknown),
                                         m_Identifier("None"),
                                         m_TrackingDeviceType(mitk::TrackingSystemNotSpecified),
                                         m_CalibrationFile("none"),
                                         m_SerialNumber(""),
                                         m_ToolRegistrationLandmarks(mitk::PointSet::New()),
                                         m_ToolCalibrationLandmarks(mitk::PointSet::New()),
                                         m_ToolTipOrientation(mitk::Quaternion(0,0,0,1))
  {
    m_ToolTipPosition[0] = 0;
    m_ToolTipPosition[1] = 0;
    m_ToolTipPosition[2] = 0;
  }

mitk::NavigationTool::~NavigationTool()
  {

  }

void mitk::NavigationTool::SetCalibrationFile(const std::string filename)
  {
  //check if file does exist:
  if (filename=="")
    {
    m_CalibrationFile = "none";
    }
  else
    {
    Poco::File myFile(filename);
    if (myFile.exists())
      m_CalibrationFile = filename;
    else
      m_CalibrationFile = "none";
    }
  }

std::string mitk::NavigationTool::GetToolName()
  {
  if (this->m_DataNode.IsNull()) {return "";}
  else {return m_DataNode->GetName();}
  }

mitk::Surface::Pointer mitk::NavigationTool::GetToolSurface()
  {
  if (this->m_DataNode.IsNull()) {return NULL;}
  else if (this->m_DataNode->GetData() == NULL) {return NULL;}
  else {return dynamic_cast<mitk::Surface*>(m_DataNode->GetData());}
  }
