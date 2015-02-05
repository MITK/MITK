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

#ifndef QmitkIGTLDeviceSetupConnectionWidget_H
#define QmitkIGTLDeviceSetupConnectionWidget_H

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
#include "ui_QmitkIGTLDeviceSetupConnectionWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to setup the connection of an
  * OpenIGTLink device.
  *
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLDeviceSetupConnectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    /**
     * \brief Initializes the widget with the given device.
     *
     * The old device is
     * dropped, so be careful, if the source is not saved somewhere else it might
     * be lost. You might want to ask the user if he wants to save the changes
     * before calling this method.
     * \param device The widget will be initialized corresponding to the state of
     * this device.
     */
    void Initialize(mitk::IGTLDevice::Pointer device);

    QmitkIGTLDeviceSetupConnectionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkIGTLDeviceSetupConnectionWidget();

//    /**
//    * \brief Is called when the current device received a message
//    */
//    void OnMessageReceived(itk::Object* caller, const itk::EventObject&);

//    /**
//     * \brief Is called when the current device received a command
//    */
//    void OnCommandReceived(itk::Object* caller, const itk::EventObject&);

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
    * \brief Is called when the current device received a message
    */
    void OnMessageReceived();

    /**
     * \brief Is called when the current device received a command
    */
    void OnCommandReceived();

  protected slots:

    void OnConnect();
    void OnPortChanged();
    void OnHostnameChanged();

    /**
     * \brief Enables/Disables the buffering of incoming messages
     */
    void OnBufferIncomingMessages(int state);

  protected:

    /**
     * \brief Adapts the GUI to the state of the device
     */
    void AdaptGUIToState();

    /**
     * \brief Calls AdaptGUIToState()
     */
    void OnDeviceStateChanged();

    /** \brief Creation of the connections */
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkIGTLDeviceSetupConnectionWidgetControls* m_Controls;

    /** @brief holds the OpenIGTLink device */
    mitk::IGTLDevice::Pointer m_IGTLDevice;

    /** @brief flag to indicate if the IGTL device is a client or a server */
    bool m_IsClient;

    unsigned long m_MessageReceivedObserverTag;
    unsigned long m_CommandReceivedObserverTag;
    unsigned long m_LostConnectionObserverTag;
    unsigned long m_NewConnectionObserverTag;
    unsigned long m_StateModifiedObserverTag;

    //############## private help methods #######################
    void DisableSourceControls();
//    void EnableSourceControls();
};
#endif
