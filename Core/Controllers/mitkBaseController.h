#ifndef BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3
#define BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "Navigator.h"
#include "Stepper.h"

namespace mitk {

//##ModelId=3DF8F7EA01DA
//##Documentation
//## Tells the render (subclass of BaseRenderer) which slice (subclass
//## SliceNavigationController) or from which direction (subclass
//## CameraController) it has to render. Contains two Steppers for stepping
//## through the slices or through different camera views (e.g., for the
//## creation of a movie around the data), respectively, and through time, if
//## there is 3D+t data.
class BaseController : public itk::Object
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E3AE5ED012E
	typedef BaseController           Self;
    //##ModelId=3E3AE5ED0157
	typedef itk::Object              Superclass;
    //##ModelId=3E3AE5ED016A
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E3AE5ED0174
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(BaseController,itk::Object);

    //##ModelId=3DF8F5CA03D8
    mitk::Stepper &GetSlice();

    //##ModelId=3DF8F61101E3
    mitk::Stepper &GetTime();

    //##ModelId=3DD528260235
    bool RemoveNavigator(mitk::Navigator* navigator);

    //##ModelId=3DD52870008D
    bool RemoveRenderer(mitk::BaseRenderer* renderer);

    //##ModelId=3DD528500222
    bool AddRenderer(mitk::BaseRenderer* renderer);

    //##ModelId=3DD5268F0013
    bool AddNavigator(mitk::Navigator* navigator);

  protected:
    //##ModelId=3E3AE32B0048
    BaseController();

    //##ModelId=3E3AE32B0070
    //##ModelId=3E3AE32B0070
    virtual ~BaseController();

    //##ModelId=3DF8C0140176
    Stepper m_Time;
    //##ModelId=3E189CAA0265
    Stepper m_Slice;
};

} // namespace mitk



#endif /* BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3 */
