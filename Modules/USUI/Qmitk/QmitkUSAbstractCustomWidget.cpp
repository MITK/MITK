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

#include "QmitkUSAbstractCustomWidget.h"

#include <usPrototypeServiceFactory.h>
#include <usModuleContext.h>

std::string QmitkUSAbstractCustomWidget::US_DEVICE_PROPKEY_CLASS()
{
  static std::string s = "org.mitk.services.UltrasoundCustomWidget.deviceClass";
  return s;
}

QmitkUSAbstractCustomWidget::QmitkUSAbstractCustomWidget(QWidget* parent)
  : QWidget(parent), m_PrototypeServiceFactory(nullptr), m_IsClonedForQt(false)
{

}

QmitkUSAbstractCustomWidget::~QmitkUSAbstractCustomWidget()
{
  this->UnregisterService();
  delete m_PrototypeServiceFactory;
}

void QmitkUSAbstractCustomWidget::SetDevice(mitk::USDevice::Pointer device)
{
  m_Device = device;

  if ( device ) { this->OnDeviceSet(); }
}

mitk::USDevice::Pointer QmitkUSAbstractCustomWidget::GetDevice() const
{
  return m_Device;
}

QmitkUSAbstractCustomWidget* QmitkUSAbstractCustomWidget::CloneForQt(QWidget* parent) const
{
  QmitkUSAbstractCustomWidget* clonedWidget = this->Clone(parent);
  clonedWidget->Initialize(); // initialize the Qt stuff of the widget
  clonedWidget->m_IsClonedForQt = true; // set flag that this object was really cloned
  return clonedWidget;
}

//////////// µS Registration & Properties //////////////

us::ServiceRegistration<QmitkUSAbstractCustomWidget> QmitkUSAbstractCustomWidget::RegisterService(us::ModuleContext *context)
{
  if (m_PrototypeServiceFactory)
    return us::ServiceRegistration<QmitkUSAbstractCustomWidget>();

  if (context == nullptr)
  {
    context = us::GetModuleContext();
  }

  // Define a PrototypeServiceFactory for the abstract super class of all custom control widgets of USUI
  // This factory is used to register the individual widgets as micro services linked to specific US devices
  struct PrototypeFactory : public us::PrototypeServiceFactory
  {
    QmitkUSAbstractCustomWidget *const m_Prototype;

    PrototypeFactory(QmitkUSAbstractCustomWidget *prototype) : m_Prototype(prototype) {}
    us::InterfaceMap GetService(us::Module * /*module*/,
      const us::ServiceRegistrationBase & /*registration*/) override
    {
      return us::MakeInterfaceMap<QmitkUSAbstractCustomWidget>(m_Prototype->Clone());
    }

    void UngetService(us::Module * /*module*/,
      const us::ServiceRegistrationBase & /*registration*/,
      const us::InterfaceMap &service) override
    {
      delete us::ExtractInterface<QmitkUSAbstractCustomWidget>(service);
    }
  };

  m_PrototypeServiceFactory = new PrototypeFactory(this); // instantiate PrototypeFactory with QmitkUSAbstractCustomWidget as prototype
  us::ServiceProperties props = this->GetServiceProperties(); // retrieve the service properties (the device class linked with each custom control widget)
  m_Reg = context->RegisterService<QmitkUSAbstractCustomWidget>(m_PrototypeServiceFactory, props); // register service
  return m_Reg;
}

void QmitkUSAbstractCustomWidget::UnregisterService()
{
  try
  {
    m_Reg.Unregister();
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << e.what();
  }
}

us::ServiceProperties QmitkUSAbstractCustomWidget::GetServiceProperties() const
{
  us::ServiceProperties result;

  result[QmitkUSAbstractCustomWidget::US_DEVICE_PROPKEY_CLASS()] = this->GetDeviceClass();

  return result;
}

void QmitkUSAbstractCustomWidget::showEvent ( QShowEvent * event )
{
  // using object from micro service directly in Qt without cloning it first
  // can cause problems when Qt deletes this object -> throw an exception to
  // show that object should be cloned before
  if ( ! m_IsClonedForQt )
  {
    MITK_ERROR << "Object wasn't cloned with CloneForQt() before using as QWidget.";
    mitkThrow() << "Object wasn't cloned with CloneForQt() before using as QWidget.";
  }

  QWidget::showEvent(event);
}
