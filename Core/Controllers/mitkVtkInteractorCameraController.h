#ifndef MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "CameraController.h"
#include "vtkRenderWindowInteractor.h"

class vtkRenderWindow;

namespace mitk {

class VtkQRenderWindowInteractor;

//##ModelId=3E6D600E017F
//##Documentation
//## @brief vtk-based camera controller
//## @ingroup NavigationControl
class VtkInteractorCameraController : public CameraController
{
public:
    //##ModelId=3E6D600F0068
    mitkClassMacro(VtkInteractorCameraController, CameraController);

    itkNewMacro(Self);

    //##ModelId=3E6D600F006B
    virtual void Resize(int w, int h);
    //##ModelId=3E6D600F0075
    virtual void MousePressEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F007D
    virtual void MouseReleaseEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F0080
    virtual void MouseMoveEvent(mitk::MouseEvent*);
    //##ModelId=3E6D600F0088
    virtual void KeyPressEvent(mitk::KeyEvent*);

    //##ModelId=3E6D600F0090
    virtual void SetRenderWindow(vtkRenderWindow* aren);
    //##ModelId=3E6D600F0093
    virtual bool AddRenderer(mitk::BaseRenderer* renderer);
protected:
    //##ModelId=3E6D600F009A
    VtkInteractorCameraController();

    //##ModelId=3E6D600F009B
    virtual ~VtkInteractorCameraController();

    //##ModelId=3E6D600F0060
    VtkQRenderWindowInteractor* m_VtkInteractor;
};

} // namespace mitk

#endif /* MITKVTKINTERACTORCAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
