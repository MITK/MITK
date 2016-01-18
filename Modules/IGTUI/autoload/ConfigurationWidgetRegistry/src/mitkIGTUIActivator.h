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
      virtual ~IGTUIActivator();
      void Load(us::ModuleContext*) override;
      void Unload(us::ModuleContext*) override;

    private:
      TrackingDeviceWidgetCollection m_DeviceWidgetCollection;

  };
}

#endif // mitkIGTUIActivator_H
