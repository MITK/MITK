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

  // see enum in vtkSmartVolumeMapper
  m_Controls->renderMode->addItem("Default");
  m_Controls->renderMode->addItem("RayCast");
  m_Controls->renderMode->addItem("GPU");

  // see vtkVolumeMapper::BlendModes
  m_Controls->blendMode->addItem("Comp");
  m_Controls->blendMode->addItem("Max");
  m_Controls->blendMode->addItem("Min");
  m_Controls->blendMode->addItem("Avg");
  m_Controls->blendMode->addItem("Add");

  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkVolumeVisualizationView::OnCurrentSelectionChanged);
  connect(m_Controls->enableRenderingCB, SIGNAL(toggled(bool)), this, SLOT(OnEnableRendering(bool)));
  connect(m_Controls->renderMode, SIGNAL(activated(int)), this, SLOT(OnRenderMode(int)));
  connect(m_Controls->blendMode, SIGNAL(activated(int)), this, SLOT(OnBlendMode(int)));

  connect(m_Controls->transferFunctionGeneratorWidget, SIGNAL(SignalUpdateCanvas()),
          m_Controls->transferFunctionWidget, SLOT(OnUpdateCanvas()));
  connect(m_Controls->transferFunctionGeneratorWidget, SIGNAL(SignalTransferFunctionModeChanged(int)),
          SLOT(OnMitkInternalPreset(int)));

  m_Controls->enableRenderingCB->setEnabled(false);
  m_Controls->blendMode->setEnabled(false);
  m_Controls->renderMode->setEnabled(false);
  m_Controls->transferFunctionWidget->setEnabled(false);
  m_Controls->transferFunctionGeneratorWidget->setEnabled(false);

  m_Controls->volumeSelectionWidget->SetAutoSelectNewNodes(true);
}

void QmitkVolumeVisualizationView::OnMitkInternalPreset(int mode)
{
  if (m_SelectedNode.IsExpired())
  {
    return;
  }

  auto node = m_SelectedNode.Lock();
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
  if (m_SelectedNode.IsExpired())
  {
    return;
  }

  m_SelectedNode.Lock()->SetProperty("volumerendering", mitk::BoolProperty::New(state));
  UpdateInterface();
  RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationView::OnRenderMode(int mode)
{
  if (m_SelectedNode.IsExpired())
  {
    return;
  }

  auto selectedNode = m_SelectedNode.Lock();

  bool usegpu = false;
  bool useray = false;
  if (DEFAULT_RENDERMODE == mode)
  {
    useray = true;
    usegpu = true;
  }
  else if (GPU_RENDERMODE == mode)
  {
    usegpu = true;
  }
  else if (RAYCAST_RENDERMODE == mode)
  {
    useray = true;
  }

  selectedNode->SetProperty("volumerendering.usegpu", mitk::BoolProperty::New(usegpu));
  selectedNode->SetProperty("volumerendering.useray", mitk::BoolProperty::New(useray));

  RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationView::OnBlendMode(int mode)
{
  if (m_SelectedNode.IsExpired())
  {
    return;
  }

  auto selectedNode = m_SelectedNode.Lock();

  bool usemip = false;
  if (vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND == mode)
  {
    usemip = true;
  }

  selectedNode->SetProperty("volumerendering.usemip", mitk::BoolProperty::New(usemip));
  selectedNode->SetProperty("volumerendering.blendmode", mitk::IntProperty::New(mode));

  RequestRenderWindowUpdate();
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

    m_Controls->renderMode->setCurrentIndex(0);
    m_Controls->renderMode->setEnabled(false);

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

    m_Controls->renderMode->setCurrentIndex(0);
    m_Controls->renderMode->setEnabled(false);

    m_Controls->transferFunctionWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionWidget->setEnabled(false);

    m_Controls->transferFunctionGeneratorWidget->SetDataNode(nullptr);
    m_Controls->transferFunctionGeneratorWidget->setEnabled(false);
    return;
  }

  // otherwise we can activate em all
  m_Controls->blendMode->setEnabled(true);
  m_Controls->renderMode->setEnabled(true);

  // Determine Combo Box mode
  {
    bool usegpu = false;
    bool useray = false;
    bool usemip = false;
    selectedNode->GetBoolProperty("volumerendering.usegpu", usegpu);
    selectedNode->GetBoolProperty("volumerendering.useray", useray);
    selectedNode->GetBoolProperty("volumerendering.usemip", usemip);

    int blendMode;
    if (selectedNode->GetIntProperty("volumerendering.blendmode", blendMode))
      m_Controls->blendMode->setCurrentIndex(blendMode);

    if (usemip)
      m_Controls->blendMode->setCurrentIndex(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);

    int mode = DEFAULT_RENDERMODE;

    if (useray)
      mode = RAYCAST_RENDERMODE;
    else if (usegpu)
      mode = GPU_RENDERMODE;

    m_Controls->renderMode->setCurrentIndex(mode);
  }

  m_Controls->transferFunctionWidget->SetDataNode(selectedNode);
  m_Controls->transferFunctionWidget->setEnabled(true);
  m_Controls->transferFunctionGeneratorWidget->SetDataNode(selectedNode);
  m_Controls->transferFunctionGeneratorWidget->setEnabled(true);
}
