/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H
#define BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H

#include "berryConfigurationElementHandle.h"

namespace berry {

class ThirdLevelConfigurationElementHandle : public ConfigurationElementHandle
{

public:

  berryObjectMacro(berry::ThirdLevelConfigurationElementHandle);

  ThirdLevelConfigurationElementHandle(const SmartPointer<const IObjectManager>& objectManager, int id);
  ThirdLevelConfigurationElementHandle(const IObjectManager* objectManager, int id);

  using ConfigurationElementHandle::GetChildren;
  QList<SmartPointer<IConfigurationElement> > GetChildren() const override;

protected:

  SmartPointer<ConfigurationElement> GetConfigurationElement() const override;

};

}

#endif // BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H
