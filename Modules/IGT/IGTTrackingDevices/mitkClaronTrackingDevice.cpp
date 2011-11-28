/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkIGTConfig.h"
#include "mitkTimeStamp.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::ClaronTrackingDevice::ClaronTrackingDevice(): mitk::TrackingDevice()
{
  //set the type of this tracking device
  this->m_Data = mitk::DeviceDataMicronTrackerH40;

  this->m_MultiThreader = itk::MultiThreader::New();
  m_ThreadID = 0;

  m_Device = mitk::ClaronInterface::New();
  //############################# standard directories (from cmake) ##################################
  if (m_Device->IsMicronTrackerInstalled())
  {
#ifdef MITK_MICRON_TRACKER_TEMP_DIR
    m_ToolfilesDir = std::string(MITK_MICRON_TRACKER_TEMP_DIR);
	m_ToolfilesDir.append("/MT-tools");
#endif
#ifdef MITK_MICRON_TRACKER_CALIBRATION_DIR
    m_CalibrationDir = std::string(MITK_MICRON_TRACKER_CALIBRATION_DIR);
#endif
  }
  else
  {
    m_ToolfilesDir = "Error - No Microntracker installed";
    m_CalibrationDir = "Error - No Microntracker installed";
  }
  //##################################################################################################
  m_Device->Initialize(m_CalibrationDir, m_ToolfilesDir);
}


mitk::ClaronTrackingDevice::~ClaronTrackingDevice()
{
}


mitk::TrackingTool* mitk::ClaronTrackingDevice::AddTool( const char* toolName, const char* fileName )
{
  mitk::ClaronTool::Pointer t = mitk::ClaronTool::New();
  if (t->LoadFile(fileName) == false)
  {
    return NULL;
  }
  t->SetToolName(toolName);
  if (this->InternalAddTool(t) == false)
    return NULL;
  return t.GetPointer();
}


bool mitk::ClaronTrackingDevice::InternalAddTool(ClaronTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}


std::vector<mitk::ClaronTool::Pointer> mitk::ClaronTrackingDevice::DetectTools()
{
  std::vector<mitk::ClaronTool::Pointer> returnValue;
  std::vector<claronToolHandle> allHandles = m_Device->GetAllActiveTools();
  for (std::vector<claronToolHandle>::iterator iter = allHandles.begin(); iter != allHandles.end(); ++iter)
  {
    ClaronTool::Pointer newTool = ClaronTool::New();
    newTool->SetToolName(m_Device->GetName(*iter));
    newTool->SetCalibrationName(m_Device->GetName(*iter));
    newTool->SetToolHandle(*iter);
    returnValue.push_back(newTool);
  }
  return returnValue;
}


bool mitk::ClaronTrackingDevice::StartTracking()
{

  //By Alfred: next line because no temp directory is set if MicronTracker is not installed
  if (!m_Device->IsMicronTrackerInstalled())
    return false;
  //##################################################################################

  //be sure that the temp-directory is empty at start: delete all files in the tool files directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str());

  //copy all toolfiles into the temp directory
  for (unsigned int i=0; i<m_AllTools.size(); i++)
  {
    itksys::SystemTools::CopyAFile(m_AllTools[i]->GetFile().c_str(), m_ToolfilesDir.c_str());
  }
  this->SetState(Tracking);            // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  //restart the Microntracker, so it will load the new tool files
  m_Device->StopTracking();
  m_Device->Initialize(m_CalibrationDir,m_ToolfilesDir);

  m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

  if (m_Device->StartTracking())
  {
    mitk::TimeStamp::GetInstance()->Start(this);
    m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
    return true;
  }
  else
  {
    m_ErrorMessage = "Error while trying to start the device!";
    return false;
  }
}


bool mitk::ClaronTrackingDevice::StopTracking()
{
  Superclass::StopTracking();
  //delete all files in the tool files directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());
  return true;
}


unsigned int mitk::ClaronTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::ClaronTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if ( toolNumber >= this->GetToolCount())
    return NULL;
  else
    return this->m_AllTools[toolNumber];
}


