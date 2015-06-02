/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H
#define BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H

#include "berryConfigurationElementHandle.h"

namespace berry {

class ThirdLevelConfigurationElementHandle : public ConfigurationElementHandle
{

public:

  berryObjectMacro(berry::ThirdLevelConfigurationElementHandle)

  ThirdLevelConfigurationElementHandle(const SmartPointer<const IObjectManager>& objectManager, int id);
  ThirdLevelConfigurationElementHandle(const IObjectManager* objectManager, int id);

  using ConfigurationElementHandle::GetChildren;
  QList<SmartPointer<IConfigurationElement> > GetChildren() const override;

protected:

  SmartPointer<ConfigurationElement> GetConfigurationElement() const override;

};

}

#endif // BERRYTHIRDLEVELCONFIGURATIONELEMENTHANDLE_H
