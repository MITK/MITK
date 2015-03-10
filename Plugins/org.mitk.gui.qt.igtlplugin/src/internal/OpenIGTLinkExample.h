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


#ifndef OpenIGTLinkExample_h
#define OpenIGTLinkExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_OpenIGTLinkExampleControls.h"
#include "mitkIGTLClient.h"
#include "mitkIGTLServer.h"
#include "mitkIGTLDeviceSource.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkIGTLMessageToNavigationDataFilter.h"

#include "qtimer.h"

/**
  \brief OpenIGTLinkExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OpenIGTLinkExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:
    ~OpenIGTLinkExample();

    static const std::string VIEW_ID;

  protected slots:

    void Start();
    void UpdatePipeline();


  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    void CreatePipeline();
    void DestroyPipeline();

    Ui::OpenIGTLinkExampleControls m_Controls;
    mitk::IGTLClient::Pointer m_IGTLClient;
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;
    mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMsgToNavDataFilter;
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_VisFilter;
    mitk::DataNode::Pointer m_DemoNodeT1;
    mitk::DataNode::Pointer m_DemoNodeT2;
    mitk::DataNode::Pointer m_DemoNodeT3;

    //REMOVE LATER
//    mitk::IGTLServer::Pointer m_IGTLServer;
//    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource2;

    QTimer m_Timer;
};

#endif // OpenIGTLinkExample_h
