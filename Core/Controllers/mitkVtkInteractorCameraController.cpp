/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include "mitkVtkInteractorCameraController.h"
#include "mitkInteractionConst.h"
#include <vtkInteractorStyleSwitch.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include "mitkAction.h"
#include "mitkVtkPropRenderer.h"

mitk::VtkInteractorCameraController::VtkInteractorCameraController(const char* type) : CameraController(type), m_VtkInteractor(NULL)
{
  //m_VtkInteractor = vtkRenderWindowInteractor::New();
 
}

mitk::VtkInteractorCameraController::~VtkInteractorCameraController()
{
  if(m_VtkInteractor!=NULL)
  {
    m_VtkInteractor->SetRenderWindow(NULL);
    m_VtkInteractor->Delete();
    m_VtkInteractor = NULL;
  }
}

void mitk::VtkInteractorCameraController::Resize(int w, int h)
{
  if(m_VtkInteractor)
    m_VtkInteractor->SetSize(w, h);
}

void mitk::VtkInteractorCameraController::MousePressEvent(mitk::MouseEvent *me)
{
  std::cout<<"Called MousePressEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
//  if(m_VtkInteractor)
//  {
//    if (!m_VtkInteractor->GetEnabled())
//    {
//      return;
//    }
//    int ctrl  = me->GetButtonState() & BS_ControlButton;
//    int shift = me->GetButtonState() & BS_ShiftButton;
//#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
//    int xp = (int)me->GetDisplayPosition()[0];
//    int yp = (int)me->GetDisplayPosition()[1];
//    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
//
//    switch (me->GetButton())
//    {
//    case BS_LeftButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
//      break;
//    case BS_MidButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
//      break;
//    case BS_RightButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
//      break;
//    default:
//      return;
//    }
//#else
//    if(me->GetButton() & BS_LeftButton)
//    {
//      //    SetCapture(wnd);
//      m_VtkInteractor->InteractorStyle->OnLeftButtonDown(ctrl, shift,
//          me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//    }
//    else
//      if(me->button() & BS_MidButton)
//      {
//        //    SetCapture(wnd);
//        m_VtkInteractor->InteractorStyle->OnMiddleButtonDown(ctrl, shift,
//            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//      }
//      else
//        if(me->button() & BS_RightButton)
//        {
//          //    SetCapture(wnd);
//          m_VtkInteractor->InteractorStyle->OnRightButtonDown(ctrl, shift,
//              me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//        }
//#endif
//
//  }
}

void mitk::VtkInteractorCameraController::MouseReleaseEvent(mitk::MouseEvent *me)
{
  std::cout<<"Called MouseReleaseEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
//  if(m_VtkInteractor)
//  {
//    if (!m_VtkInteractor->GetEnabled())
//    {
//      return;
//    }
//    int ctrl  = me->GetButtonState() & BS_ControlButton;
//    int shift = me->GetButtonState() & BS_ShiftButton;
//#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
//    int xp = (int)me->GetDisplayPosition()[0];
//    int yp = (int)me->GetDisplayPosition()[1];
//    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
//
//    switch (me->GetButton())
//    {
//    case BS_LeftButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
//      break;
//    case BS_MidButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
//      break;
//    case BS_RightButton:
//      m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
//      break;
//    default:
//      return;
//    }
//#else
//    if(me->button() & BS_LeftButton)
//    {
//      m_VtkInteractor->InteractorStyle->OnLeftButtonUp(ctrl, shift,
//          me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//      //ReleaseCapture( );
//    }
//    else
//      if(me->button() & BS_MidButton)
//      {
//        m_VtkInteractor->InteractorStyle->OnMiddleButtonUp(ctrl, shift,
//            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//        //ReleaseCapture( );
//      }
//      else
//        if(me->button() & BS_RightButton)
//        {
//          m_VtkInteractor->InteractorStyle->OnRightButtonUp(ctrl, shift,
//              me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//          //ReleaseCapture( );
//        }
//#endif
//
//  }
}

