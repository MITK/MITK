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
#include <mitkWiiMoteThread.h>

#include <mitkWiiMoteAddOn.h>
#include <mitkWiiMoteIREvent.h>
#include <mitkWiiMoteButtonEvent.h>
#include <mitkWiiMoteCalibrationEvent.h>
#include <mitkWiiMoteAllDataEvent.h>

#include <time.h>

// timelimit between two state changes to send an event
// the chosen value is empirical
const double TIMELIMIT = 0.5;
const int MAX_WIIMOTES = 4;

mitk::WiiMoteThread::WiiMoteThread()
: m_MultiThreader(itk::MultiThreader::New())
, m_WiiMoteThreadFinished(itk::FastMutexLock::New())
, m_StopWiiMote(false)
, m_ThreadID(-1)
, m_LastRecordTime(0)
, m_ReadDataOnce(false)
, m_InCalibrationMode(false)
, m_SurfaceInteraction(false)
, m_ButtonBPressed(false)
, m_SurfaceInteractionMode(1)
, m_Kalman(mitk::KalmanFilter::New())
{
  m_Kalman->SetMeasurementNoise( 0.3 );
  m_Kalman->SetProcessNoise( 1 );
  m_Kalman->ResetFilter();

  // used for measuring movement
  m_TimeStep = 0;
}

mitk::WiiMoteThread::~WiiMoteThread()
{
  for(int i = 0; i < m_NumberDetectedWiiMotes;i++)
  {
    if(m_WiiMotes[i].IsConnected())
    {
      m_WiiMotes[i].Disconnect();
    }
  }
}

void mitk::WiiMoteThread::OnStateChange(  wiimote &remote
                                        , state_change_flags changed
                                        , const wiimote_state &newState)
{
  // a MotionPlus was detected
  if(changed & MOTIONPLUS_DETECTED)
  {

    // enable it if there isn't a normal extension plugged into it
    // (MotionPlus devices don't report like normal extensions until
    // enabled - and then, other extensions attached to it will no longer be
    // reported (so disable the M+ when you want to access them again).
    if(remote.ExtensionType == wiimote_state::NONE)
    {
      if(remote.EnableMotionPlus())
      {
        MITK_INFO << "Motion Plus enabled";
      }
    }
  }
 else if(changed & EXTENSION_CONNECTED)
      {
      // switch to a report mode that includes the extension data (we will
      // loose the IR dot sizes)
      // note: there is no need to set report types for a Balance Board.
      if(!remote.IsBalanceBoard())
      {
        remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
      }
  }
}

void mitk::WiiMoteThread::Run()
{
  m_ThreadID = m_MultiThreader->SpawnThread(this->StartWiiMoteThread, this);
}

ITK_THREAD_RETURN_TYPE mitk::WiiMoteThread::StartWiiMoteThread(void* pInfoStruct)
{
  // extract this pointer from Thread Info structure
  struct itk::MultiThreader::ThreadInfoStruct * pInfo
    = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;

  if (pInfo == NULL)
    return ITK_THREAD_RETURN_VALUE;

  if (pInfo->UserData == NULL)
    return ITK_THREAD_RETURN_VALUE;

  mitk::WiiMoteThread* wiiMoteThread
    = static_cast<mitk::WiiMoteThread*>(pInfo->UserData);

  if (wiiMoteThread != NULL)
    wiiMoteThread->StartWiiMote();

  return ITK_THREAD_RETURN_VALUE;
}

void mitk::WiiMoteThread::StartWiiMote()
{
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
  if(m_WiiMotes[0].Connect(wiimote::FIRST_AVAILABLE))
  {
    MITK_INFO << "WiiMote reconnected..";
  }
}

