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

#ifndef QmitkUSAbstractCustomWidget_h
#define QmitkUSAbstractCustomWidget_h

#include <QWidget>

#include <MitkUSUIExports.h>
#include "mitkUSDevice.h"

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>

namespace us {
  class PrototypeServiceFactory;
}

class MitkUSUI_EXPORT QmitkUSAbstractCustomWidget : public QWidget
{
public:
  QmitkUSAbstractCustomWidget(QWidget* parent = 0);
  virtual ~QmitkUSAbstractCustomWidget();

  void SetDevice(mitk::USDevice::Pointer device);
  mitk::USDevice::Pointer GetDevice() const;

  virtual void OnDeviceSet() = 0;

  virtual std::string GetDeviceClass() const = 0;
  virtual QmitkUSAbstractCustomWidget* Clone(QWidget* parent = 0) const = 0;

  QmitkUSAbstractCustomWidget* CloneForQt(QWidget* parent = 0) const;

  us::ServiceRegistration<QmitkUSAbstractCustomWidget> RegisterService(us::ModuleContext* context);
  us::ServiceProperties GetServiceProperties() const;

  static const std::string US_DEVICE_PROPKEY_CLASS;        // class name of corresponding us device object

  void showEvent ( QShowEvent * event );

private:
  //void SetIsClonedForQt();

  mitk::USDevice::Pointer m_Device;

  us::PrototypeServiceFactory*    m_PrototypeServiceFactory;

  bool m_IsClonedForQt;
};


// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(QmitkUSAbstractCustomWidget, "org.mitk.QmitkUSAbstractCustomWidget")

#endif // QmitkUSAbstractCustomWidget_h