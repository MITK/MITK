#include "mitkVtkInteractorCameraController.h"
#include "mitkVtkQRenderWindowInteractor.h"

//#include <vtkInteractorStyleTrackball.h> 

//##ModelId=3E6D600F009A
mitk::VtkInteractorCameraController::VtkInteractorCameraController() : m_VtkInteractor(NULL) 
{
    m_VtkInteractor = mitk::VtkQRenderWindowInteractor::New();
    //vtkInteractorStyleTrackball* iswitch = vtkInteractorStyleTrackball::New();
    //iswitch->SetCurrentStyleToTrackballCamera();
    //m_VtkInteractor->SetInteractorStyle(iswitch);
    //iswitch->Delete();
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

//##ModelId=3E6D600F007D
void mitk::VtkInteractorCameraController::MouseReleaseEvent(mitk::MouseEvent *me)
{
    if(m_VtkInteractor)
        m_VtkInteractor->MouseReleaseEvent(me);
}

//##ModelId=3E6D600F0080
void mitk::VtkInteractorCameraController::MouseMoveEvent(mitk::MouseEvent *me)
{
    if(m_VtkInteractor)
        m_VtkInteractor->MouseMoveEvent(me);
}

//##ModelId=3E6D600F0088
void mitk::VtkInteractorCameraController::KeyPressEvent(mitk::KeyEvent *ke)
{
    if(m_VtkInteractor)
        m_VtkInteractor->KeyPressEvent(ke);
}

//##ModelId=3E6D600F0093
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

//##ModelId=3E6D600F0090
void mitk::VtkInteractorCameraController::SetRenderWindow(vtkRenderWindow* aren)
{
    m_VtkInteractor->SetRenderWindow(aren);
}

vtkRenderWindowInteractor* mitk::VtkInteractorCameraController::GetVtkInteractor()
{
    return m_VtkInteractor;
}