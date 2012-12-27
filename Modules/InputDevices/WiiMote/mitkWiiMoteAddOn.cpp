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
#include "mitkWiiMoteAddOn.h"

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"

#include "mitkWiiMoteThread.h"
#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteButtonEvent.h"
#include "mitkWiiMoteCalibrationEvent.h"
#include <mitkWiiMoteAllDataEvent.h>

mitk::WiiMoteAddOn* mitk::WiiMoteAddOn::GetInstance()
{
  static WiiMoteAddOn instance;
  return &instance;
}

mitk::WiiMoteAddOn::WiiMoteAddOn()
: m_WiiMoteThread(NULL)
{
}

mitk::WiiMoteAddOn::~WiiMoteAddOn()
{
  this->DeactivateWiiMotes();
}

void mitk::WiiMoteAddOn::ActivateWiiMotes()
{
  if(m_WiiMoteThread == NULL)
  {
    m_WiiMoteThread = new WiiMoteThread();
    m_WiiMoteThread->Run();
  }
}

void mitk::WiiMoteAddOn::DeactivateWiiMotes()
{
  if(m_WiiMoteThread != NULL)
  {
    m_WiiMoteThread->StopWiiMote();

    //needed, otherwise the mutex wants to unlock
    //when the object is already destroyed
    Sleep(1000);

    m_WiiMoteThread->Delete();
    m_WiiMoteThread = NULL;
  }
}

void mitk::WiiMoteAddOn::WiiMoteInput(const itk::EventObject& e)
{
  try
  {
    // apparently the dynamic cast does not work here
    // reason unknown - the normal cast however works fine
    mitk::StateEvent* se = new mitk::StateEvent
      (mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteIREvent*)&e);
    this->ForwardEvent(se);
    delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Incoming event is not an IR input for" <<
      "headtracking could not be transformed\n";
  }
}

void mitk::WiiMoteAddOn::WiiMoteButtonPressed(const itk::EventObject &e)
{
  try
  {
    mitk::WiiMoteButtonEvent const* wiiEvent = (const mitk::WiiMoteButtonEvent*)(&e);
    int key = wiiEvent->GetKey();
    mitk::StateEvent* se;

    switch(key)
    {
    case mitk::Key_Home: // view reset
      se = new mitk::StateEvent(mitk::EIDWIIMOTEBUTTON, wiiEvent);
      break;
    case mitk::Key_A: // calibration start/end
      se = new mitk::StateEvent(mitk::EV_INIT, wiiEvent);
      break;
    case mitk::Key_B: // surface interaction activation
      se = new mitk::StateEvent(mitk::EIDWIIMOTEBUTTONB, wiiEvent);
      break;
    }

    if(se != NULL)
    {
      this->ForwardEvent(se);
      delete se;
    }
  }
  catch(...)
  {
    MITK_ERROR << "Incoming event is not a button event" <<
      "and could not be transformed\n";
  }
}

void mitk::WiiMoteAddOn::WiiMoteButtonReleased(const itk::EventObject &e)
{
  try
  {
    mitk::WiiMoteButtonEvent const* wiiEvent = (const mitk::WiiMoteButtonEvent*)(&e);
    int key = wiiEvent->GetKey();
    mitk::StateEvent* se;

    switch(key)
    {
    case mitk::Key_B: // surface interaction deactivation
      se = new mitk::StateEvent(mitk::EV_DONE, wiiEvent);
      break;
    }

    if(se != NULL)
    {
      this->ForwardEvent(se);
      delete se;
    }
  }
  catch(...)
  {
    MITK_ERROR << "Incoming event is not a button event" <<
      "and could not be transformed\n";
  }
}

void mitk::WiiMoteAddOn::WiiMoteCalibrationInput(const itk::EventObject &e)
{
  try
  {
  mitk::StateEvent* se = new mitk::StateEvent
    (mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteCalibrationEvent*)&e);
  this->ForwardEvent(se);
  delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Incoming event is not a calibration input for headtracking" <<
      "and could not be transformed\n";
  }
}

void mitk::WiiMoteAddOn::SetWiiMoteSurfaceIModus(bool activated)
{
  if(m_WiiMoteThread != NULL)
  {
    m_WiiMoteThread->SetWiiMoteSurfaceIModus(activated);
  }
}

void mitk::WiiMoteAddOn::WiiMoteSurfaceInteractionInput(const itk::EventObject& e)
{
  try
  {
    mitk::StateEvent* se = new mitk::StateEvent
      (mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteAllDataEvent*)&e);
    this->ForwardEvent(se);
    delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Incoming event is not an input for surface interaction" <<
      "and could not be transformed\n";
  }
}





