#ifndef CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "BaseController.h"

namespace mitk {

//##ModelId=3DF8BCFD014C
//##Documentation
//## Subclass of BaseController. Controls the camera used by the associated
//## BaseRenderer.
class CameraController : public BaseController
{
public:
    mitkClassMacro(CameraController, BaseController);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    virtual void Resize(int w, int h);

    virtual void MousePressEvent(mitk::MouseEvent*);
    virtual void MouseReleaseEvent(mitk::MouseEvent*);
    virtual void MouseMoveEvent(mitk::MouseEvent*);
    virtual void KeyPressEvent(mitk::KeyEvent*);
protected:
    //##ModelId=3E3AE7440312
    CameraController();

    //##ModelId=3E3AE75803CF
    virtual ~CameraController();

};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
