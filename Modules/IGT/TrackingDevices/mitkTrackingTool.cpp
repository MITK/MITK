/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingTool.h"

mitk::TrackingTool::TrackingTool()
: itk::Object(),
  m_ToolName(""),
  m_ErrorMessage(""),
  m_IGTTimeStamp(0),
  m_TrackingError(0.0f),
  m_Enabled(true),
  m_DataValid(false),
  m_ToolTipSet(false)
{
  m_Position[0] = 0.0f;
  m_Position[1] = 0.0f;
  m_Position[2] = 0.0f;
  m_Orientation[0] = 0.0f;
  m_Orientation[1] = 0.0f;
  m_Orientation[2] = 0.0f;
  m_Orientation[3] = 0.0f;
  // this should not be necessary as the tools bring their own tooltip transformation
  m_ToolTipPosition[0] = 0.0f;
  m_ToolTipPosition[1] = 0.0f;
  m_ToolTipPosition[2] = 0.0f;
  m_ToolAxisOrientation[0] = 0.0f;
  m_ToolAxisOrientation[1] = 0.0f;
  m_ToolAxisOrientation[2] = 0.0f;
  m_ToolAxisOrientation[3] = 1.0f;
}

mitk::TrackingTool::~TrackingTool()
{
}

void mitk::TrackingTool::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "ToolName: " << m_ToolName << std::endl;
  os << indent << "ErrorMesage: " << m_ErrorMessage << std::endl;
  os << indent << "Position: " << m_Position << std::endl;
  os << indent << "Orientation: " << m_Orientation << std::endl;
  os << indent << "TrackingError: " << m_TrackingError << std::endl;
  os << indent << "Enabled: " << m_Enabled << std::endl;
  os << indent << "DataValid: " << m_DataValid << std::endl;
  os << indent << "ToolTip: " << m_ToolTipPosition << std::endl;
  os << indent << "ToolTipRotation: " << m_ToolAxisOrientation << std::endl;
  os << indent << "ToolTipSet: " << m_ToolTipSet << std::endl;
}

const char* mitk::TrackingTool::GetToolName() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return this->m_ToolName.c_str();
}

void mitk::TrackingTool::SetToolName(const char* _arg)
{
  itkDebugMacro("setting m_ToolName to " << _arg);
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if ( _arg && (_arg == this->m_ToolName) )
  {
    return;
  }
  if (_arg)
  {
    this->m_ToolName= _arg;
  }
  else
  {
    this->m_ToolName= "";
  }
  this->Modified();
}


void mitk::TrackingTool::SetToolName( const std::string _arg )
{
  this->SetToolName(_arg.c_str());
}

mitk::Point3D mitk::TrackingTool::GetToolTipPosition() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_ToolTipPosition;
}

mitk::Quaternion mitk::TrackingTool::GetToolAxisOrientation() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_ToolAxisOrientation;
}

void mitk::TrackingTool::SetToolTipPosition(mitk::Point3D toolTipPosition,
                                    mitk::Quaternion orientation,
                                    mitk::ScalarType eps)
{
  if ( !Equal(m_ToolTipPosition, toolTipPosition, eps) ||
       !Equal(m_ToolAxisOrientation, orientation, eps) )
  {
    if( (toolTipPosition[0] == 0) &&
        (toolTipPosition[1] == 0) &&
        (toolTipPosition[2] == 0) &&
        (orientation.x() == 0) &&
        (orientation.y() == 0) &&
        (orientation.z() == 0) &&
        (orientation.r() == 1))
    {
      m_ToolTipSet = false;
    }
    else
    {
      m_ToolTipSet = true;
    }
    m_ToolTipPosition = toolTipPosition;
    m_ToolAxisOrientation = orientation;
    this->Modified();
  }
}

bool mitk::TrackingTool::IsToolTipSet() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_ToolTipSet;
}

void mitk::TrackingTool::GetPosition(mitk::Point3D& position) const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_ToolTipSet)
  {
    // Compute the position of tool tip in the coordinate frame of the
    // tracking device: Rotate the position of the tip into the tracking
    // device coordinate frame then add to the position of the tracking
    // sensor
    vnl_vector<mitk::ScalarType> pos_vnl = m_Position.GetVnlVector() + m_Orientation.rotate( m_ToolTipPosition.GetVnlVector() ) ;

    position[0] = pos_vnl[0];
    position[1] = pos_vnl[1];
    position[2] = pos_vnl[2];
  }
  else
  {
    position[0] = m_Position[0];
    position[1] = m_Position[1];
    position[2] = m_Position[2];
  }
  this->Modified();
}

void mitk::TrackingTool::SetPosition(mitk::Point3D position)
{
  itkDebugMacro("setting m_Position to " << position);

  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_Position != position)
  {
    m_Position = position;
    this->Modified();
  }
}

void mitk::TrackingTool::GetOrientation(mitk::Quaternion& orientation) const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_ToolTipSet)
  {
    // Compute the orientation of the tool tip in the coordinate frame of
    // the tracking device.
    //
    //   * m_Orientation is the orientation of the sensor relative to the transmitter
    //   * m_ToolAxisOrientation is the orientation of the tool tip relative to the sensor
    orientation = m_Orientation * m_ToolAxisOrientation;
  }
  else
  {
    orientation = m_Orientation;
  }
}

void mitk::TrackingTool::SetOrientation(mitk::Quaternion orientation)
{
  itkDebugMacro("setting m_Orientation to " << orientation);

  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_Orientation != orientation)
  {
    m_Orientation = orientation;
    this->Modified();
  }
}

bool mitk::TrackingTool::Enable()
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_Enabled == false)
  {
    this->m_Enabled = true;
    this->Modified();
  }
  return true;
}

bool mitk::TrackingTool::Disable()
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_Enabled == true)
  {
    this->m_Enabled = false;
    this->Modified();
  }
  return true;
}

bool mitk::TrackingTool::IsEnabled() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_Enabled;
}

void mitk::TrackingTool::SetDataValid(bool isDataValid)
{
  itkDebugMacro("setting m_DataValid to " << isDataValid);
  if (this->m_DataValid != isDataValid)
  {
    std::lock_guard<std::mutex> lock(m_MyMutex);
    this->m_DataValid = isDataValid;
    this->Modified();
  }
}

bool mitk::TrackingTool::IsDataValid() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_DataValid;
}

float mitk::TrackingTool::GetTrackingError() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return m_TrackingError;
}

void mitk::TrackingTool::SetTrackingError(float error)
{
  itkDebugMacro("setting m_TrackingError to " << error);
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if (m_TrackingError != error)
  {
    m_TrackingError = error;
    this->Modified();
  }
}

const char* mitk::TrackingTool::GetErrorMessage() const
{
  std::lock_guard<std::mutex> lock(m_MyMutex);
  return this->m_ErrorMessage.c_str();
}

void mitk::TrackingTool::SetErrorMessage(const char* _arg)
{
  itkDebugMacro("setting m_ErrorMessage to " << _arg);
  std::lock_guard<std::mutex> lock(m_MyMutex);
  if ((_arg == nullptr) || (_arg == this->m_ErrorMessage))
    return;

  if (_arg != nullptr)
    this->m_ErrorMessage = _arg;
  else
    this->m_ErrorMessage = "";
  this->Modified();
}
