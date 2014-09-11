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

mitk::USUIActivator::USUIActivator()
  : m_CustomWidgetFactory(0), m_CustomVideoDeviceWidget(0)
{
}

mitk::USUIActivator::~USUIActivator()
{
  delete m_CustomWidgetFactory;
  delete m_CustomVideoDeviceWidget;

  if ( m_ServiceRegistration ) { m_ServiceRegistration.Unregister(); }
}

void mitk::USUIActivator::Load(us::ModuleContext* context)
{
  // create a custom video device widget, which will be used as
  // a prototype for the custom widget factory
  if ( ! m_CustomVideoDeviceWidget )
  {
    m_CustomVideoDeviceWidget = new QmitkUSControlsCustomVideoDeviceWidget();
  }

  // create a factory for custom widgets, using the video device
  // widget as a prototype
  if ( ! m_CustomWidgetFactory )
  {
    m_CustomWidgetFactory = new mitk::USUICustomWidgetFactory(m_CustomVideoDeviceWidget);
  }

  // register the custom widget factory as a microservice
  m_ServiceRegistration = m_CustomWidgetFactory->RegisterService(context);
}

void mitk::USUIActivator::Unload(us::ModuleContext* /*context*/)
{
  m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;

  delete m_CustomWidgetFactory;
  m_CustomWidgetFactory = 0;

  delete m_CustomVideoDeviceWidget;
  m_CustomVideoDeviceWidget = 0;
}
