#include "mitkWiiMoteThread.h"

#include "mitkWiiMoteAddOn.h"
#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteButtonEvent.h"

// timelimit between two state changes to send an event
// the chosen value is empirical
const double TIMELIMIT = 0.5; 
const int SLEEPDEFAULT = 70; 
const int MAX_WIIMOTES = 4;

mitk::WiiMoteThread::WiiMoteThread()
: m_MultiThreader(itk::MultiThreader::New())
, m_WiiMoteThreadFinished(itk::FastMutexLock::New())
, m_StopWiiMote(false)
, m_ThreadID(-1)
, m_LastRecordTime(0)
, m_ReadDataOnce(false)
//, m_SkipTimeSteps(4)
//, m_CurrentTimeStep(0)
, m_SleepTime(SLEEPDEFAULT)
, m_InCalibrationMode(false)
{
  // uses state-change callback to get notified of extension-related 
  // events OnStateChange must be static, otherwise the this-operator 
  // will be used and this callback can't convert it into the typedef
  // defined in the wiiyourself library
  m_WiiMote.ChangedCallback = &mitk::WiiMoteThread::OnStateChange;

  // allows flags to trigger a state change
  m_WiiMote.CallbackTriggerFlags = (state_change_flags) (CONNECTED | IR_CHANGED);
}

mitk::WiiMoteThread::~WiiMoteThread()
{
  //for(int i = 0; i < m_NumberDetectedWiiMotes;i++)
  //{  
  //  delete m_WiiMotes[i];  
  //}
}

void mitk::WiiMoteThread::OnStateChange(wiimote &remote,state_change_flags changed, const wiimote_state &newState)
{
  // TODO check for extension and configure
}

void mitk::WiiMoteThread::Run()
{ 
  m_ThreadID = m_MultiThreader->SpawnThread(this->StartWiiMoteThread, this);
}

ITK_THREAD_RETURN_TYPE mitk::WiiMoteThread::StartWiiMoteThread(void* pInfoStruct)
{
  // extract this pointer from Thread Info structure
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
    return ITK_THREAD_RETURN_VALUE;

  if (pInfo->UserData == NULL)
    return ITK_THREAD_RETURN_VALUE;

  mitk::WiiMoteThread* wiiMoteThread = static_cast<mitk::WiiMoteThread*>(pInfo->UserData);
  if (wiiMoteThread != NULL)
    wiiMoteThread->StartWiiMote();     

  return ITK_THREAD_RETURN_VALUE;
}

void mitk::WiiMoteThread::StartWiiMote()
{
  // mutex lock should be inside each method, but since
  // the driver is external, one would have to change it
 
  // transfers the execution rights to the WiiMote thread
  m_WiiMoteThreadFinished->Lock();

  // is needed in case the Wiimote was deactivated
  // while the application is running
  this->m_StopWiiMote = false;

  // transfers the execution rights back to the main thread
  m_WiiMoteThreadFinished->Unlock();

  while(CONNECTED && !m_StopWiiMote) 
  {
    m_WiiMoteThreadFinished->Lock();
    if(this->DetectWiiMotes())
    {
      break;
    }
    m_WiiMoteThreadFinished->Unlock();
  }

  if(this->m_NumberDetectedWiiMotes <= 1)
  {
    this->SingleWiiMoteUpdate();
  }
  else
  {
    this->MultiWiiMoteUpdate();
  }

  //if(m_WiiMote.IsConnected() && m_StopWiiMote)
  //{
    //clean up
    //m_WiiMoteThreadFinished->Lock();
    //m_WiiMote.Disconnect();
    //m_WiiMoteThreadFinished->Unlock();

    //MITK_INFO << "WiiMote disconnected..";
  //}

  this->DisconnectWiiMotes();

  return;
}

void mitk::WiiMoteThread::StopWiiMote()
{
  this->m_StopWiiMote = true;
}

void mitk::WiiMoteThread::ReconnectWiiMote()
{
  if(m_WiiMote.Connect(wiimote::FIRST_AVAILABLE))
  {
    MITK_INFO << "WiiMote reconnected..";
  }
}

