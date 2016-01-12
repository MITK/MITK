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

#ifndef QmitkAbstractTrackingDeviceWidget_H
#define QmitkAbstractTrackingDeviceWidget_H

#include <QWidget>
#include "MitkIGTUIExports.h"
#include "mitkTrackingDevice.h"

#include <qscrollbar.h>
#include <qthread.h>

#include <mitkIPersistenceService.h>

#include "QmitkTrackingDeviceConfigurationWidgetConnectionWorker.h"

//itk headers

/** Documentation:
 *  \brief Abstract class to configure a tracking device.
 *         Inherited widgets should be registered in the Microservice (TrackingDeviceCollectionWidget),
 *         If done so, they will be included in the QmitkTrackingDeviceConfigurationWidget of the Tracking Toolbox.
 *
 *         Each implementation of this class must have a method to construct a tracking Device (ConstructTrackingDevice).
 *         You might want to use own buttons etc., please connect them in CreateConnections / CreateQtPartControl (e.g. see QmitkVitrualTrackerWidget).
 *         You can Load and Store previous settings of your GUI elements (e.g. see QmitkNDIPolarisWidget).
 *         Also, you can add an output textbox to your widget to display information about your device status. It's optional, see e.g. QmitkNDIAuroraWidget.
 *         Some Devices need the information if drivers are installed on your computer. If this is necessary for your device to avoid crashes,
 *         please override IsDeviceInstalled. The default return value is true otherwise.
 *
 *  \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkAbstractTrackingDeviceWidget : public QWidget
{
    Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkAbstractTrackingDeviceWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

    virtual ~QmitkAbstractTrackingDeviceWidget();

    /**
     * \brief Return pointer to copy of the object.
     * Internally use of QmitkUSAbstractCustomWidget::Clone() with additionaly
     * setting an internal flag that the object was really cloned.
    */
    QmitkAbstractTrackingDeviceWidget* CloneForQt(QWidget* parent = 0) const;

  signals:
    void ConnectionTested(bool connected, QString output);

  protected slots:
    void TestConnectionFinished(bool connected, QString output);

    /* @brief This method is called when the user presses the button "test connection". The method will then create a temporary tracking device,
     *        try to open a connection and start tracking. The user can see the result of the connection test on the small output window.
     */
    void TestConnection();

  protected:
    PERSISTENCE_GET_SERVICE_METHOD_MACRO
    /// \brief Creation of the connections
    virtual void CreateConnections();
    virtual void CreateQtPartControl(QWidget*) {}

    QmitkTrackingDeviceConfigurationWidgetConnectionWorker* m_TestConnectionWorker;
    QThread* m_TestConnectionWorkerThread;

    /**
     * \brief Subclass must implement this method to return a pointer to a copy of the object.
    */
    virtual QmitkAbstractTrackingDeviceWidget* Clone(QWidget* parent = 0) const = 0;

  public:
    virtual void ResetOutput() {}
    virtual void AddOutput(std::string) {}
    virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice() = 0;

    virtual void StoreUISettings() {}
    virtual void LoadUISettings() {}
    virtual bool IsDeviceInstalled() { return true; }

    std::string m_ErrorMessage; ///< current problem description
};

#endif
