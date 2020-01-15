/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
    ~OpenIGTLinkExample() override;

    static const std::string VIEW_ID;

  protected slots:

    void Start();
    void UpdatePipeline();

  protected:

    void CreateQtPartControl(QWidget *parent) override;

    void SetFocus() override;

    void CreatePipeline();
    void DestroyPipeline();

    void ResizeBoundingBox();

    Ui::OpenIGTLinkExampleControls m_Controls;
    mitk::IGTLClient::Pointer m_IGTLClient;
    mitk::IGTLDeviceSource::Pointer m_IGTLDeviceSource;
    mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMsgToNavDataFilter;
    mitk::NavigationDataObjectVisualizationFilter::Pointer m_VisFilter;
    QList<mitk::DataNode::Pointer> m_DemoNodes;

    QTimer m_Timer;
};

#endif // OpenIGTLinkExample_h
