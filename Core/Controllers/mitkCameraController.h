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
    /** Standard class typedefs. */
    //##ModelId=3E3AE7F4026B
    typedef CameraController               Self;
    //##ModelId=3E3AE7F40289
    typedef BaseController                 Superclass;
    //##ModelId=3E3AE7F402A7
    typedef itk::SmartPointer<Self>        Pointer;
    //##ModelId=3E3AE7F402C5
    typedef itk::SmartPointer<const Self>  ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(CameraController,BaseController);
protected:
    //##ModelId=3E3AE7440312
    CameraController();

    //##ModelId=3E3AE75803CF
    virtual ~CameraController();

};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
