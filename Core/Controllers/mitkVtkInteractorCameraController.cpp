#include "mitkVtkInteractorCameraController.h"
#include "mitkVtkQRenderWindowInteractor.h"
#include <vtkInteractorStyleSwitch.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include "mitkVtkRenderWindow.h"

//##ModelId=3E6D600F009A
mitk::VtkInteractorCameraController::VtkInteractorCameraController(const char * type) : CameraController(type), m_VtkInteractor(NULL)
{
// todo: init
    m_VtkInteractor = mitk::VtkQRenderWindowInteractor::New();
  vtkInteractorStyleSwitch* interactorswitch = dynamic_cast<vtkInteractorStyleSwitch*>(m_VtkInteractor->GetInteractorStyle());
  if(interactorswitch!=NULL)
    interactorswitch->SetCurrentStyleToTrackballCamera();
}

//##ModelId=3E6D600F009B
mitk::VtkInteractorCameraController::~VtkInteractorCameraController()
{
}

//##ModelId=3E6D600F006B
void mitk::VtkInteractorCameraController::Resize(int w, int h)
{
  if(m_VtkInteractor)
    m_VtkInteractor->SetSize(w, h);
}

//##ModelId=3E6D600F0075
void mitk::VtkInteractorCameraController::MousePressEvent(mitk::MouseEvent *me)
{
  if(m_VtkInteractor) {
    if (!m_VtkInteractor->GetEnabled())
    {
        return;
    }
    int ctrl  = me->GetButtonState() & mitkEnum::ControlButton;
    int shift = me->GetButtonState() & mitkEnum::ShiftButton;
#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
    int xp = (int)me->GetDisplayPosition().x;
    int yp = (int)me->GetDisplayPosition().y;
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
    
    switch (me->GetButton()) {
    case mitkEnum::LeftButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
        break;
    case mitkEnum::MidButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
        break;
    case mitkEnum::RightButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
        break;
    default:
        return;
    }
#else
    if(me->GetButton() & mitkEnum::LeftButton)
    {
        //    SetCapture(wnd);
        m_VtkInteractor->InteractorStyle->OnLeftButtonDown(ctrl, shift,
            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
    }
    else
        if(me->button() & mitkEnum::MidButton)
        {
            //    SetCapture(wnd);
            m_VtkInteractor->InteractorStyle->OnMiddleButtonDown(ctrl, shift,
                me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
        }
        else
            if(me->button() & mitkEnum::RightButton)
            {
                //    SetCapture(wnd);
                m_VtkInteractor->InteractorStyle->OnRightButtonDown(ctrl, shift,
                    me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
            }
#endif
  }
}

//##ModelId=3E6D600F007D
void mitk::VtkInteractorCameraController::MouseReleaseEvent(mitk::MouseEvent *me)
{
  if(m_VtkInteractor) {
     if (!m_VtkInteractor->GetEnabled())
    {
        return;
    }
    int ctrl  = me->GetButtonState() & mitkEnum::ControlButton;
    int shift = me->GetButtonState() & mitkEnum::ShiftButton;
#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
    int xp = (int)me->GetDisplayPosition().x;
    int yp = (int)me->GetDisplayPosition().y;
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
    
    switch (me->GetButton()) {
    case mitkEnum::LeftButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
        break;
    case mitkEnum::MidButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
        break;
    case mitkEnum::RightButton:
        m_VtkInteractor->InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
        break;
    default:
        return;
    }
#else
    if(me->button() & mitkEnum::LeftButton)
    {
        m_VtkInteractor->InteractorStyle->OnLeftButtonUp(ctrl, shift,
            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
        //ReleaseCapture( );
    }
    else
        if(me->button() & mitkEnum::MidButton)
        {
            m_VtkInteractor->InteractorStyle->OnMiddleButtonUp(ctrl, shift,
                me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
            //ReleaseCapture( );
        }
        else
            if(me->button() & mitkEnum::RightButton)
            {
                m_VtkInteractor->InteractorStyle->OnRightButtonUp(ctrl, shift,
                    me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
                //ReleaseCapture( );
            }
#endif
 
  }
}

//##ModelId=3E6D600F0080
void mitk::VtkInteractorCameraController::MouseMoveEvent(mitk::MouseEvent *me)
{
  if(m_VtkInteractor) {
    if (!m_VtkInteractor->GetEnabled())
    {
        return;
    }
    int ctrl  = me->GetButtonState() & mitkEnum::ControlButton;
    int shift = me->GetButtonState() & mitkEnum::ShiftButton;
#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
    int xp = (int)me->GetDisplayPosition().x;
    int yp = (int)me->GetDisplayPosition().y;
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift);
    m_VtkInteractor->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
#else
    if (!m_VtkInteractor->MouseInWindow &&
        ((me->GetDisplayPosition().x >= 0) && (me->GetDisplayPosition().x < m_VtkInteractor->Size[0]) && (me->GetDisplayPosition().y >= 0) && (me->GetDisplayPosition().y < m_VtkInteractor->Size[1])))
    {
        m_VtkInteractor->InteractorStyle->OnEnter(ctrl, shift,
            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
        m_VtkInteractor->MouseInWindow = 1;
    }
    
    if (m_VtkInteractor->MouseInWindow &&
        ((me->GetDisplayPosition().x < 0) || (me->GetDisplayPosition().x >= m_VtkInteractor->Size[0]) || (me->GetDisplayPosition().y < 0) || (me->GetDisplayPosition().y >= m_VtkInteractor->Size[1])))
    {
        m_VtkInteractor->InteractorStyle->OnLeave(ctrl, shift,
            me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
        m_VtkInteractor->MouseInWindow = 0;
    }
    
    m_VtkInteractor->InteractorStyle->OnMouseMove(ctrl, shift,
        me->GetDisplayPosition().x, m_VtkInteractor->Size[1] - me->GetDisplayPosition().y - 1);
#endif
  
  }
}

//##ModelId=3E6D600F0088
void mitk::VtkInteractorCameraController::KeyPressEvent(mitk::KeyEvent *ke)
{
  if(m_VtkInteractor) {
    if (!m_VtkInteractor->GetEnabled())
    {
        return;
    }
    int ctrl  = ke->state() & mitkEnum::ControlButton;
    int shift = ke->state() & mitkEnum::ShiftButton;
#if ((VTK_MAJOR_VERSION>4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=2)))
    int xp = ke->x();
    int yp = ke->y();
    m_VtkInteractor->SetEventInformationFlipY(xp, yp, ctrl, shift, (char) tolower(ke->ascii()), 1, (const char *) ke->text());
    m_VtkInteractor->InvokeEvent(vtkCommand::KeyPressEvent, NULL);
    m_VtkInteractor->InvokeEvent(vtkCommand::CharEvent, NULL);
#else
    m_VtkInteractor->InteractorStyle->OnChar(ctrl, shift, (char)ke->ascii(), ke->count());
#endif
  
  }
  
}

//##ModelId=3E6D600F0093
bool mitk::VtkInteractorCameraController::SetRenderer(mitk::BaseRenderer* renderer)
{
  mitk::VtkQRenderWindowInteractor* windowInteractor = 
    dynamic_cast<mitk::VtkQRenderWindowInteractor*>(m_VtkInteractor);
  if (windowInteractor == NULL) {
    std::cout << "renderwindow is not a mitk::VtkRenderWindow" << std::endl;
  } else {
    windowInteractor->SetMitkRenderer(renderer);
  }
    m_VtkInteractor->Initialize();
  return true;
}

//##ModelId=3E6D600F0090
void mitk::VtkInteractorCameraController::SetRenderWindow(vtkRenderWindow* aren)
{
  m_VtkInteractor->SetRenderWindow(aren);
}

//##ModelId=3EF162760380
vtkRenderWindowInteractor* mitk::VtkInteractorCameraController::GetVtkInteractor()
{
  return m_VtkInteractor;
}

bool mitk::VtkInteractorCameraController::ExecuteAction(int actionId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  return false;
}

