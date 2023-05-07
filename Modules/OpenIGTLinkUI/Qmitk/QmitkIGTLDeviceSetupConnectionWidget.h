/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTLDeviceSetupConnectionWidget_h
#define QmitkIGTLDeviceSetupConnectionWidget_h

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

    QmitkIGTLDeviceSetupConnectionWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkIGTLDeviceSetupConnectionWidget() override;

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
    * \brief Is called when the current device received a message
    */
    void OnMessageSent();

    /**
     * \brief Is called when the current device received a command
    */
    void OnCommandReceived();

  protected slots:

    void OnConnect();
    void OnPortChanged();
    void OnHostnameChanged();
    void OnUpdateFPSLabel();

    /**
    * \brief Enables/Disables the detailed logging of incoming/outgoing messages
    */
    void OnLogMessageDetailsCheckBoxClicked();

    /**
    * \brief Enables/Disables the buffering of incoming messages
    */
    void OnBufferIncomingMessages(int state);

    /**
     * \brief Enables/Disables the buffering of outgoing messages
     *
     * This can be necessary when the data is faster produced then sent
     */
    void OnBufferOutgoingMessages(int state);

    /**
    * \brief Adapts the GUI to the state of the device
    */
    void AdaptGUIToState();

 signals:
    /**
    * \brief used for thread seperation, the worker thread must not call AdaptGUIToState directly.
    * QT signals are thread safe and seperate the threads
    */
    void AdaptGUIToStateSignal();

  protected:
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

    unsigned long m_MessageSentObserverTag;
    unsigned long m_MessageReceivedObserverTag;
    unsigned long m_CommandReceivedObserverTag;
    unsigned long m_LostConnectionObserverTag;
    unsigned long m_NewConnectionObserverTag;
    unsigned long m_StateModifiedObserverTag;

    /** @brief the number of received frames (messages) since the last fps calculation update
    *
    *   This counter is incremented every time a message is received. When the timer
    *   m_FPSCalculationTimer is fired it is reset to 0 and the number is used to calculate the FPS
    */
    unsigned int m_NumReceivedFramesSinceLastUpdate;

    /** @brief the number of sent frames (messages) since the last fps calculation update
    *
    *   This counter is incremented every time a message is sent. When the timer
    *   m_FPSCalculationTimer is fired it is reset to 0 and the number is used to calculate the FPS
    */
    unsigned int m_NumSentFramesSinceLastUpdate;

    /** @brief the timer used to calculate the frames per second */
    QTimer m_FPSCalculationTimer;

    //############## private help methods #######################
    void DisableSourceControls();
//    void EnableSourceControls();
    void RemoveObserver();
};
#endif
