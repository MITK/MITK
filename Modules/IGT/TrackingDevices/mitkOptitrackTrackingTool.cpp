/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOptitrackTrackingTool.h"

#ifdef MITK_USE_OPTITRACK_TRACKER

/**
* \brief API library header for Optitrack Systems
*/
#include <NPTrackingTools.h>

//=======================================================
// Constructor
//=======================================================
mitk::OptitrackTrackingTool::OptitrackTrackingTool()
  :  mitk::TrackingTool(),
    m_ID(-1)
{
  MITK_DEBUG << "Creating OptitrackTrackingTool Object";
  this->m_FLE = 0.0;
}

//=======================================================
// Destructor
//=======================================================
mitk::OptitrackTrackingTool::~OptitrackTrackingTool()
{
  delete this->m_calibrationPoints;
  delete m_pivotPoint;
  MITK_DEBUG << "Deleting OptitrackTrackingTool Object";
}

//=======================================================
// SetToolByFileName
//=======================================================
bool mitk::OptitrackTrackingTool::SetToolByFileName(std::string nameFile)
{
  MITK_DEBUG << "SetToolByFileName";
  MITK_INFO<<"Name of the file for configuration:  "<<nameFile;
  this->m_fileConfiguration = nameFile;
  int resultFscan, resultUpdate, resultCreateTrackable, resultTrackableTranslatePivot;

  // Check the file path
  if(this->m_fileConfiguration.empty())
  {
    MITK_INFO << "Calibration File for Tool is empty";
    mitkThrowException(mitk::IGTException) << "Calibration File for Tool is empty";
    return false;
  }

  // Open the file
  FILE* calib_file = fopen(this->m_fileConfiguration.c_str(),"r");
  if (calib_file == nullptr)
  {
    MITK_INFO << "Error using opening file";
    mitkThrowException(mitk::IGTException) << "Cannot open configuration file";
    return false;
  }

  MITK_DEBUG<<"Reading configuration file...";

  // Get the name
  this->m_ToolName = "";
  char* aux = new char[200];
  resultFscan = fscanf(calib_file,"%s\n",aux);
  this->m_ToolName.append(aux);
  delete aux;
  if ((resultFscan < 1) || this->m_ToolName.empty())
  {
    MITK_INFO << "No name found in the tool configuration file";
    mitkThrowException(mitk::IGTException) << "No name found in the tool configuration file";
    return false;
  }

  MITK_INFO<<"ToolName: " << this->m_ToolName;

  // Get the number of of points
  resultFscan = fscanf(calib_file,"%i\n",&(this->m_numMarkers));
  if (this->m_numMarkers < 3)
  {
    MITK_INFO << "The minimum number for define a tool is 3 markers";
    mitkThrowException(mitk::IGTException) << "Tool has less than 3 markers";
    return false;
  }

  MITK_INFO<<"\tNumer of Markers: " << this->m_numMarkers;

  // Read the Calibration Point locations and save them
  this->m_calibrationPoints = new float[3*this->m_numMarkers];

  for(int i=0; i<this->m_numMarkers; i++)
  {
     resultFscan = fscanf(calib_file,"%fe ",  &this->m_calibrationPoints[i*3+0]);
     if (resultFscan < 1)
     {
       MITK_INFO << "Cannot read X location for marker " << i;
       mitkThrowException(mitk::IGTException) << "Cannot read X location for marker " << i;
       return false;
     }

     resultFscan = fscanf(calib_file,"%fe ",  &this->m_calibrationPoints[i*3+1]);
     if (resultFscan < 1)
     {
       MITK_INFO << "Cannot read Y location for marker " << i;
       mitkThrowException(mitk::IGTException) << "Cannot read Y location for marker " << i;
       return false;
     }

     resultFscan = fscanf(calib_file,"%fe\n",  &this->m_calibrationPoints[i*3+2]);
     if (resultFscan < 1)
     {
       MITK_INFO << "Cannot read Z location for marker " << i;
       mitkThrowException(mitk::IGTException) << "Cannot read Z location for marker " << i;
       return false;
     }
     MITK_DEBUG << "\t\tMarker " << i;
     MITK_DEBUG << "\t\t X: " << this->m_calibrationPoints[i*3+0] << " Y: " << this->m_calibrationPoints[i*3+1] << " Z: " << this->m_calibrationPoints[i*3+2];

     this->m_calibrationPoints[i*3+0] = this->m_calibrationPoints[i*3+0]/1000;
     this->m_calibrationPoints[i*3+1] = this->m_calibrationPoints[i*3+1]/1000;
     this->m_calibrationPoints[i*3+2] = -this->m_calibrationPoints[i*3+2]/1000;// Optitrack works with Left Handed System

  }

  // Read the Pivot Point location
  this->m_pivotPoint = new float[3];
  resultFscan = fscanf(calib_file,"%fe ",  &this->m_pivotPoint[0]);
  if (resultFscan < 1)
  {
    MITK_INFO << "Cannot read X location for Pivot Point ";
    mitkThrowException(mitk::IGTException) << "Cannot read X location for Pivot Point ";
    return false;
  }

  resultFscan = fscanf(calib_file,"%fe ",  &this->m_pivotPoint[1]);
  if (resultFscan < 1)
  {
    MITK_INFO << "Cannot read Y location for Pivot Point " ;
    mitkThrowException(mitk::IGTException) << "Cannot read Y location for Pivot Point ";
    return false;
  }

  resultFscan = fscanf(calib_file,"%fe\n",  &this->m_pivotPoint[2]);
  if (resultFscan < 1)
  {
    MITK_INFO << "Cannot read Z location for Pivot Point " ;
    mitkThrowException(mitk::IGTException) << "Cannot read Z location for Pivot Point ";
    return false;
   }

  MITK_INFO << "\tPivotPoint " ;
  MITK_INFO << "\t\t X: " << this->m_pivotPoint[0] << " Y: " << this->m_pivotPoint[1] << " Z: " << this->m_pivotPoint[2];

  // mm -> m
  this->m_pivotPoint[0] = this->m_pivotPoint[0]/1000;
  this->m_pivotPoint[1] = this->m_pivotPoint[1]/1000;
  this->m_pivotPoint[2] = -this->m_pivotPoint[2]/1000;

  // get the ID for next tool in Optitrack System
  this->m_ID = this->get_IDnext();

  // Create the Tool
  for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
  {
    resultCreateTrackable = TT_CreateTrackable(m_ToolName.c_str(), this->m_ID,this->m_numMarkers,this->m_calibrationPoints);
    if(NPRESULT_SUCCESS == resultCreateTrackable)
    {
      MITK_INFO << "Trackable Created Successfully";
      i = -1;
    }
    else
    {
      MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultCreateTrackable);
      MITK_DEBUG << "Trying again...";
    }
  }

  for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
  {
    resultUpdate = TT_Update();
    if(NPRESULT_SUCCESS == resultUpdate)
    {
      resultTrackableTranslatePivot = TT_TrackableTranslatePivot(this->m_ID,this->m_pivotPoint[0],this->m_pivotPoint[1],this->m_pivotPoint[2]);
      if(NPRESULT_SUCCESS == resultCreateTrackable)
      {
        MITK_INFO << "Pivot Translation Successfull";
        fclose(calib_file);
        i=-1;
        return true;
      }
      else
      {
        MITK_INFO << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultUpdate);
        MITK_DEBUG << "Trying again...";
      }
    }
    else
    {
      MITK_INFO << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultUpdate);
      MITK_DEBUG << "Trying again...";
    }
  }

  MITK_INFO << "Cannot create tool ";
  mitkThrowException(mitk::IGTException) << "Cannot create tool ";
  return false;
}

