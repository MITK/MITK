#ifndef CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722

#include "mitkCommon.h"
#include "BaseController.h"

namespace mitk {

//##ModelId=3DF8BCFD014C
//##Documentation
//## @brief controls the camera used by the associated BaseRenderer
//## @ingroup NavigationControl
//## Subclass of BaseController. Controls the camera used by the associated
//## BaseRenderer.
class CameraController : public BaseController
{
public:
    //##ModelId=3E6D5DD4008F
    mitkClassMacro(CameraController, BaseController);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    //##ModelId=3E6D5DD400A3
    virtual void Resize(int w, int h);

    //##ModelId=3E6D5DD400C1
    virtual void MousePressEvent(mitk::MouseEvent*);
    //##ModelId=3E6D5DD400D5
    virtual void MouseReleaseEvent(mitk::MouseEvent*);
    //##ModelId=3E6D5DD400F3
    virtual void MouseMoveEvent(mitk::MouseEvent*);
    //##ModelId=3E6D5DD40107
    virtual void KeyPressEvent(mitk::KeyEvent*);
protected:
    //##ModelId=3E3AE7440312
    CameraController();

    //##ModelId=3E3AE75803CF
    virtual ~CameraController();

};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
