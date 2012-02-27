/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-07-22 16:41:18 +0200 (Fr, 17 Aug 2007) $
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkVtkEventProvider.h"
#include "mitkVtkEventAdapter.h"
#include <mbilog.h>

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>

#define VTKEVENTPROVIDER_INFO  MBI_INFO("mitk.core.vtkeventprovider")
#define VTKEVENTPROVIDER_WARN  MBI_WARN("mitk.core.vtkeventprovider")
#define VTKEVENTPROVIDER_ERROR MBI_ERROR("mitk.core.vtkeventprovider")
#define VTKEVENTPROVIDER_DEBUG MBI_DEBUG("mitk.core.vtkeventprovider")


namespace mitk
{
vtkCxxRevisionMacro(vtkEventProvider, "$Revision: 1.18 $");
vtkStandardNewMacro(vtkEventProvider);
}
//----------------------------------------------------------------------------
mitk::vtkEventProvider::vtkEventProvider()
{
  // priority of the observer/command; we want MITK events processed in the very beginning
  this->Priority = 99999.99;

  //take over the processing of delete and keypress events from the superclass
  this->EventCallbackCommand->SetCallback(
    vtkEventProvider::ProcessEvents);
  
  // Set/Get the passive observer flag. If this is set to true, this
  // indicates that this command does not change the state of the
  // system in any way. Passive observers are processed first, and
  // are not called even when another command has focus.
  this->EventCallbackCommand->SetPassiveObserver(1); // get events first

  // mouse move
  AddInteractionEvent(vtkCommand::MouseMoveEvent);
  // mouse press
  AddInteractionEvent(vtkCommand::LeftButtonPressEvent);
  AddInteractionEvent(vtkCommand::MiddleButtonPressEvent);
  AddInteractionEvent(vtkCommand::RightButtonPressEvent);
  // mouse release
  AddInteractionEvent(vtkCommand::LeftButtonReleaseEvent);
  AddInteractionEvent(vtkCommand::MiddleButtonReleaseEvent);
  AddInteractionEvent(vtkCommand::RightButtonReleaseEvent);
  // wheel event
  AddInteractionEvent(vtkCommand::MouseWheelBackwardEvent);
  AddInteractionEvent(vtkCommand::MouseWheelForwardEvent);
  // key press event
  AddInteractionEvent(vtkCommand::KeyPressEvent);
}

mitk::vtkEventProvider::~vtkEventProvider()
{
  this->SetInteractor(0);
}

void mitk::vtkEventProvider::SetMitkRenderWindow(mitk::RenderWindow* renWin)
{
  m_RenderWindow = renWin;
}

mitk::RenderWindow* mitk::vtkEventProvider::GetRenderWindow()
{
  return m_RenderWindow;
}

void mitk::vtkEventProvider::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    VTKEVENTPROVIDER_ERROR <<"The interactor must be set prior to enabling/disabling widget";
    return;
    }

  if ( enabling ) //----------------------------------------------------------
    {
    VTKEVENTPROVIDER_DEBUG << "Enabling widget";

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }
    
    this->Enabled = 1;

    // listen to all event types specified in m_InteractionEventsVector
    vtkRenderWindowInteractor *i = this->Interactor;

    InteractionEventsVectorType::iterator it;
    for(it = m_InteractionEventsVector.begin(); it != m_InteractionEventsVector.end(); it++)
    {
      // add observer to interactorStyle
      i->GetInteractorStyle()->AddObserver((vtkCommand::EventIds) (*it), this->EventCallbackCommand, 
        this->Priority);
    }

    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }

  else //disabling-----------------------------------------------------------
    {
    VTKEVENTPROVIDER_DEBUG <<"Disabling widget";

    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }

    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);
    //this->Interactor->HandleEventLoop = 0;

    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    }
}