bool mitk::ClaronTrackingDevice::OpenConnection()
{
  bool returnValue;
  //Create the temp directory
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str());

  m_Device->Initialize(m_CalibrationDir,m_ToolfilesDir);
  returnValue = m_Device->StartTracking();

  if (returnValue)
  {
    this->SetState(Ready);
  }
  else
  {
    //reset everything
    if (m_Device.IsNull())
    {
      m_Device = mitk::ClaronInterface::New();
      m_Device->Initialize(m_CalibrationDir, m_ToolfilesDir);
    }
    m_Device->StopTracking();
    this->SetState(Setup);
    m_ErrorMessage = "Error while trying to open connection to the MicronTracker.";
  }
  return returnValue;
}


bool mitk::ClaronTrackingDevice::CloseConnection()
{
  bool returnValue = true;
  if (this->GetState() == Setup)
    return true;

  returnValue = m_Device->StopTracking();

  //delete the temporary directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());

  this->SetState(Setup);
  return returnValue;
}


mitk::ClaronInterface* mitk::ClaronTrackingDevice::GetDevice()
{
  return m_Device;
}


std::vector<mitk::ClaronTool::Pointer> mitk::ClaronTrackingDevice::GetAllTools()
{
  return this->m_AllTools;
}


void mitk::ClaronTrackingDevice::TrackTools()
{
  try
  {
    /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
    MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

    bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
    this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
    localStopTracking = this->m_StopTracking;
    this->m_StopTrackingMutex->Unlock();

    while ((this->GetState() == Tracking) && (localStopTracking == false))
    {
      this->GetDevice()->GrabFrame();

      std::vector<mitk::ClaronTool::Pointer> detectedTools = this->DetectTools();
      std::vector<mitk::ClaronTool::Pointer> allTools = this->GetAllTools();
      std::vector<mitk::ClaronTool::Pointer>::iterator itAllTools;
      for(itAllTools = allTools.begin(); itAllTools != allTools.end(); itAllTools++)
      {
        mitk::ClaronTool::Pointer currentTool = *itAllTools;
        //test if current tool was detected
        std::vector<mitk::ClaronTool::Pointer>::iterator itDetectedTools;
        bool foundTool = false;
        for(itDetectedTools = detectedTools.begin(); itDetectedTools != detectedTools.end(); itDetectedTools++)
        {
          mitk::ClaronTool::Pointer aktuDet = *itDetectedTools;
          std::string tempString(currentTool->GetCalibrationName());
          if (tempString.compare(aktuDet->GetCalibrationName())==0)
          {
            currentTool->SetToolHandle(aktuDet->GetToolHandle());
            foundTool = true;
          }
        }
        if (!foundTool)
        {
          currentTool->SetToolHandle(0);
        }

        if (currentTool->GetToolHandle() != 0)
        {
          currentTool->SetDataValid(true);
          //get tip position of tool:
          std::vector<double> pos_vector = this->GetDevice()->GetTipPosition(currentTool->GetToolHandle());
          //write tip position into tool:
          mitk::Point3D pos;
          pos[0] = pos_vector[0];
          pos[1] = pos_vector[1];
          pos[2] = pos_vector[2];
          currentTool->SetPosition(pos);
          //get tip quaternion of tool
          std::vector<double> quat = this->GetDevice()->GetTipQuaternions(currentTool->GetToolHandle());
          //write tip quaternion into tool
          mitk::Quaternion orientation(quat[1], quat[2], quat[3], quat[0]);
          currentTool->SetOrientation(orientation);
        }
        else
        {
          mitk::Point3D origin;
          origin.Fill(0);
          currentTool->SetPosition(origin);
          currentTool->SetOrientation(mitk::Quaternion(0,0,0,0));
          currentTool->SetDataValid(false);
        }
      }
      /* Update the local copy of m_StopTracking */
      this->m_StopTrackingMutex->Lock();
      localStopTracking = m_StopTracking;
      this->m_StopTrackingMutex->Unlock();
    }
  }
  catch(...)
  {
    this->StopTracking();
    this->SetErrorMessage("Error while trying to track tools. Thread stopped.");
  }
}


bool mitk::ClaronTrackingDevice::IsMicronTrackerInstalled()
{
  return this->m_Device->IsMicronTrackerInstalled();
}


ITK_THREAD_RETURN_TYPE mitk::ClaronTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  ClaronTrackingDevice *trackingDevice = (ClaronTrackingDevice*)pInfo->UserData;

  if (trackingDevice != NULL)
    trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}