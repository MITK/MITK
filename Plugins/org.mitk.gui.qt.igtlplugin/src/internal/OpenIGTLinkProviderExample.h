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


#ifndef OpenIGTLinkProviderExample_h
#define OpenIGTLinkProviderExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QmitkIGTLStreamingConnector.h>

#include "ui_OpenIGTLinkProviderExampleControls.h"
#include "mitkIGTLServer.h"
#include "mitkIGTLMessageProvider.h"
#include "mitkNavigationDataToIGTLMessageFilter.h"
#include "mitkNavigationDataPlayer.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

#include "qtimer.h"

/**
  \brief OpenIGTLinkProviderExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OpenIGTLinkProviderExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:
    ~OpenIGTLinkProviderExample();

    static const std::string VIEW_ID;

  protected slots:

    void Start();
    void OnOpenFile();
    void UpdateVisualization();


  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    void CreatePipeline();
    void DestroyPipeline();

    Ui::OpenIGTLinkProviderExampleControls m_Controls;
    mitk::IGTLServer::Pointer m_IGTLServer;
    mitk::IGTLMessageProvider::Pointer m_IGTLMessageProvider;
    mitk::NavigationDataToIGTLMessageFilter::Pointer m_NavDataToIGTLMsgFilter;
    mitk::NavigationDataPlayer::Pointer m_NavDataPlayer;
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_NavDataVisualizer;
    mitk::DataNode::Pointer m_DemoNodeT1;
    mitk::DataNode::Pointer m_DemoNodeT2;
    mitk::DataNode::Pointer m_DemoNodeT3;
    mitk::NavigationDataSet::Pointer m_NavDataSet;
    QmitkIGTLStreamingConnector m_StreamingConnector;

    QTimer m_VisualizerTimer;
};

#endif // OpenIGTLinkProviderExample_h