//=======================================================
// get_IDnext
//=======================================================
int mitk::OptitrackTrackingTool::get_IDnext()
{
  MITK_DEBUG << "get_ID";
  int num_trackables = -1;
  int resultUpdate;

  for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
  {
    resultUpdate = TT_Update();
    if(NPRESULT_SUCCESS == resultUpdate)
    {
      num_trackables =  TT_TrackableCount();
      MITK_DEBUG << " Next ID: " << num_trackables;
      if(num_trackables > -1)
      {
        return num_trackables;
      }
      else
      {
        MITK_DEBUG << "get_IDnext failed";
        mitkThrowException(mitk::IGTException) << "get_IDnext failed";
      }

    }
    else
    {
      MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultUpdate);
      MITK_DEBUG << "Trying again...";
    }
  }

  mitkThrowException(mitk::IGTException) << "get_IDnext failed";
  return num_trackables;
}

//=======================================================
// DeleteTrackable
//=======================================================
bool mitk::OptitrackTrackingTool::DeleteTrackable()
{
  MITK_DEBUG << "DeleteTrackable";
  int resultRemoveTrackable;
  resultRemoveTrackable = TT_RemoveTrackable(this->m_ID);

  if(resultRemoveTrackable != NPRESULT_SUCCESS)
  {
    MITK_INFO << "Cannot Remove Trackable";
    MITK_INFO << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultRemoveTrackable);
    mitkThrowException(mitk::IGTException) << "Cannot Remove Trackable" << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultRemoveTrackable);
    return false;
  }
  else
  {
    MITK_INFO<<"Trackable " << this->m_ToolName  << " removed";
  }

  return true;
}

