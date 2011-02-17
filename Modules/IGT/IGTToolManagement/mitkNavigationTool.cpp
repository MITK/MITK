/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkNavigationTool.h"
#include "Poco/File.h"

mitk::NavigationTool::NavigationTool()
  {
    m_Type = mitk::NavigationTool::Unknown;
    m_Identifier = "None";
    m_TrackingDeviceType = mitk::TrackingSystemNotSpecified;
    m_CalibrationFile = "none";
    m_SerialNumber = "";
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