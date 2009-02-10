#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include <itksys/SystemTools.hxx>
#include <iostream>

bool mitk::ClaronTrackingDevice::AddTool(ClaronTool::Pointer tool)
{
  m_allTools.push_back(tool);
  return true;
}

std::vector<mitk::ClaronTool::Pointer> mitk::ClaronTrackingDevice::DetectTools()
{
  std::vector<mitk::ClaronTool::Pointer> returnValue;
  std::vector<claronToolHandle> allHandles = m_theDevice->GetAllActiveTools();
  for (std::vector<claronToolHandle>::iterator iter = allHandles.begin(); iter != allHandles.end(); ++iter)
  {
    ClaronTool::Pointer neu = ClaronTool::New();
    neu->SetToolName(m_theDevice->GetName(*iter));
    neu->SetCalibrationName(m_theDevice->GetName(*iter));
    neu->SetToolHandle(*iter);
    returnValue.push_back(neu);
  }

  return returnValue;
}

mitk::ClaronTrackingDevice::ClaronTrackingDevice(void)
{
  //set the type of this trackingdevice
  this->m_Type = ClaronMicron;

  //set the tracking volume
  this->m_TrackingVolume->SetTrackingDeviceType(this->m_Type);

  this->m_MultiThreader = itk::MultiThreader::New();
  //############################# standard directories ###############################################
  m_ToolfilesDir = std::string("C:\\TEMP\\MT");
  m_CalibrationDir = std::string("C:\\Programme\\Claron Technology\\MicronTracker\\CalibrationFiles");
  //##################################################################################################
}

mitk::ClaronTrackingDevice::~ClaronTrackingDevice(void)
{
}

bool mitk::ClaronTrackingDevice::StartTracking()
{

  //copy all toolfiles into the temp directory
  for (unsigned int i=0; i<m_allTools.size(); i++)
  {
    itksys::SystemTools::CopyAFile(m_allTools[i]->GetFile(),m_ToolfilesDir.c_str());
  }

  this->SetMode(Tracking);            // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  //restart the Microntracker, so that it will load the new tool files
  m_theDevice->StopTracking();
  delete m_theDevice;

  printf("\n ------------------------------- \n MicronTracker: Start Tracking \n Anzahl Tools: %i \n MTC: ",this->GetToolCount());
  m_theDevice = new ClaronInterface(m_CalibrationDir,m_ToolfilesDir);
  m_theDevice->StartTracking();
  printf(" -------------------------------\n");


  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
  return true;
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

  //delete all files in the tool files directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str());
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str()); 

  m_TrackingFinishedMutex->Lock();
  return true;
}

unsigned int mitk::ClaronTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_allTools.size();
}

mitk::TrackingTool* mitk::ClaronTrackingDevice::GetTool(unsigned int toolNumber)
{
  if ( toolNumber >= this->GetToolCount()) return NULL;
  else return this->m_allTools[toolNumber];
} 

bool mitk::ClaronTrackingDevice::OpenConnection()
{
  bool returnValue;
  //Create the temp directory
  itksys::SystemTools::MakeDirectory(m_ToolfilesDir.c_str());
  printf("\n ------------------------------- \n MicronTracker: Connecting ... \n MTC: ");
  m_theDevice = new ClaronInterface(m_CalibrationDir,m_ToolfilesDir);
  returnValue = m_theDevice->StartTracking();

  if (returnValue) 
  {
    printf(" Connection over Firewire established!\n -------------------------------");
    this->SetMode(Ready);
  } else {
    printf(" Connection failed, no Microntracker availiable!\n -------------------------------");

    //reset everything
    if(m_theDevice == NULL)  m_theDevice = new ClaronInterface(m_CalibrationDir,m_ToolfilesDir);
    m_theDevice->StopTracking(); 
    delete m_theDevice;

    this->SetMode(Setup);
  }

  return returnValue;

}

bool mitk::ClaronTrackingDevice::CloseConnection()
{
  bool returnValue = true; 
  if(this->GetMode() == Setup)
    return true;
  else //if(this->GetMode() == Tracking)
    returnValue = m_theDevice->StopTracking();
  if(m_theDevice != NULL)
    delete m_theDevice;

  //delete the temporary directory
  itksys::SystemTools::RemoveADirectory(m_ToolfilesDir.c_str()); 

  printf("\n ------------------------------- \n MicronTracker: Connection closed. \n ------------------------------- ");

  this->SetMode(Setup);

  return returnValue;
}

mitk::ClaronInterface* mitk::ClaronTrackingDevice::GetDevice()
{
  return m_theDevice;
}

std::vector<mitk::ClaronTool::Pointer> mitk::ClaronTrackingDevice::GetAllTools()
{
  return this->m_allTools;
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
        mitk::ClaronTool::Pointer aktu = *itAllTools;

        //schauen, ob das aktuelle Tool erkannt wurde.
        std::vector<mitk::ClaronTool::Pointer>::iterator itDetectedTools;
        bool foundTool = false;
        for(itDetectedTools = detectedTools.begin(); itDetectedTools != detectedTools.end(); itDetectedTools++)
        {
          mitk::ClaronTool::Pointer aktuDet = *itDetectedTools;
          if (aktu->GetCalibrationName().compare(aktuDet->GetCalibrationName())==0)
          {
            aktu->SetToolHandle(aktuDet->GetToolHandle());
            foundTool = true;
          }       
        }
        if (!foundTool) 
        {
          aktu->SetToolHandle(0);
        }

        if (aktu->GetToolHandle()!=0)
        {
          //get tip position of tool:
          std::vector<double> pos = this->GetDevice()->GetTipPosition(aktu->GetToolHandle());
          //write tip position into tool:
          aktu->SetPosition(pos[0],pos[1],pos[2]);
          //get tip quaternion of tool
          std::vector<double> quat = this->GetDevice()->GetTipQuaternions(aktu->GetToolHandle());
          //write tip quaternion into tool
          aktu->SetQuaternion(quat[0],quat[1],quat[2],quat[3]);

        }
        else 
        {
          aktu->SetPosition(0,0,0);
          aktu->SetQuaternion(0,0,0,0);
        }
      }
    }
  }
  catch(...)
  {
    this->StopTracking();
    this->SetErrorMessage("Error while trying to track tools. Thread stoped.");
  }

}

 bool mitk::ClaronTrackingDevice::IsMicronTrackerInstalled()
    {
    return this->m_theDevice->IsMicronTrackerInstalled();
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

  if (trackingDevice != NULL) trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}
