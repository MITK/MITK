#ifndef MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "CameraController.h"
#include "vtkRenderWindowInteractor.h"

class vtkRenderWindow;

namespace mitk {

class VtkQRenderWindowInteractor;

class VtkInteractorCameraController : public CameraController
{
public:
    mitkClassMacro(VtkInteractorCameraController, CameraController);

    itkNewMacro(Self);

    virtual void Resize(int w, int h);
    virtual void MousePressEvent(mitk::MouseEvent*);
    virtual void MouseReleaseEvent(mitk::MouseEvent*);
    virtual void MouseMoveEvent(mitk::MouseEvent*);
    virtual void KeyPressEvent(mitk::KeyEvent*);

    virtual void SetRenderWindow(vtkRenderWindow* aren);
    virtual bool AddRenderer(mitk::BaseRenderer* renderer);
protected:
    VtkInteractorCameraController();

    virtual ~VtkInteractorCameraController();

    VtkQRenderWindowInteractor* m_VtkInteractor;
};

} // namespace mitk

#endif /* MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
