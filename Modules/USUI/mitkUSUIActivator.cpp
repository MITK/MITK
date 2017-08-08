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

#include "mitkUSUIActivator.h"
#include "mitkUSUICustomWidgetFactory.h"
#include "QmitkUSControlsCustomVideoDeviceWidget.h"
#include "QmitkUSControlsCustomDiPhASDeviceWidget.h"

mitk::USUIActivator::USUIActivator()
    : m_CustomWidgetFactory(0), m_CustomVideoDeviceWidget(0), m_CustomDiPhASDeviceWidget(0)
{
}

mitk::USUIActivator::~USUIActivator()
{
  for(auto i : m_CustomWidgetFactory)
    delete i;
  delete m_CustomVideoDeviceWidget;

  if ( m_ServiceRegistration ) { m_ServiceRegistration.Unregister(); }
}

void mitk::USUIActivator::Load(us::ModuleContext* context)
{
  // create a custom video device widget, which will be used as
  // a prototype for the custom widget factory

  if (!m_CustomVideoDeviceWidget)
  {
    m_CustomVideoDeviceWidget = new QmitkUSControlsCustomVideoDeviceWidget();
  }

  // create a custom DiPhAS device widget, which will be used as
  // a prototype for the custom widget factory

  if (!m_CustomDiPhASDeviceWidget)
  {
    m_CustomDiPhASDeviceWidget = new QmitkUSControlsCustomDiPhASDeviceWidget();
  }

  // create a factory for custom widgets, using the video device
  // widget as a prototype
  m_CustomWidgetFactory.push_back(new mitk::USUICustomWidgetFactory(m_CustomVideoDeviceWidget));

  // create a factory for custom widgets, using the DiPhAS device
  // widget as a prototype
  m_CustomWidgetFactory.push_back(new mitk::USUICustomWidgetFactory(m_CustomDiPhASDeviceWidget));

  // register the custom widget factories as a microservice
  for (auto i : m_CustomWidgetFactory)
    m_ServiceRegistration = i->RegisterService(context);
}

void mitk::USUIActivator::Unload(us::ModuleContext* /*context*/)
{
  m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;

  for (auto i : m_CustomWidgetFactory)
    delete i;

  m_CustomWidgetFactory.clear();

  delete m_CustomVideoDeviceWidget;
  m_CustomVideoDeviceWidget = 0;
}
