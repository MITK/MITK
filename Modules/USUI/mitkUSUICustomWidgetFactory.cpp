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

#include "mitkUSUICustomWidgetFactory.h"

#include "QmitkUSAbstractCustomWidget.h"

#include <usModuleContext.h>

mitk::USUICustomWidgetFactory::USUICustomWidgetFactory(QmitkUSAbstractCustomWidget* prototype)
    : m_Prototype(prototype)
{
}

us::ServiceRegistration<QmitkUSAbstractCustomWidget> mitk::USUICustomWidgetFactory::RegisterService(us::ModuleContext* context)
{
  return context->RegisterService<QmitkUSAbstractCustomWidget>(m_Prototype, m_Prototype->GetServiceProperties());
}

us::InterfaceMap mitk::USUICustomWidgetFactory::GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/)
{
  // clone the prototype for returning a uniqe instance
  return us::MakeInterfaceMap<QmitkUSAbstractCustomWidget>(m_Prototype->Clone());
}

void mitk::USUICustomWidgetFactory::UngetService(us::Module*, const us::ServiceRegistrationBase&, const us::InterfaceMap& service)
{
  // just delete the given service
  delete us::ExtractInterface<QmitkUSAbstractCustomWidget>(service);
}
