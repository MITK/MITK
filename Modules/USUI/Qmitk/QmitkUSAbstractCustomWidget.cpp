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
  static std::string s = "ork.mitk.services.UltrasoundCustomWidget.deviceClass";
  return s;
}

QmitkUSAbstractCustomWidget::QmitkUSAbstractCustomWidget(QWidget* parent)
  : QWidget(parent), m_PrototypeServiceFactory(0), m_IsClonedForQt(false)
{
}

QmitkUSAbstractCustomWidget::~QmitkUSAbstractCustomWidget()
{
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
