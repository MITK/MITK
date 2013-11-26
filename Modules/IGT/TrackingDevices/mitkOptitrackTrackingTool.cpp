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
#include "mitkOptitrackTrackingTool.h"


// Constructors
mitk::OptitrackTrackingTool::OptitrackTrackingTool()
  :  mitk::InternalTrackingTool(),
    ID(-1)
{
}

mitk::OptitrackTrackingTool::~OptitrackTrackingTool()
{
    // Delete trackable using ID
    //if(this->ID >= 0) DeleteTrackable();
}



bool mitk::OptitrackTrackingTool::SetToolByFileName(std::string nameFile)
{
  MITK_INFO<<"DEBUG: OptitrackTrackingTool::SetToolByFileName starts reading file "<<nameFile;
    this->fileConfiguration = nameFile;

    char* aux = new char[200];
    std::string string_aux = "";

    // Open the calibration File
    FILE* calib_file = fopen(this->fileConfiguration.c_str(),"r");

    if (calib_file == NULL)
    {
        perror("Cannot Find the Calibration File for this tool");
        exit(EXIT_FAILURE);
    }

    // Copy the tool Name from the File
    fscanf(calib_file,"%s\n",this->m_ToolName);

    string_aux = "";
    string_aux += "Creating tool called: ";
    string_aux += this->m_ToolName;
    MITK_INFO << string_aux;

    // Copy the number of points
    fscanf(calib_file,"%i\n",&(this->numPoints));
    string_aux = "";
    string_aux += "Reading num of Points: ";
    string_aux += _itoa(this->numPoints,aux,10);
    MITK_INFO << string_aux;

    // Read the Calibration Point locations and save them
    this->calibrationPoints = new float[3*this->numPoints];

    for(int i=0; i<this->numPoints; i++)
    {
        fscanf(calib_file,"%fe ",  &this->calibrationPoints[i*3+0]);
        fscanf(calib_file,"%fe ",  &this->calibrationPoints[i*3+1]);
        fscanf(calib_file,"%fe\n",  &this->calibrationPoints[i*3+2]);

        this->calibrationPoints[i*3+0] = this->calibrationPoints[i*3+0]/1000;
        this->calibrationPoints[i*3+1] = this->calibrationPoints[i*3+1]/1000;
        this->calibrationPoints[i*3+2] = this->calibrationPoints[i*3+2]/1000;
    }

    string_aux = "";
    string_aux += "Reading Points: ";
    string_aux += _itoa(this->numPoints,aux,10);
    MITK_INFO << string_aux;

    // Read the Pivot Point location
    this->pivotPoint = new float[3];
    fscanf(calib_file,"%fe ",  &this->pivotPoint[0]);
    fscanf(calib_file,"%fe ",  &this->pivotPoint[1]);
    fscanf(calib_file,"%fe\n",  &this->pivotPoint[2]);

    // mm -> m
    this->pivotPoint[0] = this->pivotPoint[0]/1000;
    this->pivotPoint[1] = this->pivotPoint[1]/1000;
    this->pivotPoint[2] = this->pivotPoint[2]/1000;

    string_aux = "";
    string_aux += "Reading Pivot Point... ";
    MITK_INFO << string_aux;

    this->ID = this->get_IDnext();

    // Create the Tool
    while(true)
    {
        if(NPRESULT_SUCCESS == TT_Update())
        {
      NPRESULT msg = TT_CreateTrackable(m_ToolName.c_str(), this->ID,this->numPoints,this->calibrationPoints);
      if(msg != NPRESULT_SUCCESS)
        MITK_INFO<<"Cannot create Trackable... "<< GetOptitrackErrorMessage(msg);
      else
        MITK_INFO<<"Trackable Created";
            break;
        }
    }

    // Translate the Pivot Point
    while(true)
    {
        if(NPRESULT_SUCCESS == TT_Update())
        {
            NPRESULT msg= TT_TrackableTranslatePivot(this->ID,this->pivotPoint[0],this->pivotPoint[1],this->pivotPoint[2]);
      if(msg != NPRESULT_SUCCESS)
        MITK_INFO<<"Cannot update Pivot... "<< GetOptitrackErrorMessage(msg);
      else
        MITK_INFO<<"Pivot updated";
            break;
        }
    }
    delete[] aux;

  return true;
}

int mitk::OptitrackTrackingTool::get_IDnext()
{
    int num_trackables;
    while(true)
    {
        if(NPRESULT_SUCCESS == TT_Update())
        {
            num_trackables = TT_TrackableCount();
            break;
        }
    }
    return (num_trackables) ;
}

