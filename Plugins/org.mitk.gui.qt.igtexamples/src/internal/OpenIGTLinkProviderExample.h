/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef OpenIGTLinkProviderExample_h
#define OpenIGTLinkProviderExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QmitkIGTLStreamingConnector.h>

#include "ui_OpenIGTLinkProviderExampleControls.h"
#include "mitkIGTLServer.h"
#include "mitkIGTLMessageProvider.h"
#include "mitkNavigationDataToIGTLMessageFilter.h"
#include "mitkNavigationDataSequentialPlayer.h"
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
    ~OpenIGTLinkProviderExample() override;

    static const std::string VIEW_ID;

  protected slots:

    void Start();
    void OnOpenFile();
    void UpdateVisualization();


  protected:

    void CreateQtPartControl(QWidget *parent) override;

    void SetFocus() override;

    void CreatePipeline();
    void DestroyPipeline();

    void ResizeBoundingBox();

    Ui::OpenIGTLinkProviderExampleControls m_Controls;
    mitk::IGTLServer::Pointer m_IGTLServer;
    mitk::IGTLMessageProvider::Pointer m_IGTLMessageProvider;
    mitk::NavigationDataToIGTLMessageFilter::Pointer m_NavDataToIGTLMsgFilter;
    mitk::NavigationDataSequentialPlayer::Pointer m_NavDataPlayer;
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_NavDataVisualizer;
    QList<mitk::DataNode::Pointer> m_DemoNodes;
    mitk::NavigationDataSet::Pointer m_NavDataSet;
    QmitkIGTLStreamingConnector m_StreamingConnector;

    QTimer m_VisualizerTimer;
};

#endif // OpenIGTLinkProviderExample_h
