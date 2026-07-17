/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeVisualizationV2View.h"

#include <mitkImage.h>
#include <mitkImageStatisticsHolder.h>

#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>

#include <mitkProperties.h>

#include <ui_QmitkVolumeVisualizationV2View.h>

#include <ctkDoubleSlider.h>

#include <algorithm>

namespace
{
  constexpr double WIDTH_MIN = 0.1;
  constexpr double WIDTH_MAX = 3.0;
}

const std::string QmitkVolumeVisualizationV2View::VIEW_ID = "org.mitk.views.volumevisualization_v2";

QmitkVolumeVisualizationV2View::QmitkVolumeVisualizationV2View() 
{
  m_Controls = std::make_unique<Ui::QmitkVolumeVisualizationV2View>();
}

QmitkVolumeVisualizationV2View::~QmitkVolumeVisualizationV2View()
{
}

void QmitkVolumeVisualizationV2View::SetFocus() 
{
}

void QmitkVolumeVisualizationV2View::CreateQtPartControl(QWidget *parent)
{
  m_Controls->setupUi(parent);
  m_Controls->verticalLayout->setStretch(1, 3);
  m_Controls->verticalLayout->setStretch(2, 1);
  m_Controls->volumeSelectionWidget->SetDataStorage(this->GetDataStorage());
  m_Controls->volumeSelectionWidget->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Image>::New(),
    mitk::NodePredicateOr::New(mitk::NodePredicateDimension::New(3), mitk::NodePredicateDimension::New(4)),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  m_Controls->volumeSelectionWidget->SetSelectionIsOptional(true);
  m_Controls->volumeSelectionWidget->SetEmptyInfo(QString("Please select a 3D / 4D image volume"));
  m_Controls->volumeSelectionWidget->SetPopUpTitel(QString("Select image volume"));

  for (const auto &name : m_Presets.GetPresetNames())
  {
    m_Controls->presetComboBox->addItem(QString::fromStdString(name));
  }

  //select the applied preset, or -1 for "none":
  m_Controls->presetComboBox->setCurrentIndex(-1);
  m_Controls->presetComboBox->setEnabled(false);
  m_Controls->enableRenderingCB->setEnabled(false);

  m_Controls->shiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->widthSlider->setOrientation(Qt::Horizontal);
  m_Controls->widthSlider->setMinimum(WIDTH_MIN);
  m_Controls->widthSlider->setMaximum(WIDTH_MAX);
  m_Controls->widthSlider->setSingleStep(0.05);
  
  connect(m_Controls->volumeSelectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
      this, &QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged);
    
  connect(m_Controls->enableRenderingCB, &QCheckBox::toggled,
    this, &QmitkVolumeVisualizationV2View::OnEnabledRendering);
  
  connect(m_Controls->presetComboBox, &QComboBox::textActivated,
    this, &QmitkVolumeVisualizationV2View::OnTransferFunctionPresetSelected);

  connect(m_Controls->shiftSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnShiftOrWidthChanged);
  connect(m_Controls->widthSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeVisualizationV2View::OnShiftOrWidthChanged);
  connect(m_Controls->resetTfButton, &QPushButton::clicked,
    this, &QmitkVolumeVisualizationV2View::OnResetTransferFunction);

  m_Controls->volumeSelectionWidget->SetAutoSelectNewNodes(true);
}

void QmitkVolumeVisualizationV2View::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  m_SelectedNode = nullptr;
  m_AppliedTransferFunction = nullptr;

  if (nodes.empty() || nodes.front().IsNull())
  {
    this->UpdateInterface();
    return;
  }

  auto selectedNode = nodes.front();
  
  if (selectedNode->GetDataAs<mitk::Image>() != nullptr)
    m_SelectedNode = selectedNode;

  this->UpdateInterface();
}

void QmitkVolumeVisualizationV2View::OnEnabledRendering(bool state)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  selectedNode->SetProperty("volumerendering", mitk::BoolProperty::New(state));
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::OnTransferFunctionPresetSelected(const QString &presetName)
{
  auto selectedNode = m_SelectedNode.Lock();

  if (selectedNode.IsNull())
    return;

  auto preset = m_Presets.CreateTransferFunction(presetName.toStdString());

  if (preset.IsNull())
    return;

  m_AppliedTransferFunction = preset;
  m_EffectiveRange = m_Presets.GetEffectiveRange(presetName.toStdString());

  // Preserve property identity: reuse the existing property, create it only once.
  auto *tfProperty = dynamic_cast<mitk::TransferFunctionProperty *>(selectedNode->GetProperty("TransferFunction"));

  if(tfProperty != nullptr)
  {
    tfProperty->SetValue(preset);
  } 
  else
  {
    selectedNode->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(preset));
  }

  this->SnapshotAppliedTransferFunction();
  this->ResetShiftWidthControls();
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();

  m_Controls->tfControlPanelsWidget->SetDataNode(selectedNode);
}

