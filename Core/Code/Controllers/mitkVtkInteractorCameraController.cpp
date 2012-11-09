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
  MITK_INFO<<"Called MousePressEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
  if(m_VtkInteractor)
  {
    if (!m_VtkInteractor->GetEnabled())
    {
      return;
    }
    int ctrl  = me->GetButtonState() & BS_ControlButton;
    int shift = me->GetButtonState() & BS_ShiftButton;
    int xp = (int)me->GetDisplayPosition()[0];
    int yp = (int)me->GetDisplayPosition()[1];
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->GetButton())
    {
    case BS_LeftButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
      break;
    case BS_MidButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
      break;
    case BS_RightButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
      break;
    default:
      return;
    }
  }
}

void mitk::VtkInteractorCameraController::MouseReleaseEvent(mitk::MouseEvent *me)
{
  MITK_INFO<<"Called MouseReleaseEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
  if(m_VtkInteractor)
  {
    if (!m_VtkInteractor->GetEnabled())
    {
      return;
    }
    int ctrl  = me->GetButtonState() & BS_ControlButton;
    int shift = me->GetButtonState() & BS_ShiftButton;
    int xp = (int)me->GetDisplayPosition()[0];
    int yp = (int)me->GetDisplayPosition()[1];
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->GetButton())
    {
    case BS_LeftButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
      break;
    case BS_MidButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
      break;
    case BS_RightButton:
      m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
      break;
    default:
      return;
    }
  }
}

void mitk::VtkInteractorCameraController::MouseMoveEvent(mitk::MouseEvent *me)
{
  MITK_INFO<<"Called MouseMoveEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
  if(m_VtkInteractor)
  {
    if (!m_VtkInteractor->GetEnabled())
    {
      return;
    }
    int ctrl  = me->GetButtonState() & BS_ControlButton;
    int shift = me->GetButtonState() & BS_ShiftButton;
    int xp = (int)me->GetDisplayPosition()[0];
    int yp = (int)me->GetDisplayPosition()[1];
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
    m_VtkInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
  }
}

void mitk::VtkInteractorCameraController::KeyPressEvent(mitk::KeyEvent *ke)
{
    MITK_INFO<<"Called KeyPressEvent() in VtkInteractorCameraController. Seems to be obsolete after QVTK restructuring (bug #1503, bug #954)"<<std::endl;
  if(m_VtkInteractor)
  {
    if (!m_VtkInteractor->GetEnabled())
    {
      return;
    }
    int ctrl  = ke->GetButtonState() & BS_ControlButton;
    int shift = ke->GetButtonState() & BS_ShiftButton;
    Point2D p(ke->GetDisplayPosition());
    m_VtkInteractor->SetEventInformationFlipY(static_cast<int>(p[0]), static_cast<int>(p[1]), ctrl, shift, tolower(ke->GetText()[0]), 1, ke->GetText());
    m_VtkInteractor->InvokeEvent(vtkCommand::KeyPressEvent, NULL);
    m_VtkInteractor->InvokeEvent(vtkCommand::CharEvent, NULL);
  }
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