bool mitk::WiiMoteThread::DetectWiiMotes()
{
  bool result = false;
  unsigned detected       = 0;
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

      // ---------------- unclean solution, missing generic approach ---------------------

      // uses state-change callback to get notified of extension-related
      // events OnStateChange must be static, otherwise the this-operator
      // will be used and this callback can't convert it into the typedef
      // defined in the wiiyourself library
      m_WiiMotes[0].ChangedCallback = &mitk::WiiMoteThread::OnStateChange;

      // allows flags to trigger a state change
      m_WiiMotes[0].CallbackTriggerFlags = (state_change_flags) (CONNECTED
                                                               | EXTENSION_CHANGED
                                                               | MOTIONPLUS_CHANGED);

      m_WiiMotes[0].SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);

      // fixes the problem while in release
      // the Motion Plus cannot be connected
      // with the state change
      if(!m_WiiMotes[0].MotionPlusEnabled()
        && m_WiiMotes[0].MotionPlusConnected())
      {
        m_WiiMotes[0].EnableMotionPlus();
        m_WiiMotes[0].SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
      }

      // ----------------------------------- END ---------------------------------------

      detected++;
      connectionTrys = 0;

      result = true;
    } // end else

    itksys::SystemTools::Delay(500);

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
  if( m_WiiMotes[0].IR.Dot[0].bVisible
  /*  && m_WiiMotes[0].IR.Dot[1].bVisible*/ )
  {
    m_Command = ReceptorCommand::New();
    m_Command->SetCallbackFunction
      ( mitk::WiiMoteAddOn::GetInstance()
      , &mitk::WiiMoteAddOn::WiiMoteInput );

    double tempTime(itksys::SystemTools::GetTime());
    float inputCoordinates[2] = {m_WiiMotes[0].IR.Dot[0].RawX, m_WiiMotes[0].IR.Dot[0].RawY};
    mitk::Point2D tempPoint(inputCoordinates);

    int sliceValue = 0;

    if(m_WiiMotes[0].IR.Dot[1].bVisible)
    {
      sliceValue = ( m_WiiMotes[0].IR.Dot[1].RawY  );
    }
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
        mitk::WiiMoteIREvent e(resultingVector, tempTime, sliceValue);
        mitk::CallbackFromGUIThread::GetInstance()
          ->CallThisFromGUIThread(m_Command, e.MakeObject());
      }
    }
    m_LastRecordTime = tempTime;
    m_LastReadData = tempPoint;
  }
}

void mitk::WiiMoteThread::WiiMoteButtonPressed(int buttonType)
{
  m_Command = ReceptorCommand::New();
  m_Command->SetCallbackFunction
    ( mitk::WiiMoteAddOn::GetInstance()
    , &mitk::WiiMoteAddOn::WiiMoteButtonPressed );

  mitk::WiiMoteButtonEvent e
    ( mitk::Type_WiiMoteButton
    , mitk::BS_NoButton
    , mitk::BS_NoButton
    , buttonType );
  mitk::CallbackFromGUIThread::GetInstance()
    ->CallThisFromGUIThread(m_Command, e.MakeObject());
}

void mitk::WiiMoteThread::WiiMoteButtonReleased(int buttonType)
{
  m_Command = ReceptorCommand::New();
  m_Command->SetCallbackFunction
    ( mitk::WiiMoteAddOn::GetInstance()
    , &mitk::WiiMoteAddOn::WiiMoteButtonReleased );

  mitk::WiiMoteButtonEvent e
    ( mitk::Type_WiiMoteButton
    , mitk::BS_NoButton
    , mitk::BS_NoButton
    , buttonType );
  mitk::CallbackFromGUIThread::GetInstance()
    ->CallThisFromGUIThread(m_Command, e.MakeObject());
}

void mitk::WiiMoteThread::WiiMoteCalibrationInput()
{
  if(m_WiiMotes[0].IR.Dot[0].bVisible)
  {
    m_Command = ReceptorCommand::New();
    m_Command->SetCallbackFunction
      ( mitk::WiiMoteAddOn::GetInstance()
      , &mitk::WiiMoteAddOn::WiiMoteCalibrationInput );

    mitk::WiiMoteCalibrationEvent e
      ( m_WiiMotes[0].IR.Dot[0].RawX
      , m_WiiMotes[0].IR.Dot[0].RawY );
    mitk::CallbackFromGUIThread::GetInstance()
      ->CallThisFromGUIThread(m_Command, e.MakeObject());
  }
}

