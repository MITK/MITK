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


#ifndef OpenIGTLink_h
#define OpenIGTLink_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_OpenIGTLinkControls.h"
#include "mitkIGTLClient.h"
#include "mitkIGTLDeviceSource.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkIGTLMessageToNavigationDataFilter.h"

#include "qtimer.h"

/**
  \brief OpenIGTLink

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OpenIGTLink : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

  protected slots:

    void ConnectWithServer();
    void ChangePort();
    void ChangeIP();

    void Start();
    void UpdatePipeline();


  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    void CreatePipeline();
    void DestroyPipeline();

    Ui::OpenIGTLinkControls m_Controls;
    mitk::IGTLClient::Pointer m_IGTLClient;
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;
    mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMsgToNavDataFilter;
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_VisFilter;
    mitk::DataNode::Pointer m_DemoNode;

    QTimer m_Timer;
};

#endif // OpenIGTLink_h