bool mitk::WiiMoteThread::DetectWiiMotes()
{
  bool result = false;
  unsigned detected		  = 0;
  unsigned connectionTrys = 0;

  while(detected < 5)
  {
    connectionTrys++;

    if(!m_WiiMotes[detected].Connect(wiimote::FIRST_AVAILABLE))
    {
      if(connectionTrys > 5)
      {
        break;
      }
    }
    else
    {
      MITK_INFO << "WiiMote detected :: Assigned ID: " << detected;

      switch(detected)
      {
        // 0x00 none
        // 0x0f all leds
        // 0x01 first led
        // 0x02 second led
        // 0x03 third led
        // 0x04 fourth led
      case 0:
        m_WiiMotes[detected].SetLEDs(0x01);
        break;

      case 1:
        m_WiiMotes[detected].SetLEDs(0x02);
        break;

      case 2:
        m_WiiMotes[detected].SetLEDs(0x03);
        break;

      case 3:
        m_WiiMotes[detected].SetLEDs(0x04);
        break;
      }

      // use a non-extension report mode (this gives us back the IR dot sizes)
      m_WiiMotes[detected].SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);

      detected++;
      connectionTrys = 0;

      result = true;
    } // end else
  } // end while

  m_NumberDetectedWiiMotes = detected;
  return result;
}

void mitk::WiiMoteThread::DisconnectWiiMotes()
{
  for(int i = 0; i < m_NumberDetectedWiiMotes;i++)
  {  
      this->m_WiiMotes[i].Disconnect();  
  }

  MITK_INFO << "Wiimotes disconnected";
}

void mitk::WiiMoteThread::WiiMoteIRInput()
{
  if(IR_CHANGED && m_WiiMotes[0].IR.Dot[0].bVisible)
  {
    //m_CurrentTimeStep++;
    //if(m_CurrentTimeStep > m_SkipTimeSteps)
    //{
    m_Command = ReceptorCommand::New(); 
    m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteInput);

    double tempTime(itksys::SystemTools::GetTime());
    float inputCoordinates[2] = {m_WiiMotes[0].IR.Dot[0].RawX, m_WiiMotes[0].IR.Dot[0].RawY};
    mitk::Point2D tempPoint(inputCoordinates);

    // if the last read data is not valid,
    // because the thread was not started
    if(!m_ReadDataOnce)
    {
      m_ReadDataOnce = true;  
    }
    else // there is old data available - calculate the movement and send event
    {
      // this time constraint allows the user to move the camera
      // with the IR sender (by switching the IR source on and off) 
      // similiar to a mouse (e.g. if you lift the mouse from the 
      // surface and put it down again on another position, there 
      // was no input. Now the input starts again and the movement 
      // will begin from the last location of the mouse, although 
      // the physical position of the mouse changed.)
      if ((tempTime-m_LastRecordTime) < TIMELIMIT) 
      {
        mitk::Vector2D resultingVector(m_LastReadData-tempPoint);
        mitk::WiiMoteIREvent e(resultingVector, tempTime);
        mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
      }
    }
    m_LastRecordTime = tempTime;
    m_LastReadData = tempPoint;

    itksys::SystemTools::Delay(m_SleepTime);

    //m_CurrentTimeStep = 0;
    //}
  }

  //read data
  //MITK_INFO << "Pitch: " << m_WiiMote.Acceleration.Orientation.Pitch;
  //MITK_INFO << "Roll: " << m_WiiMote.Acceleration.Orientation.Roll;

  //MITK_INFO << "X: " << m_WiiMote.Acceleration.Orientation.X;
  //MITK_INFO << "Y: " << m_WiiMote.Acceleration.Orientation.Y;
  //MITK_INFO << "Z: " << m_WiiMote.Acceleration.Orientation.Z;
}

void mitk::WiiMoteThread::WiiMoteButtonPressed(int buttonType)
{
  m_Command = ReceptorCommand::New(); 
  m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteButtonPressed);

  mitk::WiiMoteButtonEvent e(mitk::Type_WiiMoteButton, mitk::BS_NoButton, mitk::BS_NoButton, buttonType);
  mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
}

void mitk::WiiMoteThread::WiiMoteCalibrationInput()
{
  if(IR_CHANGED && m_WiiMotes[0].IR.Dot[0].bVisible)
  {
    m_Command = ReceptorCommand::New();
    m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteCalibrationInput);

    mitk::WiiMoteCalibrationEvent e(m_WiiMotes[0].IR.Dot[0].RawX,m_WiiMotes[0].IR.Dot[0].RawY);
    mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
  }

  itksys::SystemTools::Delay(m_SleepTime);
}

