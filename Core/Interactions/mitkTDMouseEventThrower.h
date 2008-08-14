
#ifndef TDMouseEventThrower__h__
#define TDMouseEventThrower__h__

#include <mitkVector.h>
#include <iostream>

namespace mitk {

  class TDMouseEventThrower
  {
  public:

    static TDMouseEventThrower* GetInstance();

    void DeviceChange (long device, long keys, long programmableKeys);
    void KeyDown (int keyCode);
    void KeyUp (int keyCode);
    void SensorInput( mitk::Vector3D translation, mitk::Vector3D rotation, mitk::ScalarType angle);

  protected:
    TDMouseEventThrower();
    ~TDMouseEventThrower();
  };

}//namespace

#endif // TDMouseEventThrower__h__
