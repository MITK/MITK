/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeVisualizationView.h"

#include <vtkSmartVolumeMapper.h>

#include <mitkImage.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionInitializer.h>
#include <mitkTransferFunctionProperty.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

#include <mitkProperties.h>

const std::string QmitkVolumeVisualizationView::VIEW_ID = "org.mitk.views.volumevisualization";

enum
{
  DEFAULT_RENDERMODE = 0,
  RAYCAST_RENDERMODE = 1,
  GPU_RENDERMODE = 2
};

QmitkVolumeVisualizationView::QmitkVolumeVisualizationView()
  : QmitkAbstractView()
  , m_Controls(nullptr)
{
}

void QmitkVolumeVisualizationView::SetFocus()
{
}

void QmitkVolumeVisualizationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls = new Ui::QmitkVolumeVisualizationViewControls;
  m_Controls->setupUi(parent);

  m_Controls->volumeSelectionWidget->SetDataStorage(GetDataStorage());
  m_Controls->volumeSelectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Image>::New(),
    mitk::NodePredicateOr::New(mitk::NodePredicateDimension::New(3), mitk::NodePredicateDimension::New(4)),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  m_Controls->volumeSelectionWidget->SetSelectionIsOptional(true);
  m_Controls->volumeSelectionWidget->SetEmptyInfo(QString("Please select a 3D / 4D image volume"));
  m_Controls->volumeSelectionWidget->SetPopUpTitel(QString("Select image volume"));

  // Fill the transfer function presets in the generator widget
  std::vector<std::string> names;
  mitk::TransferFunctionInitializer::GetPresetNames(names);
  for (const auto& name : names)
  {
    m_Controls->transferFunctionGeneratorWidget->AddPreset(QString::fromStdString(name));
  }

  // see vtkVolumeMapper::BlendModes
  m_Controls->blendMode->addItem("Composite", vtkVolumeMapper::COMPOSITE_BLEND);
  m_Controls->blendMode->addItem("Maximum intensity", vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
  m_Controls->blendMode->addItem("Minimum intensity", vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
  m_Controls->blendMode->addItem("Average intensity", vtkVolumeMapper::AVERAGE_INTENSITY_BLEND);
  m_Controls->blendMode->addItem("Additive", vtkVolumeMapper::ADDITIVE_BLEND);

  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkVolumeVisualizationView::OnCurrentSelectionChanged);
  connect(m_Controls->enableRenderingCB, SIGNAL(toggled(bool)), this, SLOT(OnEnableRendering(bool)));
  connect(m_Controls->blendMode, SIGNAL(activated(int)), this, SLOT(OnBlendMode(int)));

  connect(m_Controls->transferFunctionGeneratorWidget, SIGNAL(SignalUpdateCanvas()),
          m_Controls->transferFunctionWidget, SLOT(OnUpdateCanvas()));
  connect(m_Controls->transferFunctionGeneratorWidget, SIGNAL(SignalTransferFunctionModeChanged(int)),
          SLOT(OnMitkInternalPreset(int)));

  m_Controls->enableRenderingCB->setEnabled(false);
  m_Controls->blendMode->setEnabled(false);
  m_Controls->transferFunctionWidget->setEnabled(false);
  m_Controls->transferFunctionGeneratorWidget->setEnabled(false);

  m_Controls->volumeSelectionWidget->SetAutoSelectNewNodes(true);

  this->m_TimePointChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_TimePointChangeListener,
          &QmitkSliceNavigationListener::SelectedTimePointChanged,
          this,
          &QmitkVolumeVisualizationView::OnSelectedTimePointChanged);
}

void QmitkVolumeVisualizationView::RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart)
{
  this->m_TimePointChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkVolumeVisualizationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart)
{
  this->m_TimePointChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkVolumeVisualizationView::OnMitkInternalPreset(int mode)
{
  auto node = m_SelectedNode.Lock();

  if (node.IsNull())
  {
    return;
  }

  mitk::TransferFunctionProperty::Pointer transferFuncProp;
  if (node->GetProperty(transferFuncProp, "TransferFunction"))
  {
    // first item is only information
    if (--mode == -1)
      return;

    // -- Creat new TransferFunction
    mitk::TransferFunctionInitializer::Pointer tfInit = mitk::TransferFunctionInitializer::New(transferFuncProp->GetValue());
    tfInit->SetTransferFunctionMode(mode);
    RequestRenderWindowUpdate();
    m_Controls->transferFunctionWidget->OnUpdateCanvas();
  }
}

void QmitkVolumeVisualizationView::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_SelectedNode = nullptr;

  if (nodes.empty() || nodes.front().IsNull())
  {
    UpdateInterface();
    return;
  }

  auto selectedNode = nodes.front();
  auto image = dynamic_cast<mitk::Image*>(selectedNode->GetData());
  if (nullptr != image)
  {
    m_SelectedNode = selectedNode;
  }

  UpdateInterface();
}

void QmitkVolumeVisualizationView::OnEnableRendering(bool state)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
  {
    return;
  }

  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));
  UpdateInterface();
  RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationView::OnBlendMode(int index)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  int mode = m_Controls->blendMode->itemData(index).toInt();
  selectedNode->SetProperty("volumerendering.blendmode", mitk::IntProperty::New(mode));

  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationView::OnSelectedTimePointChanged(const mitk::TimePointType & /*newTimePoint*/)
{
  this->UpdateInterface();
}

void QmitkVolumeVisualizationView::UpdateInterface()
{
  if (m_SelectedNode.IsExpired())
  {
    // turnoff all
    m_Controls->enableRenderingCB->setChecked(false);
    m_Controls->enableRenderingCB->setEnabled(false);

    m_Controls->blendMode->setCurrentIndex(0);
    m_Controls->blendMode->setEnabled(false);

    m_Controls->transferFunctionWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionWidget->setEnabled(false);

    m_Controls->transferFunctionGeneratorWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionGeneratorWidget->setEnabled(false);
    return;
  }

  bool enabled = false;
  auto selectedNode = m_SelectedNode.Lock();

  selectedNode->GetBoolProperty("volumerendering", enabled);
  m_Controls->enableRenderingCB->setEnabled(true);
  m_Controls->enableRenderingCB->setChecked(enabled);

  if (!enabled)
  {
    // turnoff all except volumerendering checkbox
    m_Controls->blendMode->setCurrentIndex(0);
    m_Controls->blendMode->setEnabled(false);

    m_Controls->transferFunctionWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionWidget->setEnabled(false);

    m_Controls->transferFunctionGeneratorWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionGeneratorWidget->setEnabled(false);
    return;
  }

  // otherwise we can activate em all
  m_Controls->blendMode->setEnabled(true);

  // Determine Combo Box mode
  int blendMode;
  if (selectedNode->GetIntProperty("volumerendering.blendmode", blendMode))
    m_Controls->blendMode->setCurrentIndex(blendMode);

  auto time = this->GetRenderWindowPart()->GetTimeNavigationController()->GetSelectedTimeStep();
  m_Controls->transferFunctionWidget->SetDataNode(selectedNode, time);
  m_Controls->transferFunctionWidget->setEnabled(true);
  m_Controls->transferFunctionGeneratorWidget->SetDataNode(selectedNode, time);
  m_Controls->transferFunctionGeneratorWidget->setEnabled(true);
}
