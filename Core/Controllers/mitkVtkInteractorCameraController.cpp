#include "mitkVtkInteractorCameraController.h"
#include "mitkVtkQRenderWindowInteractor.h"

#include <vtkInteractorStyleTrackballCamera.h> 

//##ModelId=3E3AE7440312
mitk::VtkInteractorCameraController::VtkInteractorCameraController() : m_VtkInteractor(NULL) 
{
    m_VtkInteractor = mitk::VtkQRenderWindowInteractor::New();
    m_VtkInteractor->SetInteractorStyle(vtkInteractorStyleTrackballCamera::New());
}

//##ModelId=3E3AE75803CF
mitk::VtkInteractorCameraController::~VtkInteractorCameraController()
{
}

void mitk::VtkInteractorCameraController::Resize(int w, int h)
{
    if(m_VtkInteractor)
        m_VtkInteractor->SetSize(w, h);
}

void mitk::VtkInteractorCameraController::MousePressEvent(mitk::MouseEvent *me)
{
    if(m_VtkInteractor)
        m_VtkInteractor->MousePressEvent(me);
//    if (!this->Enabled) 
//    {
//        return;
//    }
//    if((me->button() & mitk::ButtonState::LeftButton) == 0)
//        return;
////    SetCapture(wnd);
//    this->InteractorStyle->OnLeftButtonDown(me->state() & ControlButton, 
//        me->state() & mitk::ShiftButton, 
//        me->x(), this->Size[1] - me->y() - 1);
}

void mitk::VtkInteractorCameraController::MouseReleaseEvent(mitk::MouseEvent *me)
{
    if(m_VtkInteractor)
        m_VtkInteractor->MouseReleaseEvent(me);
}

void mitk::VtkInteractorCameraController::MouseMoveEvent(mitk::MouseEvent *me)
{
    if(m_VtkInteractor)
        m_VtkInteractor->MouseMoveEvent(me);
}

void mitk::VtkInteractorCameraController::KeyPressEvent(mitk::KeyEvent *ke)
{
    if(m_VtkInteractor)
        m_VtkInteractor->KeyPressEvent(ke);
}

bool mitk::VtkInteractorCameraController::AddRenderer(mitk::BaseRenderer* renderer)
{
    bool res = CameraController::AddRenderer(renderer);

    if(res)
    {
        m_VtkInteractor->SetMitkRenderer(renderer);
        m_VtkInteractor->Initialize();
    }

    return res;
}

void mitk::VtkInteractorCameraController::SetRenderWindow(vtkRenderWindow* aren)
{
    m_VtkInteractor->SetRenderWindow(aren);
}