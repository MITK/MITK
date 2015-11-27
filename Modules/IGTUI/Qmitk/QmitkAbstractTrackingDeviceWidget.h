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

#include "QmitkTrackingDeviceConfigurationWidgetConnectionWorker.h";

//itk headers

/** Documentation:
 *   \brief Simple and fast access to a pre-configured TrackingDeviceSource.
 *
 *   This widget creates a fully configured, connected and started TrackingDeviceSource.
 *   Clicking "Connect" requires to specify a NavigationToolStorage that holds all tools to be used
 *   in the application. Corresponding surfaces are added to the DataStorage that has to be set for
 *   the widget.
 *
 *   Inputs: DataStorage
 *   Outputs: TrackingDeviceSource, NavigationToolStorage
 *   Signals: TrackingDeviceConnected, TrackingDeviceDisconnected
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkAbstractTrackingDeviceWidget : public QWidget
{
  Q_OBJECT;

public:
  static const std::string VIEW_ID;

  QmitkAbstractTrackingDeviceWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

  virtual ~QmitkAbstractTrackingDeviceWidget();

signals:
  /* @brief This signal is sent if a progress which was started before ends.*/
  void ProgressFinished();
  /* @brief This signal is sent if an internal thread (caused by an action of the user) was started. The widget is blocked until the progress ends.*/
  void ProgressStarted();

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
  virtual void CreateQtPartControl(QWidget *parent){};

  QmitkTrackingDeviceConfigurationWidgetConnectionWorker* m_TestConnectionWorker;
  QThread* m_TestConnectionWorkerThread;

public:
  virtual void ResetOutput(){};
  virtual void AddOutput(std::string s){};
  virtual mitk::TrackingDevice::Pointer ConstructTrackingDevice() = 0;

  virtual void StoreUISettings(){};
  virtual void LoadUISettings(){};
  virtual bool IsDeviceInstalled(){ return true; }

  std::string m_ErrorMessage; ///< current problem description
};

#endif