//=======================================================
// SetPosition
//=======================================================
void mitk::OptitrackTrackingTool::SetPosition(mitk::Point3D position, ScalarType eps)
{
  MITK_DEBUG << "SetPosition";
  // sets the position
  this->m_Position[0] = position[0];
  this->m_Position[1] = position[1];
  this->m_Position[2] = position[2];
}

//=======================================================
// SetOrientation
//=======================================================
void mitk::OptitrackTrackingTool::SetOrientation(mitk::Quaternion orientation, ScalarType eps)
{
  MITK_DEBUG << "SetOrientation";
  // sets the orientation as a quaternion
  this->m_Orientation.x() = orientation.x();
  this->m_Orientation.y() = orientation.y();
  this->m_Orientation.z() = orientation.z();
  this->m_Orientation.r() = orientation.r();
}

//=======================================================
// GetPosition
//=======================================================
void mitk::OptitrackTrackingTool::GetPosition(mitk::Point3D& positionOutput) const
{
  MITK_DEBUG << "GetPosition";
  // returns the current position of the tool as an array of three floats (in the tracking device coordinate system)
  positionOutput[0] = this->m_Position[0];
  positionOutput[1] = this->m_Position[1];
  positionOutput[2] = this->m_Position[2];
}

//=======================================================
// GetOrientation
//=======================================================
void mitk::OptitrackTrackingTool::GetOrientation(mitk::Quaternion& orientation) const
{
  MITK_DEBUG << "GetOrientation";
  // returns the current orientation of the tool as a quaternion (in the tracking device coordinate system)
  orientation.x() = this->m_Orientation.x();
  orientation.y() = this->m_Orientation.y();
  orientation.z() = this->m_Orientation.z();
  orientation.r() = this->m_Orientation.r();
}

//=======================================================
// Enable
//=======================================================
bool mitk::OptitrackTrackingTool::Enable()
{
  MITK_DEBUG << "Enable";
  // enable the tool, so that it will be tracked. Returns true if enabling was successfull
  TT_SetTrackableEnabled(this->m_ID, true);

  if(TT_TrackableEnabled(this->m_ID) == true)
  {
    this->m_Enabled = true;
    return true;
  }
  else
  {
    this->m_Enabled = false;
    MITK_INFO << "Enable failed";
    mitkThrowException(mitk::IGTException) << "Enable failed";
    return false;
  }
}

//=======================================================
// Disable
//=======================================================
bool mitk::OptitrackTrackingTool::Disable()
{
  MITK_DEBUG << "Disable";
  // disables the tool, so that it will not be tracked anymore. Returns true if disabling was successfull
  TT_SetTrackableEnabled(this->m_ID, false);

  if(TT_TrackableEnabled(this->m_ID) == true)
  {
    this->m_Enabled = false;
    return true;
  }
  else
  {
    this->m_Enabled = true;
    MITK_INFO << "Disable failed";
    mitkThrowException(mitk::IGTException) << "Disable failed";
    return false;
  }

}

//=======================================================
// IsEnabled
//=======================================================
bool mitk::OptitrackTrackingTool::IsEnabled() const
{
  MITK_DEBUG << "IsEnabled";
  // returns whether the tool is enabled or disabled
  return TT_TrackableEnabled(this->m_ID);
}

//=======================================================
// IsDataValid
//=======================================================
bool mitk::OptitrackTrackingTool::IsDataValid() const
{
  MITK_DEBUG << "IsDataValid";
  // returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
  return this->m_DataValid;
}

//=======================================================
// GetTrackingError
//=======================================================
float mitk::OptitrackTrackingTool::GetTrackingError() const
{
  MITK_DEBUG << "GetTrackingError";
  // return one value that corresponds to the overall tracking error. The dimension of this value is specific to each tracking device
  return this->m_TrackingError;
}

//=======================================================
// SetTrackingError
//=======================================================
void mitk::OptitrackTrackingTool::SetTrackingError(float error)
{
  MITK_DEBUG << "GetTrackingError";
  //< sets the tracking error
  //this->m_FLE = error;
  //this->UpdateError;
  this->m_TrackingError = error;
}

