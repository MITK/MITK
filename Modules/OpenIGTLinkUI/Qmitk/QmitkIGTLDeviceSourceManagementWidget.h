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

#ifndef QMITKIGTLDeviceSourceMANAGEMENTWIDGET_H
#define QMITKIGTLDeviceSourceMANAGEMENTWIDGET_H

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
class MITK_OPENIGTLINKUI_EXPORT QmitkIGTLDeviceSourceManagementWidget : public QWidget
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

    QmitkIGTLDeviceSourceManagementWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkIGTLDeviceSourceManagementWidget();

    /// \brief Is called when the current device received a message
    void OnMessageReceived(itk::Object* caller, const itk::EventObject&);

    /// \brief Is called when the current device received a command
    void OnCommandReceived(itk::Object* caller, const itk::EventObject&);

  signals:

    /** This signal is emmited if a new source was added by the widget itself,
     *  e.g. because a source was loaded.
     *  @param newSource Holds the new source which was added.
     *  @param sourceName Name of the new source
     */
    void NewSourceAdded(mitk::IGTLDeviceSource::Pointer newSource,
                        std::string sourceName);


  protected slots:

    void OnConnect();
    void OnPortChanged();
    void OnHostnameChanged();
    void OnCommandChanged(const QString& curCommand);

    void OnSendMessage();
    void OnSendCommand();

  protected:
    /// \brief Fills the commands combo box with available commands
    void FillCommandsComboBox();

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkIGTLDeviceSourceManagementWidgetControls* m_Controls;

    /** @brief holds the OpenIGTLink device */
    mitk::IGTLDevice* m_IGTLDevice;

    /** @brief holds the IGTLDeviceSource we are working with. */
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;

    /** @brief shows if we are in edit mode, if not we create new source. */
    bool m_edit;


    igtl::MessageBase::Pointer m_CurrentCommand;

    /** mutex to control access to m_State */
//    itk::FastMutexLock::Pointer m_OutputMutex;
    /** @brief a string stream used for logging */
//    std::stringstream m_Output;
    /** @brief flag to indicate if the output has to be updated */
//    bool m_OutputChanged;


    /** @brief flag to indicate if the IGTL device is a client or a server */
    bool m_IsClient;


    /** @brief a string stream used for logging */
//    QTimer m_UpdateLoggingWindowTimer;

    unsigned long m_MessageReceivedObserverTag;
    unsigned long m_CommandReceivedObserverTag;


    itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget >::Pointer m_MessageReceivedCommand;

    //############## private help methods #######################
    void MessageBox(std::string s);
//    void UpdateToolTable();
    void DisableSourceControls();
    void EnableSourceControls();
};
#endif
