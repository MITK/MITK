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

#ifndef mitkIGTActivator_H
#define mitkIGTActivator_H

#include <usModuleActivator.h>
#include <mitkTrackingDeviceTypeCollection.h>

#include <memory>

#include "mitkTrackingTypes.h"

namespace mitk {


class IGTExtActivator : public us::ModuleActivator
{
public:
  IGTExtActivator();
  virtual ~IGTExtActivator();
  void Load(us::ModuleContext*context) override;
  void Unload(us::ModuleContext* context) override;

private:

  TrackingDeviceTypeCollection m_DeviceTypeCollection;

};

}

#endif // mitkIGTActivator_H