//=======================================================
// SetDataValid
//=======================================================
void mitk::OptitrackTrackingTool::SetDataValid(bool validate)
{
  MITK_DEBUG << "SetDataValid";
  // sets if the tracking data (position & Orientation) is valid
  this->m_DataValid = validate;
}

//=======================================================
// updateTool
//=======================================================
void mitk::OptitrackTrackingTool::updateTool()
{
  MITK_DEBUG << "updateTool";
  float yaw,pitch,roll;
  float data[7];

  if(TT_Update() == NPRESULT_SUCCESS)
  {
    if(this->IsEnabled())
    {
      TT_TrackableLocation(this->m_ID,  &data[0],  &data[1],  &data[2],             // Position
                                        &data[3],  &data[4],  &data[5],  &data[6],  // Orientation
                                        &yaw,    &pitch,    &roll);                 // Orientation

      //for( int i=0; i<7; i++)
      //{
      //  if(boost::math::isinf<float>(data[i])) // Possible Tracking check for INF numbers
      //  {
      //    this->SetDataValid(false);
      //    MITK_DEBUG << "Data set to INF by the system";
      //    return;
      //  }
      //}

    // m -> mm
      this->m_Position[0] = data[0]*1000;
      this->m_Position[1] = data[1]*1000;
      this->m_Position[2] = -data[2]*1000; // Correction from LeftHanded to RightHanded system

      this->m_Orientation.x() = data[3];
      this->m_Orientation.y() = data[4];
      this->m_Orientation.z() = -data[5];
      this->m_Orientation.r() = data[6];

      this->SetDataValid(true);

      MITK_DEBUG << this->m_Position[0] << "   " << this->m_Position[1] << "   " << this->m_Position[2];
      MITK_DEBUG << data[3] << "   " << data[4] << "   " << data[5] << "   " << data[6];

    }
    else
    {
      this->SetDataValid(false);
      MITK_DEBUG << "Trackable: "<< this->m_ToolName << "is not Tracked";
    }
  }
  else
  {
    this->SetDataValid(false);
    MITK_DEBUG << "Update Failed";
  }
}

//=======================================================
// IF Optitrack is not installed set functions to warnings
//=======================================================
#else

//=======================================================
// Constructor
//=======================================================
mitk::OptitrackTrackingTool::OptitrackTrackingTool()
  :  mitk::TrackingTool(),
    m_ID(-1)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// Destructor
//=======================================================
mitk::OptitrackTrackingTool::~OptitrackTrackingTool()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// SetToolByFileName
//=======================================================
bool mitk::OptitrackTrackingTool::SetToolByFileName(std::string)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// get_IDnext
//=======================================================
int mitk::OptitrackTrackingTool::get_IDnext()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return -1;
}

//=======================================================
// DeleteTrackable
//=======================================================
bool mitk::OptitrackTrackingTool::DeleteTrackable()
{
 MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
 return false;
}

//=======================================================
// SetPosition
//=======================================================
void mitk::OptitrackTrackingTool::SetPosition(mitk::Point3D, ScalarType)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// SetOrientation
//=======================================================
void mitk::OptitrackTrackingTool::SetOrientation(mitk::Quaternion, ScalarType)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// GetPosition
//=======================================================
void mitk::OptitrackTrackingTool::GetPosition(mitk::Point3D&) const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// GetOrientation
//=======================================================
void mitk::OptitrackTrackingTool::GetOrientation(mitk::Quaternion&) const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// Enable
//=======================================================
bool mitk::OptitrackTrackingTool::Enable()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// Disable
//=======================================================
bool mitk::OptitrackTrackingTool::Disable()
{
 MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
 return false;
}

//=======================================================
// IsEnabled
//=======================================================
bool mitk::OptitrackTrackingTool::IsEnabled() const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// IsDataValid
//=======================================================
bool mitk::OptitrackTrackingTool::IsDataValid() const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// GetTrackingError
//=======================================================
float mitk::OptitrackTrackingTool::GetTrackingError() const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return 0.0;
}

//=======================================================
// SetTrackingError
//=======================================================
void mitk::OptitrackTrackingTool::SetTrackingError(float)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// SetDataValid
//=======================================================
void mitk::OptitrackTrackingTool::SetDataValid(bool)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// updateTool
//=======================================================
void mitk::OptitrackTrackingTool::updateTool()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}


#endif