void mitk::OptitrackTrackingTool::DeleteTrackable()
{
  NPRESULT msg = TT_RemoveTrackable(this->ID);
  if(msg != NPRESULT_SUCCESS)
    MITK_INFO<<"Cannot remove Trackable... "<<GetOptitrackErrorMessage(msg);
  else
    MITK_INFO<<"Trackable removed";
}

void mitk::OptitrackTrackingTool::SetPosition(mitk::Point3D position)
{
    ///< sets the position
    this->m_Position[0] = position[0];
    this->m_Position[1] = position[1];
    this->m_Position[2] = position[2];
}

void mitk::OptitrackTrackingTool::SetOrientation(mitk::Quaternion orientation)
{
    ///< sets the orientation as a quaternion
    this->m_Orientation.x() = orientation.x();
    this->m_Orientation.y() = orientation.y();
    this->m_Orientation.z() = orientation.z();
    this->m_Orientation.r() = orientation.r();
}

void mitk::OptitrackTrackingTool::GetPosition(mitk::Point3D& positionOutput) const
{
    ///< returns the current position of the tool as an array of three floats (in the tracking device coordinate system)
    positionOutput[0] = this->m_Position[0];
    positionOutput[1] = this->m_Position[1];
    positionOutput[2] = this->m_Position[2];
}

void mitk::OptitrackTrackingTool::GetOrientation(mitk::Quaternion& orientation) const
{
    ///< returns the current orientation of the tool as a quaternion (in the tracking device coordinate system)
    orientation.x() = this->m_Orientation.x();
    orientation.y() = this->m_Orientation.y();
    orientation.z() = this->m_Orientation.z();
    orientation.r() = this->m_Orientation.r();
}

bool mitk::OptitrackTrackingTool::Enable()
{
    ///< enablea the tool, so that it will be tracked. Returns true if enabling was successfull
    TT_SetTrackableEnabled(this->ID, true);
    return TT_TrackableEnabled(this->ID);
}

bool mitk::OptitrackTrackingTool::Disable()
{
    ///< disables the tool, so that it will not be tracked anymore. Returns true if disabling was successfull
    TT_SetTrackableEnabled(this->ID, false);
    return !TT_TrackableEnabled(this->ID); ;
}

bool mitk::OptitrackTrackingTool::IsEnabled() const
{
    ///< returns whether the tool is enabled or disabled
    return TT_TrackableEnabled(this->ID);
}

bool mitk::OptitrackTrackingTool::IsDataValid() const
{
    ///< returns true if the current position data is valid (no error during tracking, tracking error below threshold, ...)
    return TT_IsTrackableTracked(this->ID);
}

float mitk::OptitrackTrackingTool::GetTrackingError() const
{
    ///< return one value that corresponds to the overall tracking error. The dimension of this value is specific to each tracking device
    return 0.0;
}

void mitk::OptitrackTrackingTool::SetTrackingError(float error)
{
    ///< sets the tracking error
    this->m_TrackingError = error;
}

void mitk::OptitrackTrackingTool::SetDataValid(bool _arg)
{
    ///< sets if the tracking data (position & Orientation) is valid
    this->m_DataValid = _arg;
}

void mitk::OptitrackTrackingTool::SetErrorMessage(const char* _arg)
{
    ///< sets the error message
    MITK_INFO << "[ERROR]: OptitrackTool";
}

void mitk::OptitrackTrackingTool::updateTool()
{
    int nFrameCounter=0;
    float yaw,pitch,roll;
    float data[7];

  //while(true)
  //{
    if(NPRESULT_SUCCESS == TT_Update()
      && TT_IsTrackableTracked(this->ID))
    {
      TT_TrackableLocation(this->ID,  &data[0],  &data[1],  &data[2],
                  &data[3],  &data[4],  &data[5],  &data[6],
                  &yaw,    &pitch,    &roll);

      for(unsigned int i=0; i<7; i++)
        if (false)//if(boost::math::isinf<float>(data[i]))
        {
          this->SetDataValid(false);
          return;
        }
      this->m_Position[0] = data[0];
      this->m_Position[1] = data[1];
      this->m_Position[2] = -data[2];

      this->m_Orientation.x() = data[3];
      this->m_Orientation.y() = data[4];
      this->m_Orientation.z() = data[5];
      this->m_Orientation.r() = data[6];

      this->SetDataValid(true);
    }
    else
      this->SetDataValid(false);
//  }
}
