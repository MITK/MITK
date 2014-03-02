/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
