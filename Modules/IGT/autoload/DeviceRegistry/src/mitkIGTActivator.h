/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTActivator_h
#define mitkIGTActivator_h

#include <usModuleActivator.h>
#include <mitkTrackingDeviceTypeCollection.h>

namespace mitk
{

  /** Documentation:
  *   \brief  The load function of this class is executed every time, the module is loaded.
  *           Attention: no static variables of any class in IGT Module are initialized at this moment!
  *   \ingroup IGT
  */
  class IGTActivator : public us::ModuleActivator
  {
    public:
      IGTActivator();
      ~IGTActivator() override;
      void Load(us::ModuleContext*) override;
      void Unload(us::ModuleContext*) override;

    private:
      TrackingDeviceTypeCollection m_DeviceTypeCollection;

  };
}

#endif
