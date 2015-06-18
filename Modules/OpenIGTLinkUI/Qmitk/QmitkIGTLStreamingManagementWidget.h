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

#ifndef QMITKIGTLStreamingMANAGEMENTWIDGET_H
#define QMITKIGTLStreamingMANAGEMENTWIDGET_H

//QT headers
#include <QWidget>
#include <QTimer>

//mitk headers
#include "MitkOpenIGTLinkUIExports.h"
#include "mitkIGTLMessageProvider.h"
#include "mitkIGTLClient.h"
#include "mitkDataStorage.h"

//itk
#include <itkCommand.h>

//ui header
#include "ui_QmitkIGTLStreamingManagementWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to manage the streaming of
  * message sources.
  *
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLStreamingManagementWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    /** Loads a provider to the widget. The old source is dropped, so be careful,
     *  if the source is not saved somewhere else it might be lost. You might
     *  want to ask the user if he wants to save the changes before calling this
     *  method.
     *  @param provider This provider will be loaded and might be modified
     *  by the user.
     */
    void LoadSource(mitk::IGTLMessageProvider::Pointer provider);

    QmitkIGTLStreamingManagementWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkIGTLStreamingManagementWidget();

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


  protected slots:
    void OnStartStreaming();
    void OnStopStreaming();

    /** \brief Is called when a new source is selected.
     *  @param source the newly selected source
     */
    void SourceSelected(mitk::IGTLMessageSource::Pointer source);

    /**
    * \brief Adapts the GUI to the state of the device
    */
    void AdaptGUIToState();

    /**
    * \brief selects the current source and adapts the GUI according to the selection
    */
    void SelectSourceAndAdaptGUI();

 signals:
    /**
    * \brief used for thread seperation, the worker thread must not call AdaptGUIToState directly.
    * QT signals are thread safe and seperate the threads
    */
    void AdaptGUIToStateSignal();
    /**
    * \brief used for thread seperation, the worker thread must not call SelectSourceAndAdaptGUI
    * directly.
    * QT signals are thread safe and seperate the threads
    */
    void SelectSourceAndAdaptGUISignal();

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

    Ui::QmitkIGTLStreamingManagementWidgetControls* m_Controls;

    /** @brief holds the OpenIGTLink device */
    mitk::IGTLDevice::Pointer m_IGTLDevice;

    /** @brief holds the IGTL Message Provider that will send the stream */
    mitk::IGTLMessageProvider::Pointer m_IGTLMsgProvider;

    /** @brief holds the IGTLDeviceSource we are working with. */
    mitk::IGTLMessageSource::Pointer m_IGTLMsgSource;

    /** @brief flag to indicate if the IGTL device is a client or a server */
    bool m_IsClient;

    unsigned long m_MessageReceivedObserverTag;
    unsigned long m_CommandReceivedObserverTag;
    unsigned long m_LostConnectionObserverTag;
    unsigned long m_NewConnectionObserverTag;
    unsigned long m_StateModifiedObserverTag;

    //############## private help methods #######################
    void DisableSourceControls();
    void RemoveObserver();
};
#endif
