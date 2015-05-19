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
#include "mitkIGTException.h"
#include "mitkNavigationData.h"
#include "Poco/File.h"

mitk::NavigationTool::NavigationTool() : m_Identifier("None"),
                                         m_Type(mitk::NavigationTool::Unknown),
                                         m_CalibrationFile("none"),
                                         m_SerialNumber(""),
                                         m_TrackingDeviceType(mitk::TrackingSystemNotSpecified),
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

mitk::AffineTransform3D::Pointer mitk::NavigationTool::GetToolTipTransform()
 {
   mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();
   returnValue->SetPosition(this->m_ToolTipPosition);
   returnValue->SetOrientation(this->m_ToolTipOrientation);
   return returnValue->GetAffineTransform3D();
 }

void mitk::NavigationTool::Graft( const DataObject *data )
{
  // Attempt to cast data to an NavigationData
  const Self* nd;
  try
  {
    nd = dynamic_cast<const Self *>( data );
  }
  catch( ... )
  {
    mitkThrowException(mitk::IGTException) << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() ;
  }
  if (!nd)
  {
    // pointer could not be cast back down
    mitkThrowException(mitk::IGTException) << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() ;
  }
  // Now copy anything that is needed
  m_Identifier = nd->GetIdentifier();
  m_Type = nd->GetType();
  m_DataNode->SetName(nd->GetDataNode()->GetName());
  m_DataNode->SetData(nd->GetDataNode()->GetData());
  m_SpatialObject = nd->GetSpatialObject();
  m_TrackingTool = nd->GetTrackingTool();
  m_CalibrationFile = nd->GetCalibrationFile();
  m_SerialNumber = nd->GetSerialNumber();
  m_TrackingDeviceType = nd->GetTrackingDeviceType();
  m_ToolRegistrationLandmarks = nd->GetToolRegistrationLandmarks();
  m_ToolCalibrationLandmarks = nd->GetToolCalibrationLandmarks();
  m_ToolTipPosition = nd->GetToolTipPosition();
  m_ToolTipOrientation = nd->GetToolTipOrientation();

}

bool mitk::NavigationTool::IsToolTipSet()
  {
  if( (m_ToolTipPosition[0] == 0) &&
    (m_ToolTipPosition[1] == 0) &&
    (m_ToolTipPosition[2] == 0) &&
    (m_ToolTipOrientation.x() == 0) &&
    (m_ToolTipOrientation.y() == 0) &&
    (m_ToolTipOrientation.z() == 0) &&
    (m_ToolTipOrientation.r() == 1))
  return false;
  else return true;
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
  if (this->m_DataNode.IsNull()) {return nullptr;}
  else if (this->m_DataNode->GetData() == nullptr) {return nullptr;}
  else {return dynamic_cast<mitk::Surface*>(m_DataNode->GetData());}
  }
