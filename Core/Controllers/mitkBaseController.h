#ifndef BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3
#define BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3

#include "mitkCommon.h"
#include "Navigator.h"
#include "Stepper.h"
#include <itkObjectFactory.h>

namespace mitk {

class BaseRenderer;

//##ModelId=3DF8F7EA01DA
//##Documentation
//## @brief Baseclass for renderer slice-/camera-control
//## @ingroup NavigationControl
//## Tells the render (subclass of BaseRenderer) which slice (subclass
//## SliceNavigationController) or from which direction (subclass
//## CameraController) it has to render. Contains two Stepper for stepping
//## through the slices or through different camera views (e.g., for the
//## creation of a movie around the data), respectively, and through time, if
//## there is 3D+t data.
//## @note not yet implemented
class BaseController : public itk::Object
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E6D5DD301B0
    mitkClassMacro(BaseController, itk::Object);

    /** Method for creation through the object factory. */
	itkNewMacro(Self);

    //##ModelId=3DF8F5CA03D8
    //##Documentation
    //## @brief Get the Stepper through the slices
    mitk::Stepper &GetSlice();

    //##ModelId=3DF8F61101E3
    //##Documentation
    //## @brief Get the Stepper through the time
    mitk::Stepper &GetTime();

    //##ModelId=3DD528500222
    //##Documentation
    //## @brief Add a Renderer to control by this BaseController
    //## @note not yet implemented
    virtual bool AddRenderer(mitk::BaseRenderer* renderer);

    //##ModelId=3DD52870008D
    //##Documentation
    //## @brief Remove a Renderer to control by this BaseController
    //## @note not yet implemented
    virtual bool RemoveRenderer(mitk::BaseRenderer* renderer);

    //##ModelId=3DD5268F0013
    //##Documentation
    //## @brief Add a Navigator displaying/changing this BaseController
    //## @note not yet implemented
    virtual bool AddNavigator(mitk::Navigator* navigator);

    //##ModelId=3DD528260235
    //##Documentation
    //## @brief Remove a Navigator displaying/changing this BaseController
    //## @note not yet implemented
    virtual bool RemoveNavigator(mitk::Navigator* navigator);

  protected:
    //##ModelId=3E3AE32B0048
    BaseController();

    //##ModelId=3E3AE32B0070
    virtual ~BaseController();

    //##ModelId=3DF8C0140176
    //## @brief Stepper through the time
    Stepper m_Time;
    //##ModelId=3E189CAA0265
    //## @brief Stepper through the slices
    Stepper m_Slice;
    //BaseRenderer::Pointer m_BaseRenderer;
};

} // namespace mitk



#endif /* BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3 */
