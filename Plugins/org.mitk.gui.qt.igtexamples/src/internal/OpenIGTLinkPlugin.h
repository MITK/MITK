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

#ifndef OpenIGTLinkPlugin_h
#define OpenIGTLinkPlugin_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_OpenIGTLinkPluginControls.h"

#include "qtimer.h"

//OIGTL
#include "mitkIGTLClient.h"
#include "mitkIGTL2DImageDeviceSource.h"
#include "mitkIGTL3DImageDeviceSource.h"
#include "mitkIGTLTrackingDataDeviceSource.h"

//mitk
#include "mitkNavigationDataObjectVisualizationFilter.h"
#include "mitkIGTLMessageToNavigationDataFilter.h"
#include "mitkIGTLMessageToUSImageFilter.h"
#include <mitkSurface.h>
#include <mitkDataNode.h>

//vtk
#include <vtkSphereSource.h>

/**
  \brief OpenIGTLinkPlugin

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
  */
class OpenIGTLinkPlugin : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  protected slots:

  void ConnectButtonClicked();

  void ReceivingButtonClicked();

  void UpdatePipeline();

protected:

  enum State{
    IDLE, CONNECTED, RECEIVING
  };

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::OpenIGTLinkPluginControls m_Controls;

  mitk::IGTL2DImageDeviceSource::Pointer m_IGTL2DImageDeviceSource;
  mitk::IGTL3DImageDeviceSource::Pointer m_IGTL3DImageDeviceSource;
  mitk::IGTLTrackingDataDeviceSource::Pointer m_IGTLTransformDeviceSource;

  mitk::IGTLClient::Pointer m_IGTLClient;

  QTimer m_Timer;

  mitk::IGTLMessageToNavigationDataFilter::Pointer m_IGTLMessageToNavigationDataFilter;
  mitk::NavigationDataObjectVisualizationFilter::Pointer m_NavigationDataObjectVisualizationFilter;
  mitk::IGTLMessageToUSImageFilter::Pointer m_ImageFilter3D;
  mitk::IGTLMessageToUSImageFilter::Pointer m_ImageFilter2D;

  mitk::DataNode::Pointer m_Image2dNode;

private:

  void StateChanged(State newState);

  State m_State;
};

#endif // OpenIGTLinkPlugin_h