void mitk::WiiMoteThread::SingleWiiMoteUpdate()
{
   // update and settings process
  while(!m_WiiMotes[0].Button.Minus() && !m_StopWiiMote )
  {
    m_WiiMoteThreadFinished->Lock();
    while(m_WiiMotes[0].RefreshState() == NO_CHANGE)
    {
      itksys::SystemTools::Delay(1);
    }
    m_WiiMoteThreadFinished->Unlock();

    m_WiiMoteThreadFinished->Lock();
    if(!m_InCalibrationMode)
    {
      this->WiiMoteIRInput();
    }
    else
    {
      this->WiiMoteCalibrationInput();
    }
    m_WiiMoteThreadFinished->Unlock();

    if(m_WiiMotes[0].Button.Home())
      this->WiiMoteButtonPressed(mitk::Key_Home);

    if(m_WiiMotes[0].Button.A())
    {
      this->WiiMoteButtonPressed(mitk::Key_A);

      // necessary to avoid sending the movementvector
      // instead of the raw coordinates for the calibration
      if(this->m_InCalibrationMode)
      {
        this->m_InCalibrationMode = false;
      }
      else
      {
        this->m_InCalibrationMode = true;
      }
 
      itksys::SystemTools::Delay(3000);
    }

    if(m_WiiMotes[0].ConnectionLost())
    {
      m_WiiMoteThreadFinished->Lock();
      this->ReconnectWiiMote();
      m_WiiMoteThreadFinished->Unlock();
    }

    //// testing
    //m_WiiMoteThreadFinished->Lock();
    //m_WiiMote.SetRumble(m_WiiMote.Button.B());
    //m_WiiMoteThreadFinished->Unlock();
  }
}

void mitk::WiiMoteThread::MultiWiiMoteUpdate()
{
 // update and settings process
  while(!m_WiiMotes[0].Button.Minus() && !m_StopWiiMote )
  {
    m_WiiMoteThreadFinished->Lock();
    while(m_WiiMotes[0].RefreshState() == NO_CHANGE
      && m_WiiMotes[1].RefreshState() == NO_CHANGE)
    {
      itksys::SystemTools::Delay(1);
    }
    m_WiiMoteThreadFinished->Unlock();

    m_WiiMoteThreadFinished->Lock();
    if(!m_InCalibrationMode)
    {
      this->MultiWiiMoteIRInput();
    }
    else
    {
      // stereo calibration?
    }
    m_WiiMoteThreadFinished->Unlock();

    if(m_WiiMotes[0].Button.A())
    {
      this->WiiMoteButtonPressed(mitk::Key_A);

      if(this->m_InCalibrationMode)
      {
        this->m_InCalibrationMode = false;
      }
      else
      {
        this->m_InCalibrationMode = true;
      }
 
      itksys::SystemTools::Delay(3000);
    }
  } // end while
}

void mitk::WiiMoteThread::MultiWiiMoteIRInput()
{
  if(IR_CHANGED && m_WiiMotes[0].IR.Dot[0].bVisible && m_WiiMotes[1].IR.Dot[0].bVisible)
  {
    //m_Command = ReceptorCommand::New(); 
    //m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteInput);

    double tempTime(itksys::SystemTools::GetTime());

    float inputCoordinates[2] = {m_WiiMotes[0].IR.Dot[0].RawX, m_WiiMotes[0].IR.Dot[0].RawY};
    mitk::Point2D tempPoint(inputCoordinates);

    float inputCoordinates2[2] = {m_WiiMotes[1].IR.Dot[0].RawX, m_WiiMotes[1].IR.Dot[0].RawY};
    mitk::Point2D tempPoint2(inputCoordinates2);

    mitk::Vector2D result = (tempPoint2 - tempPoint);
    MITK_INFO << "IR1 :: X: " << tempPoint[0];
    MITK_INFO << "IR1 :: Y: " << tempPoint[1];

    MITK_INFO << "IR2 :: X: " << tempPoint2[0];
    MITK_INFO << "IR2 :: Y: " << tempPoint2[1];

    //// if the last read data is not valid,
    //// because the thread was not started
    //if(!m_ReadDataOnce)
    //{
    //  m_ReadDataOnce = true;  
    //}
    //else // there is old data available - calculate the movement and send event
    //{
    //  // this time constraint allows the user to move the camera
    //  // with the IR sender (by switching the IR source on and off) 
    //  // similiar to a mouse (e.g. if you lift the mouse from the 
    //  // surface and put it down again on another position, there 
    //  // was no input. Now the input starts again and the movement 
    //  // will begin from the last location of the mouse, although 
    //  // the physical position of the mouse changed.)
    //  if ((tempTime-m_LastRecordTime) < TIMELIMIT) 
    //  {
    //    mitk::Vector2D resultingVector(m_LastReadData-tempPoint);
    //    mitk::WiiMoteIREvent e(resultingVector, tempTime);
    //    mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
    //  }
    //}
    //m_LastRecordTime = tempTime;
    //m_LastReadData = tempPoint;

    //itksys::SystemTools::Delay(m_SleepTime);
  }
}