void mitk::VtkInteractorCameraController::MouseMoveEvent(mitk::MouseEvent *me)
{
  std::cout<<"Called MouseMoveEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
//  if(m_VtkInteractor)
//  {
//    if (!m_VtkInteractor->GetEnabled())
//    {
//      return;
//    }
//    int ctrl  = me->GetButtonState() & BS_ControlButton;
//    int shift = me->GetButtonState() & BS_ShiftButton;
//#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
//    int xp = (int)me->GetDisplayPosition()[0];
//    int yp = (int)me->GetDisplayPosition()[1];
//    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
//    m_VtkInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
//#else
//    if (!m_VtkInteractor->MouseInWindow &&
//        ((me->GetDisplayPosition().x >= 0) && (me->GetDisplayPosition().x < m_VtkInteractor->Size[0]) && (me->GetDisplayPosition().y >= 0) && (me->GetDisplayPosition().y < m_VtkInteractor->Size[1])))
//    {
//      m_VtkInteractor->InteractorStyle->OnEnter(ctrl, shift,
//          me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//      m_VtkInteractor->MouseInWindow = 1;
//    }
//
//    if (m_VtkInteractor->MouseInWindow &&
//        ((me->GetDisplayPosition().x < 0) || (me->GetDisplayPosition().x >= m_VtkInteractor->Size[0]) || (me->GetDisplayPosition().y < 0) || (me->GetDisplayPosition().y >= m_VtkInteractor->Size[1])))
//    {
//      m_VtkInteractor->InteractorStyle->OnLeave(ctrl, shift,
//          me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//      m_VtkInteractor->MouseInWindow = 0;
//    }
//
//    m_VtkInteractor->InteractorStyle->OnMouseMove(ctrl, shift,
//        me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
//#endif
//
//  }
}

void mitk::VtkInteractorCameraController::KeyPressEvent(mitk::KeyEvent *ke)
{
    std::cout<<"Called KeyPressEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
//  if(m_VtkInteractor)
//  {
//    if (!m_VtkInteractor->GetEnabled())
//    {
//      return;
//    }
//    int ctrl  = ke->state() & BS_ControlButton;
//    int shift = ke->state() & BS_ShiftButton;
//#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
//    int xp = ke->x();
//    int yp = ke->y();
//    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift, tolower(ke->text()[0]), 1, ke->text());
//    //m_VtkInteractor->InvokeEvent(vtkCommand::KeyPressEvent, NULL);
//    //m_VtkInteractor->InvokeEvent(vtkCommand::CharEvent, NULL);
//#else
//    m_VtkInteractor->InteractorStyle->OnChar(ctrl, shift, (char)ke->ascii(), ke->count());
//#endif
//  }
}

void mitk::VtkInteractorCameraController::SetRenderer(const mitk::BaseRenderer* renderer)
{
  Superclass::SetRenderer(renderer);
  if (renderer)
  {
    // CHG 11-07: QVTK-Widget comes along with vtkRenderWindow and vtkRenWinInteractor, therefore do not 
    // generate a new one any more
    m_VtkInteractor = renderer->GetVtkRenderer()->GetRenderWindow()->GetInteractor();
		m_VtkInteractor->SetRenderWindow(renderer->GetVtkRenderer()->GetRenderWindow());
		// Enable possibility to (mouse-)interact with the renderer
		m_VtkInteractor->Enable();
    m_VtkInteractor->Register(NULL);
  }

  /*  VtkRenderWindowInteractor* windowInteractor =
      dynamic_cast<VtkRenderWindowInteractor*>(m_VtkInteractor);
    if (windowInteractor == NULL)
    {
      itkWarningMacro(<< "renderwindow is not an mitk::VtkRenderWindow");
    }
    else
    {
      windowInteractor->SetMitkRenderer(const_cast<mitk::BaseRenderer*>(this->GetRenderer()));
    }
    m_VtkInteractor->Initialize();
   
    m_VtkInteractor->SetRenderWindow(renderer->GetVtkRenderWindow());
   
  }
  else
  {
    m_VtkInteractor->SetRenderWindow(NULL);
    m_VtkInteractor->Delete();
    m_VtkInteractor = NULL;
  }*/
}

vtkRenderWindowInteractor* mitk::VtkInteractorCameraController::GetVtkInteractor()
{
  return m_VtkInteractor;
}

///*
//bool mitk::VtkInteractorCameraController::ExecuteAction(Action*, mitk::StateEvent const * /*stateEvent*/
//{
//  return false;
//}
//*/
