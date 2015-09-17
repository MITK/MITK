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


#ifndef OpenIGTLinkImageProviderExample_h
#define OpenIGTLinkImageProviderExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <QmitkIGTLStreamingConnector.h>

#include "ui_OpenIGTLinkImageProviderExampleControls.h"
#include "mitkIGTLServer.h"
#include "mitkIGTLMessageProvider.h"
#include "mitkNavigationDataToIGTLMessageFilter.h"
#include "mitkNavigationDataSequentialPlayer.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkImageToIGTLMessageFilter.h"

#include "qtimer.h"

/**
  \brief OpenIGTLinkImageProviderExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class OpenIGTLinkImageProviderExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:
    ~OpenIGTLinkImageProviderExample();

    static const std::string VIEW_ID;

  protected slots:

    void OnStartPipeline();

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    void CreatePipeline();
    void DestroyPipeline();

    Ui::OpenIGTLinkImageProviderExampleControls m_Controls;
    mitk::IGTLServer::Pointer m_IGTLServer;
    mitk::IGTLMessageProvider::Pointer m_IGTLMessageProvider;
    mitk::ImageToIGTLMessageFilter::Pointer m_ImageToIGTLMsgFilter;
    mitk::ImageSource::Pointer m_Source;
};

#endif // OpenIGTLinkImageProviderExample_h
