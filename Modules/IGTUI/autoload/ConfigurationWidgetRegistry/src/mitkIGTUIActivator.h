/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTUIActivator_H
#define mitkIGTUIActivator_H

#include <usModuleActivator.h>
#include <mitkTrackingDeviceWidgetCollection.h>

namespace mitk
{

  /** Documentation:
  *   \brief  The load function of this class is executed every time, the module is loaded.
  *           Attention: don't do any qt initialization during autoload. keep it as simple as possible! And be careful with static variables,
  *                      they might not be initialized yet...
  *   \ingroup IGT
  */
  class IGTUIActivator : public us::ModuleActivator
  {
    public:
      IGTUIActivator();
      ~IGTUIActivator() override;
      void Load(us::ModuleContext*) override;
      void Unload(us::ModuleContext*) override;

    private:
      TrackingDeviceWidgetCollection m_DeviceWidgetCollection;

  };
}

#endif // mitkIGTUIActivator_H
