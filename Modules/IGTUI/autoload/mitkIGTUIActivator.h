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



#include <memory>

namespace mitk {

  /** Documentation:
  *   \brief  The load function of this class is executed every time, the module is loaded.
  *           Attention: no static variables of any class in IGT Module are initialized at this moment!
  *   \ingroup IGT
  */
class IGTUIActivator : public us::ModuleActivator
{
public:
  IGTUIActivator();
  virtual ~IGTUIActivator();
  void Load(us::ModuleContext*context) override;
  void Unload(us::ModuleContext* context) override;

private:

  TrackingDeviceWidgetCollection m_DeviceWidgetCollection;

};

}

#endif // mitkIGTUIActivator_H
