#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkIGTConfig.h"
#include "mitkTimeStamp.h"
#include <itksys/SystemTools.hxx>
#include <iostream>



mitk::ClaronTrackingDevice::~ClaronTrackingDevice()
{
}


bool mitk::ClaronTrackingDevice::AddTool(ClaronTool::Pointer tool)
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
    ClaronTool::Pointer neu = ClaronTool::New();
    neu->SetToolName(m_Device->GetName(*iter));
    neu->SetCalibrationName(m_Device->GetName(*iter));
    neu->SetToolHandle(*iter);
    returnValue.push_back(neu);
  }
  return returnValue;
}

mitk::ClaronTrackingDevice::ClaronTrackingDevice(void)
{
  //set the type of this tracking device
  this->m_Type = ClaronMicron;

  //set the tracking volume
  this->m_TrackingVolume->SetTrackingDeviceType(this->m_Type);

  this->m_MultiThreader = itk::MultiThreader::New();

  //############################# standard directories (from cmake) ##################################
  if (m_Device->IsMicronTrackerInstalled())
    {
    #ifdef MITK_MICRON_TRACKER_TEMP_DIR
    m_ToolfilesDir = std::string(MITK_MICRON_TRACKER_TEMP_DIR);
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
}



bool mitk::ClaronTrackingDevice::StartTracking()
{

  //By Alfred: next line because no temp directory is set if MicronTracker is not installed
  if (!m_Device->IsMicronTrackerInstalled()) return false;
  //##################################################################################

  //copy all toolfiles into the temp directory
  for (unsigned int i=0; i<m_AllTools.size(); i++)
  {
    itksys::SystemTools::CopyAFile(m_AllTools[i]->GetFile(), m_ToolfilesDir.c_str());
  }
  this->SetMode(Tracking);            // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  //restart the Microntracker, so it will load the new tool files
  m_Device->StopTracking();
  delete m_Device;

  m_Device = new ClaronInterface(m_CalibrationDir, m_ToolfilesDir);
  if (m_Device->StartTracking())
  {
    mitk::TimeStamp::GetInstance()->StartTracking(this);
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
  if (this->GetMode() == Tracking) // Only if the object is in the correct state
  {
    m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTracking = true;
    m_StopTrackingMutex->Unlock();
    this->SetMode(Ready);
  }

  m_TrackingFinishedMutex->Lock();
  mitk::TimeStamp::GetInstance()->StopTracking(this);

  //delete all files in the tool files directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str());

  return true;
}


unsigned int mitk::ClaronTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::ClaronTrackingDevice::GetTool(unsigned int toolNumber)
{
  if ( toolNumber >= this->GetToolCount())
    {
    m_ErrorMessage = "Error while trying to get a tool!";
    return NULL;
    }
  else
    return this->m_AllTools[toolNumber];
}


bool mitk::ClaronTrackingDevice::OpenConnection()
{
  bool returnValue;
  //Create the temp directory
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str());

  m_Device = new ClaronInterface(m_CalibrationDir, m_ToolfilesDir);
  returnValue = m_Device->StartTracking();

  if (returnValue)
  {
    this->SetMode(Ready);
  }
  else
  {
    //reset everything
    if(m_Device == NULL)
      m_Device = new ClaronInterface(m_CalibrationDir, m_ToolfilesDir);
    m_Device->StopTracking();
    delete m_Device;
    this->SetMode(Setup);
    m_ErrorMessage = "Error while trying to open connection to the MicronTracker 2!";
  }
  return returnValue;
}


bool mitk::ClaronTrackingDevice::CloseConnection()
{
  bool returnValue = true;
  if(this->GetMode() == Setup)
    return true;

  returnValue = m_Device->StopTracking();
  delete m_Device;

  //delete the temporary directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());

  this->SetMode(Setup);
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
    while (this->GetMode() == Tracking)
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
          std::vector<double> pos = this->GetDevice()->GetTipPosition(currentTool->GetToolHandle());
          //write tip position into tool:
          currentTool->SetPosition(pos[0], pos[1], pos[2]);
          //get tip quaternion of tool
          std::vector<double> quat = this->GetDevice()->GetTipQuaternions(currentTool->GetToolHandle());
          //write tip quaternion into tool
          currentTool->SetQuaternion(quat[0], quat[1], quat[2], quat[3]);
        }
        else
        {
          currentTool->SetPosition(0,0,0);
          currentTool->SetQuaternion(0,0,0,0);
          currentTool->SetDataValid(false);
        }
      }
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