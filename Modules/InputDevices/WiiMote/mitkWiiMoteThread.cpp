#include "mitkWiiMoteThread.h"

#include "mitkWiiMoteAddOn.h"
#include "mitkWiiMoteEvent.h"

// static variables need declaration,
// so that they can be linked
//wiimote mitk::WiiMoteThread::m_WiiMote;
//mitk::Point2D mitk::WiiMoteThread::m_LastReadData;
//double mitk::WiiMoteThread::m_LastRecordTime;
//bool mitk::WiiMoteThread::s_ReadDataOnce = false;
//mitk::WiiMoteThread::ReceptorCommandPointer mitk::WiiMoteThread::m_Command;
//int mitk::WiiMoteThread::s_SkipTimeSteps = 1;
//int mitk::WiiMoteThread::m_CurrentTimeSteps = 0;
//wiimote mitk::WiiMoteAddOn::m_WiiMotes;

// timelimit between two state changes to send an event
// the chosen value is empirical
const double TIMELIMIT = 3; 

mitk::WiiMoteThread::WiiMoteThread()
: m_MultiThreader(itk::MultiThreader::New())
, m_WiiMoteThreadFinished(itk::FastMutexLock::New())
, m_StopWiiMote(false)
, m_ThreadID(-1)
, m_LastRecordTime(0)
, m_ReadDataOnce(false)
, m_SkipTimeSteps(4)
, m_CurrentTimeStep(0)
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
    // connection process
    if(m_WiiMote.Connect(wiimote::FIRST_AVAILABLE))
    {
      m_WiiMoteThreadFinished->Lock();

      MITK_INFO << "WiiMote connected..";

      // 0x00 none
      // 0x0f all leds
      // 0x01 first led
      // 0x02 second led
      // 0x03 third led
      // 0x04 fourth led
      m_WiiMote.SetLEDs(0x01);

      // use a non-extension report mode (this gives us back the IR dot sizes)
      m_WiiMote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);

      m_WiiMoteThreadFinished->Unlock();

      break;
    }
  }

  // update and settings process
  while(!m_WiiMote.Button.Minus() && !m_StopWiiMote )
  {
    m_WiiMoteThreadFinished->Lock();
    while(m_WiiMote.RefreshState() == NO_CHANGE)
    {
      Sleep(1);
    }
    m_WiiMoteThreadFinished->Unlock();

    m_WiiMoteThreadFinished->Lock();
    this->WiiMoteInput();
    m_WiiMoteThreadFinished->Unlock();

    if(m_WiiMote.Button.Home())
    {
      this->WiiMoteHomeButton();
    }

    if(m_WiiMote.ConnectionLost())
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

  //if(m_WiiMote.IsConnected() && m_StopWiiMote)
  //{
    //clean up
    m_WiiMoteThreadFinished->Lock();
    m_WiiMote.Disconnect();
    m_WiiMoteThreadFinished->Unlock();

    MITK_INFO << "WiiMote disconnected..";
  //}


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

void mitk::WiiMoteThread::DetectWiiMotes()
{
  //unsigned detected		  = 0;
  //unsigned connectionTries = 0;

  //while(detected < 3 || connectionTrys < 5)
  //{
  //  ++connectionTrys;
  //
  //  wiimote* newWiiMote;

  //  if(!newWiiMote->Connect(wiimote::FIRST_AVAILABLE))
  //    break;

  //  MITK_INFO << "New WiiMote detected :: Assigned ID: " << detected;
  //  m_WiiMotes[detected++] = newWiiMote;
  //}
}

void mitk::WiiMoteThread::WiiMoteInput()
{
  if(IR_CHANGED && m_WiiMote.IR.Dot[0].bVisible)
  {
    m_CurrentTimeStep++;
    if(m_CurrentTimeStep > m_SkipTimeSteps)
    {
      m_Command = ReceptorCommand::New(); 
      m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteInput);

      double tempTime(itksys::SystemTools::GetTime());
      float inputCoordinates[2] = {m_WiiMote.IR.Dot[0].RawX, m_WiiMote.IR.Dot[0].RawY};
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

          // -------------------------------- old ---------------------------------------
          //        mitk::WiiMoteEvent* e = new mitk::WiiMoteEvent(resultingVector, tempTime);
          //        itk::AnyEvent* tempEvent;
          //
          //        if(tempEvent = dynamic_cast<itk::AnyEvent*>(e))
          //        {
          //          mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, tempEvent);
          //        }
          //
          // not working

          // -------------------------------- new ---------------------------------------
          mitk::WiiMoteEvent e(resultingVector, tempTime);
          mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
        }
      }
      m_LastRecordTime = tempTime;
      m_LastReadData = tempPoint;

      m_CurrentTimeStep = 0;
      }
  }

  //read data
  //MITK_INFO << "Pitch: " << m_WiiMote.Acceleration.Orientation.Pitch;
  //MITK_INFO << "Roll: " << m_WiiMote.Acceleration.Orientation.Roll;

  //MITK_INFO << "X: " << m_WiiMote.Acceleration.Orientation.X;
  //MITK_INFO << "Y: " << m_WiiMote.Acceleration.Orientation.Y;
  //MITK_INFO << "Z: " << m_WiiMote.Acceleration.Orientation.Z;
}

void mitk::WiiMoteThread::WiiMoteHomeButton()
{
  m_Command = ReceptorCommand::New(); 
  m_Command->SetCallbackFunction(mitk::WiiMoteAddOn::GetInstance(), &mitk::WiiMoteAddOn::WiiMoteHomeButton);

  mitk::WiiMoteEvent e(mitk::Type_WiiMoteHomeButton);
  mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(m_Command, e.MakeObject());
}






