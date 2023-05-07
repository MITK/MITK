/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTLDeviceSourceManagementWidget_h
#define QmitkIGTLDeviceSourceManagementWidget_h

//QT headers
#include <QWidget>
#include <QTimer>

//mitk headers
#include "MitkOpenIGTLinkUIExports.h"
#include "mitkIGTLDeviceSource.h"
#include "mitkIGTLClient.h"
#include "mitkDataStorage.h"

//itk
#include <itkCommand.h>

//ui header
#include "ui_QmitkIGTLDeviceSourceManagementWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to manage OpenIGTLink Device
  *       Sources and OpenIGTLink Devices.
  *
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLDeviceSourceManagementWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    /** Loads a source to the widget. The old source is dropped, so be careful,
     *  if the source is not saved somewhere else it might be lost. You might
     *  want to ask the user if he wants to save the changes before calling this
     *  method.
     *  @param sourceToLoad This source will be loaded and might be modified
     *  by the user.
     */
    void LoadSource(mitk::IGTLDeviceSource::Pointer sourceToLoad);

    QmitkIGTLDeviceSourceManagementWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkIGTLDeviceSourceManagementWidget() override;


  protected slots:
     void OnSendMessage();

     /**
     * \brief Is called when the current device received a message
     */
     void OnMessageReceived();

     /**
     * \brief Is called when the current device received a command
     */
     void OnCommandReceived();

     /**
     * \brief Is called when the current device lost a connection to one of its
     * sockets
     */
     void OnLostConnection();

     /**
     * \brief Is called when the current device connected to another device
     */
     void OnNewConnection();
     /**
     * \brief Adapts the GUI to the state of the device
     */
     void AdaptGUIToState();

   signals:
     /**
     * \brief used for thread seperation, the worker thread must not call AdaptGUIToState directly
     * QT signals are thread safe and seperate the threads
     */
     void AdaptGUIToStateSignal();

  protected:
    /**
     * \brief Calls AdaptGUIToState()
     */
    void OnDeviceStateChanged();

    /// \brief Fills the commands combo box with available commands
    void FillCommandsComboBox();

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkIGTLDeviceSourceManagementWidgetControls* m_Controls;

    /** @brief holds the OpenIGTLink device */
    mitk::IGTLDevice::Pointer m_IGTLDevice;

    /** @brief holds the IGTLDeviceSource we are working with. */
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;

    /** @brief flag to indicate if the IGTL device is a client or a server */
    bool m_IsClient;

    unsigned long m_MessageReceivedObserverTag;
    unsigned long m_CommandReceivedObserverTag;
    unsigned long m_LostConnectionObserverTag;
    unsigned long m_NewConnectionObserverTag;
    unsigned long m_StateModifiedObserverTag;

    //############## private help methods #######################
    void DisableSourceControls();
};
#endif
