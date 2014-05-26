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

#ifndef MITKUSUICUSTOMWIDGETFACTORY_H
#define MITKUSUICUSTOMWIDGETFACTORY_H

#include <usPrototypeServiceFactory.h>

class QmitkUSAbstractCustomWidget;

namespace us {
class ModuleContext;
}

namespace mitk {
/**
  * \brief Prototype service factory for creating unique instances of QmitUSAbstractCustomWidget.
  */
class USUICustomWidgetFactory : public us::PrototypeServiceFactory {
public:
  USUICustomWidgetFactory(QmitkUSAbstractCustomWidget* prototype);

  /**
    * \brief Registers this factory in the given module context.
    */
  us::ServiceRegistration<QmitkUSAbstractCustomWidget> RegisterService(us::ModuleContext* context);

  us::InterfaceMap GetService(us::Module* /*module*/, const us::ServiceRegistrationBase& /*registration*/);
  void UngetService(us::Module*, const us::ServiceRegistrationBase&, const us::InterfaceMap& service);

private:
    QmitkUSAbstractCustomWidget* const m_Prototype;
};
} // namespace mitk

#endif // MITKUSUICUSTOMWIDGETFACTORY_H
