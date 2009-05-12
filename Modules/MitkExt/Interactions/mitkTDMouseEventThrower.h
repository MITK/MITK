/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
