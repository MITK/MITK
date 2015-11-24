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

#ifndef QmitkMicronTrackerWidget_H
#define QmitkMicronTrackerWidget_H

#include <QWidget>
#include "MitkIGTUIExports.h"
#include "ui_QmitkMicronTrackerWidget.h"

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
class MITKIGTUI_EXPORT QmitkMicronTrackerWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)

  public:
    static const std::string VIEW_ID;

    QmitkMicronTrackerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkMicronTrackerWidget();

  signals:
    /*!
    \brief signal emitted when TrackingDevice was successfully connected
    */
    void TrackingDeviceConnected();


  protected slots:
    /*!
    \brief Asks the user to specify a tool file and finally connects the TrackingDeviceSource
    */
    void OnConnect();

  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);


    Ui::QmitkMicronTrackerWidget* m_Controls;

    std::string m_ErrorMessage; ///< current problem description
};
#endif
