#ifndef BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3
#define BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "Navigator.h"
#include "Stepper.h"

namespace mitk {

//##ModelId=3DF8F7EA01DA
class BaseController : public itk::Object
{
  public:
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
    //##ModelId=3DF8C0140176
    Stepper m_Time;
    //##ModelId=3E189CAA0265
    Stepper m_Slice;




};

} // namespace mitk



#endif /* BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3 */
