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
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>

#include <usModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
  class ModuleContext;
}

/**
  * \brief Abstract superclass for all custom control widgets of mitk::USDevice classes.
  *
  * The custom control widgets are made available using a us::PrototypeServiceFactory. This means that each
  * concrete subclass should be registered in the microservice by calling
  * QmitkUSAbstractCustomWidget::RegisterService() on an object. The best place for doing this would be in
  * the corresponding module or plugin activator.
  *
  * Afterwards a copy of the registered object can be obtained from the microservice as shown in the example
  * below. Do not forget to call QmitkUSAbstractCustomWidget::CloneForQt() on the object received from the
  * microservice. This is necessary to allow deleting the object as it is necessary in Qt for removing it from
  * a layout.
  *
  *
  * Subclasses must implement three methods:
  * - QmitkUSAbstractCustomWidget::OnDeviceSet() -> should handle initialization when mitk:USDevice was set
  * - QmitkUSAbstractCustomWidget::GetDeviceClass() -> must return device class of corresponding mitk::USDevice
  * - QmitkUSAbstractCustomWidget::Clone() -> must create a copy of the current object
  *
  *
  * The code to use a custom control widget in a plugin can look like this:
  *
  * \code
  * ctkPluginContext* pluginContext = // get the plugig context
  * mitk::USDevice device = // get the ultrasound device
  *
  * // get service references for ultrasound device
  * std::string filter = "(ork.mitk.services.UltrasoundCustomWidget.deviceClass=" + device->GetDeviceClass() + ")";
  * QString interfaceName ( us_service_interface_iid<QmitkUSAbstractCustomWidget>() );
  * QList<ctkServiceReference> serviceRefs = pluginContext->getServiceReferences(interfaceName, QString::fromStdString(filter));
  *
  * if (serviceRefs.size() > 0)
  * {
  *   // get widget from the service and make sure that it is cloned, so that
  *   // it can be deleted if it should be removed from the GUI later
  *   QmitkUSAbstractCustomWidget* widget = pluginContext->getService<QmitkUSAbstractCustomWidget>
  *          (serviceRefs.at(0))->CloneForQt(parentWidget);
  *   // now the widget can be used like any other QWidget
  * }
  * \endcode
  */
class MitkUSUI_EXPORT QmitkUSAbstractCustomWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkUSAbstractCustomWidget(QWidget* parent = 0);
  virtual ~QmitkUSAbstractCustomWidget();

  void SetDevice(mitk::USDevice::Pointer device);
  mitk::USDevice::Pointer GetDevice() const;

  /**
    * \brief Called every time a mitk::USDevice was set with QmitkUSAbstractCustomWidget::SetDevice().
    * A sublcass can implement this function to handle initialiation actions
    * necessary when a device was set.
    */
  virtual void OnDeviceSet() = 0;

  /**
    * \brief Subclass must implement this method to return device class of corresponding mitk::USDevice.
    *
    * \return same value as mitk::USDevice::GetDeviceClass() of the corresponding mitk::USDevice
    */
  virtual std::string GetDeviceClass() const = 0;

  /**
    * \brief Subclass must implement this method to return a pointer to a copy of the object.
    */
  virtual QmitkUSAbstractCustomWidget* Clone(QWidget* parent = 0) const = 0;

  /**
    * \brief Method for initializing the Qt stuff of the widget (setupUI, connect).
    * This method will be called in CloneForQt() and has to be implemented by concrete
    * subclasses.
    * \warning All Qt initialization stuff belongs into this method rather than in the constructor.
    */
  virtual void Initialize() = 0;

  /**
    * \brief Return pointer to copy of the object.
    * Internally use of QmitkUSAbstractCustomWidget::Clone() with additionaly
    * setting an internal flag that the object was really cloned.
    */
  QmitkUSAbstractCustomWidget* CloneForQt(QWidget* parent = 0) const;

  /**
    * \brief Returns the properties of the micro service.
    * Properties consist of just the device class of the corresponding
    * mitk::USDevice.
    */
  us::ServiceProperties GetServiceProperties() const;

  /**
    * \brief Overwritten Qt even method.
    * It is checked if the object was cloned with
    * QmitkUSAbstractCustomWidget::CloneForQt() before. An exception is thrown
    * if not. This is done, because using the object from micro service directly
    * in Qt without cloning it first can cause problems after Qt deleted the
    * object.
    *
    * \throws mitk::Exception
    */
  void showEvent ( QShowEvent * event );

  /**
    * \brief Property key for the class name of corresponding us device object.
    */
  static std::string US_DEVICE_PROPKEY_CLASS();

private:

  mitk::USDevice::Pointer         m_Device;
  us::PrototypeServiceFactory*    m_PrototypeServiceFactory;
  bool                            m_IsClonedForQt;
};

// This is the microservice declaration. Do not meddle!
MITK_DECLARE_SERVICE_INTERFACE(QmitkUSAbstractCustomWidget, "org.mitk.QmitkUSAbstractCustomWidget")

#endif // QmitkUSAbstractCustomWidget_h