//----------------------------------------------------------------------------
// This adds the keypress event observer and the delete event observer
void mitk::vtkEventProvider::SetInteractor(vtkRenderWindowInteractor* i)
{
  if (i == this->Interactor)
  {
    return;
  }
  // if we already have an Interactor then stop observing it
  if (this->Interactor)
    this->SetEnabled(0); //disable the old interactor
  
  this->Interactor = i;

  this->Modified();
}

//----------------------------------------------------------------------------
void mitk::vtkEventProvider::ProcessEvents(vtkObject* object, 
                                               unsigned long event,
                                               void* clientData, 
                                               void* vtkNotUsed(callData))
{

  vtkEventProvider* self = 
    reinterpret_cast<vtkEventProvider *>( clientData );
  vtkRenderWindowInteractor* rwi = 
    static_cast<vtkInteractorStyle *>( object )->GetInteractor();

  // base renderer
  mitk::BaseRenderer* baseRenderer = mitk::BaseRenderer::GetInstance(self->GetRenderWindow()->GetVtkRenderWindow());
  
  
  switch(event)
  {
    // key press
    case vtkCommand::KeyPressEvent:
    { 
      VTKEVENTPROVIDER_DEBUG << "key press event";
      mitk::KeyEvent mke(mitk::VtkEventAdapter::AdaptKeyEvent(baseRenderer,event,rwi));
      self->GetRenderWindow()->keyPressMitkEvent(&mke);
      break;
    }  

    // mouse events
    case vtkCommand::MouseMoveEvent:
  { 
      VTKEVENTPROVIDER_DEBUG << "mouse move event";
      mitk::MouseEvent me(mitk::VtkEventAdapter::AdaptMouseEvent(baseRenderer,event,rwi));
      self->GetRenderWindow()->mouseMoveMitkEvent(&me);
      break;
    }  

  case vtkCommand::LeftButtonPressEvent:
    case vtkCommand::MiddleButtonPressEvent:
    case vtkCommand::RightButtonPressEvent:
  { 
      VTKEVENTPROVIDER_DEBUG << "mouse press event";
      mitk::MouseEvent me(mitk::VtkEventAdapter::AdaptMouseEvent(baseRenderer,event,rwi));
      self->GetRenderWindow()->mousePressMitkEvent(&me);
      break;
    }  
  
  case vtkCommand::LeftButtonReleaseEvent:
    case vtkCommand::MiddleButtonReleaseEvent:
    case vtkCommand::RightButtonReleaseEvent:
    { 
      VTKEVENTPROVIDER_DEBUG << "mouse release event";
      mitk::MouseEvent me(mitk::VtkEventAdapter::AdaptMouseEvent(baseRenderer,event,rwi));
      self->GetRenderWindow()->mouseReleaseMitkEvent(&me);
      break;
    }  

  // mouse WHEEL
    case vtkCommand::MouseWheelForwardEvent:
    case vtkCommand::MouseWheelBackwardEvent:
    { 
      VTKEVENTPROVIDER_DEBUG << "mouse wheel event";
      mitk::WheelEvent we(mitk::VtkEventAdapter::AdaptWheelEvent(baseRenderer,event,rwi));
      self->GetRenderWindow()->wheelMitkEvent(&we);
      break;
    }

  default:
      VTKEVENTPROVIDER_INFO << "VTK event not mapped properly.";
    break;
  }
   
}

void mitk::vtkEventProvider::RemoveInteractionEvent(unsigned long ievent)
{
  InteractionEventsVectorType::iterator it;
  if(m_InteractionEventsVector.size() > 0)
  {
    it = std::find(m_InteractionEventsVector.begin(),m_InteractionEventsVector.end(),ievent);
    if(it != m_InteractionEventsVector.end())
    {
      m_InteractionEventsVector.erase(it);
      return;
    }
  }
}

void mitk::vtkEventProvider::AddInteractionEvent(unsigned long ievent)
{
  // Remove event if it already exists
  RemoveInteractionEvent(ievent);

  m_InteractionEventsVector.push_back(ievent);

}