void mitk::WiiMoteThread::SingleWiiMoteUpdate()
{
  // update and settings process
  // the minus button is a manual abort criteria
  // it can be chosen differently, if needed
  while(!m_WiiMotes[0].Button.Minus() && !m_StopWiiMote)
  {
    m_WiiMoteThreadFinished->Lock();
    // refreshes the state of the wiimote
    // do not remove
    while(m_WiiMotes[0].RefreshState() == NO_CHANGE)
    {
      itksys::SystemTools::Delay(1);
    }
    m_WiiMoteThreadFinished->Unlock();

    m_WiiMoteThreadFinished->Lock();
    if(!m_SurfaceInteraction)
    {
      if(!m_InCalibrationMode)
      {
        this->WiiMoteIRInput();
      }
      else
      {
        this->WiiMoteCalibrationInput();
      }

      if(m_WiiMotes[0].Button.Home())
        this->WiiMoteButtonPressed(mitk::Key_Home);

      if(m_WiiMotes[0].Button.A())
      {
        this->WiiMoteButtonPressed(mitk::Key_A);

        // necessary to avoid sending the movement vector
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
    }
    else
    {
      // to be able to distinguish whether
      // the button B was pressed is now released;
      // state changes are defined in the StateMachine.xml
      // files
      if(m_WiiMotes[0].Button.B())
      {
        // case 1: button is now pressed and
        // was pressed before -> still surface
        // interaction
        if(m_ButtonBPressed)
        {
          this->SurfaceInteraction();
        }
        // case 2: button is now pressed and
        // was not pressed before -> allow surface
        // interaction
        else
        {
          this->WiiMoteButtonPressed(mitk::Key_B);

          // needed to set it true for the first
          // time after a release or at the start
          // of the application
          m_ButtonBPressed = true;
        }
      }
      else
      {
        // case 3: button is not pressed and
        // was pressed before -> stop surface
        // interaction
        if(m_ButtonBPressed)
        {
          this->WiiMoteButtonReleased(mitk::Key_B);
          m_ButtonBPressed = false;

          // reset to measure new movement
          m_TimeStep = 0;
        }

        // case 4: button is not pressed and
        // was not pressed before -> no change
      }

      //// used for measurements
      //// start
      //if(m_WiiMotes[0].Button.B())
      //{
      //  if(!m_ButtonBPressed)
      //  {
      //    m_ButtonBPressed = true;
      //  }
      //}

      //// stop
      //if(m_WiiMotes[0].Button.A())
      //{
      //  if(m_ButtonBPressed)
      //  {
      //    m_ButtonBPressed = false;
      //    m_TimeStep = 0;
      //    itksys::SystemTools::Delay(1000);
      //  }
      //}

      //// recording
      //if(m_ButtonBPressed)
      //{
      //  this->SurfaceInteraction();
      //}

      // reset object
      if(m_WiiMotes[0].Button.Home())
        this->WiiMoteButtonPressed(mitk::Key_Home);

      // interaction modes

      // surface interaction relative to object
      if(m_WiiMotes[0].Button.One())
        this->m_SurfaceInteractionMode = 1;

      // surface interaction relative to camera
      if(m_WiiMotes[0].Button.Two())
        this->m_SurfaceInteractionMode = 2;
    }
    m_WiiMoteThreadFinished->Unlock();

    if(m_WiiMotes[0].ConnectionLost())
    {
      m_WiiMoteThreadFinished->Lock();
      this->ReconnectWiiMote();
      m_WiiMoteThreadFinished->Unlock();
    }
  }
}

void mitk::WiiMoteThread::MultiWiiMoteUpdate()
{
 // update and settings process
  while(!m_WiiMotes[0].Button.Minus() && !m_StopWiiMote )
  {
    m_WiiMoteThreadFinished->Lock();
    // refreshes the state of the wiimotes
    // do not remove
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
     // calibration
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
  // testing multiple wiimotes
  if(m_WiiMotes[0].IR.Dot[0].bVisible
    && m_WiiMotes[1].IR.Dot[0].bVisible)
  {
    float inputCoordinates[2] = {m_WiiMotes[0].IR.Dot[0].RawX, m_WiiMotes[0].IR.Dot[0].RawY};
    mitk::Point2D tempPoint(inputCoordinates);

    float inputCoordinates2[2] = {m_WiiMotes[1].IR.Dot[0].RawX, m_WiiMotes[1].IR.Dot[0].RawY};
    mitk::Point2D tempPoint2(inputCoordinates2);

    mitk::Vector2D result = (tempPoint2 - tempPoint);
    MITK_INFO << "IR1 :: X: " << tempPoint[0];
    MITK_INFO << "IR1 :: Y: " << tempPoint[1];

    MITK_INFO << "IR2 :: X: " << tempPoint2[0];
    MITK_INFO << "IR2 :: Y: " << tempPoint2[1];
  }
}

void mitk::WiiMoteThread::SetWiiMoteSurfaceIModus(bool active)
{
  this->m_SurfaceInteraction = active;
}

void mitk::WiiMoteThread::SurfaceInteraction()
{
  m_Command = ReceptorCommand::New();
  m_Command->SetCallbackFunction
    ( mitk::WiiMoteAddOn::GetInstance()
    , &mitk::WiiMoteAddOn::WiiMoteSurfaceInteractionInput );

  mitk::WiiMoteAllDataEvent e
    ( mitk::Type_WiiMoteInput
    , m_WiiMotes[0].MotionPlus.Speed.Pitch
    , m_WiiMotes[0].MotionPlus.Speed.Roll
    , m_WiiMotes[0].MotionPlus.Speed.Yaw
    , m_WiiMotes[0].Acceleration.Orientation.X
    , m_WiiMotes[0].Acceleration.Orientation.Y
    , m_WiiMotes[0].Acceleration.Orientation.Z
    , m_WiiMotes[0].Acceleration.Orientation.Roll
    , m_WiiMotes[0].Acceleration.Orientation.Pitch
    , m_WiiMotes[0].Acceleration.X
    , m_WiiMotes[0].Acceleration.Y
    , m_WiiMotes[0].Acceleration.Z
    , m_SurfaceInteractionMode );

  //float test = m_Kalman->ProcessValue( m_WiiMotes[0].Acceleration.Z );

  mitk::CallbackFromGUIThread::GetInstance()
    ->CallThisFromGUIThread(m_Command, e.MakeObject());

  //double tempTime(itksys::SystemTools::GetTime());
  //double diff = tempTime - m_LastRecordTime;
  //
  //MITK_INFO << "Speed Pitch: " << m_WiiMotes[0].MotionPlus.Speed.Pitch;
  //MITK_INFO << "Speed Roll: " << m_WiiMotes[0].MotionPlus.Speed.Roll;
  //MITK_INFO << "Speed Yaw: " << m_WiiMotes[0].MotionPlus.Speed.Yaw;

  //MITK_INFO << "Speed X: " << m_WiiMotes[0].Acceleration.X;
  //MITK_INFO << "Speed Y: " << m_WiiMotes[0].Acceleration.Y;
  //MITK_INFO << "Speed Z: " << m_WiiMotes[0].Acceleration.Z;

  //MITK_INFO << "Time difference: " << diff;

  //if(m_TimeStep <= 400)
  //{
  //  std::ofstream file;
  //  file.open("C:/WiiMotionData/motion.txt",ios::app);

  //  file << m_TimeStep << " "
  //    << m_WiiMotes[0].Acceleration.X << " "
  //    << m_WiiMotes[0].Acceleration.Y << " "
  //    << m_WiiMotes[0].Acceleration.Z << " "
  //    << m_WiiMotes[0].MotionPlus.Speed.Pitch << " "
  //    << m_WiiMotes[0].MotionPlus.Speed.Yaw << " "
  //    << m_WiiMotes[0].MotionPlus.Speed.Roll << " "
  //    << m_WiiMotes[0].Acceleration.Orientation.X << " "
  //    << m_WiiMotes[0].Acceleration.Orientation.Y << " "
  //    << m_WiiMotes[0].Acceleration.Orientation.Z << " "
  //    << m_WiiMotes[0].Acceleration.Orientation.Pitch << " "
  //    << m_WiiMotes[0].Acceleration.Orientation.Roll << " "
  //    << std::endl;
  //  file.close();

  //  // remove, if not needed
  //  // otherwise this will eventually
  //  // disable correct functionality of Headtracking
  //  // since it's using its m_LastRecordTime
  //  //m_LastRecordTime = tempTime;
  //  m_TimeStep++;
  //}

}