void QmitkVolumeVisualizationV2View::SnapshotAppliedTransferFunction()
{
  if (m_AppliedTransferFunction.IsNull())
  {
    m_BaseScalarOpacity.clear();
    m_BaseColor.clear();
    m_WidthCenter = 0.0;
    return;
  }

  m_BaseScalarOpacity = m_AppliedTransferFunction->GetScalarOpacityPoints();
  m_BaseColor = m_AppliedTransferFunction->GetRGBPoints();

  // Points come back from VTK sorted by position, so front/back are the extremes.
  m_WidthCenter = m_BaseScalarOpacity.empty()
    ? 0.0
    : 0.5 * (m_BaseScalarOpacity.front().first + m_BaseScalarOpacity.back().first);
}

void QmitkVolumeVisualizationV2View::ResetShiftWidthControls()
{
  auto selectedNode = m_SelectedNode.Lock();

  //double rangeMin = 0.0;
  //double rangeMax = 0.0;

  //if (selectedNode.IsNotNull())
  //{
  //  if (auto *image = selectedNode->GetDataAs<mitk::Image>())
  //  {
  //    rangeMin = image->GetStatistics()->GetScalarValueMin();
  //    rangeMax = image->GetStatistics()->GetScalarValueMax();
  //  }
  //}

  //const double span = std::max(1.0, rangeMax - rangeMin);

  const double effWidth = std::max(1.0, m_EffectiveRange[1] - m_EffectiveRange[0]);

  const QSignalBlocker blockShift(m_Controls->shiftSlider);
  const QSignalBlocker blockWidth(m_Controls->widthSlider);

  //m_Controls->shiftSlider->setMinimum(-span);
  //m_Controls->shiftSlider->setMaximum(span);
  m_Controls->shiftSlider->setMinimum(-effWidth);
  m_Controls->shiftSlider->setMaximum(effWidth);
  m_Controls->shiftSlider->setValue(0.0);
  m_Controls->widthSlider->setValue(1.0);
}

void QmitkVolumeVisualizationV2View::OnShiftOrWidthChanged()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (m_AppliedTransferFunction.IsNull() || selectedNode.IsNull() || m_BaseScalarOpacity.empty())
    return;

  const double shift = m_Controls->shiftSlider->value();
  const double width = m_Controls->widthSlider->value();

  const double scale = width;
  const double offset = m_WidthCenter * (1.0 - width) + shift;

  m_AppliedTransferFunction->SetScalarOpacityPoints(mitk::RemapIntensity(m_BaseScalarOpacity, scale, offset));
  m_AppliedTransferFunction->SetRGBPoints(mitk::RemapIntensity(m_BaseColor, scale, offset));

  m_AppliedTransferFunction->Modified();
  this->RequestRenderWindowUpdate();

  m_Controls->tfControlPanelsWidget->OnUpdateCanvas();
}

void QmitkVolumeVisualizationV2View::OnResetTransferFunction()
{
  this->ResetShiftWidthControls();
  this->OnShiftOrWidthChanged();
}

void QmitkVolumeVisualizationV2View::UpdateInterface()
{
  auto selectedNode = m_SelectedNode.Lock();

  if(selectedNode.IsNull())
  {
    m_Controls->enableRenderingCB->setChecked(false);
    m_Controls->enableRenderingCB->setEnabled(false);
    m_Controls->presetComboBox->setEnabled(false);
    m_Controls->shiftSlider->setEnabled(false);
    m_Controls->widthSlider->setEnabled(false);
    m_Controls->resetTfButton->setEnabled(false);
    return;
  }

  bool volumeRenderingOn = false;
  selectedNode->GetBoolProperty("volumerendering", volumeRenderingOn);

  m_Controls->enableRenderingCB->setEnabled(true);
  m_Controls->presetComboBox->setEnabled(volumeRenderingOn);

  const bool tfAdjustable = volumeRenderingOn && m_AppliedTransferFunction.IsNotNull();
  m_Controls->shiftSlider->setEnabled(tfAdjustable);
  m_Controls->widthSlider->setEnabled(tfAdjustable);
  m_Controls->resetTfButton->setEnabled(tfAdjustable);

  const QSignalBlocker blocker(m_Controls->enableRenderingCB);
  m_Controls->enableRenderingCB->setChecked(volumeRenderingOn);
}
