/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeVisualizationV2View.h"

#include <mitkImage.h>

#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>
#include <QmitkCombinedTransferFunctionCanvas.h>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

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
  //m_Controls->verticalLayout->setStretch(1, 3);
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
  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::OpacityChanged,
    this, &QmitkVolumeVisualizationV2View::OnCanvasOpacityChanged);

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

  m_Controls->tfControlPanelsWidget->SetDataNode(selectedNode);

  auto *image = selectedNode->GetDataAs<mitk::Image>();
  mitk::SimpleHistogram *histogram = (image != nullptr) ? m_HistogramCache[image] : nullptr;

  m_Controls->combinedTfCanvas->SetHistogram(histogram);
  m_Controls->combinedTfCanvas->SetColorTransferFunction(preset->GetColorTransferFunction());
  m_Controls->combinedTfCanvas->SetPiecewiseFunction(preset->GetScalarOpacityFunction());

  if (histogram != nullptr)
  {
    // Use the image scalar range as the visible x-axis so histogram, gradient
    // and curve line up. (SetPiecewiseFunction defaulted these to the function's
    // own range, so this must come after it.)
    m_DataRange = { histogram->GetMin(), histogram->GetMax() };
    m_Controls->combinedTfCanvas->SetMin(histogram->GetMin());
    m_Controls->combinedTfCanvas->SetMax(histogram->GetMax());
  }

  this->SnapshotAppliedTransferFunction();
  this->ResetShiftWidthControls();
  this->UpdateInterface();
  this->RequestRenderWindowUpdate();
}

void QmitkVolumeVisualizationV2View::SnapshotAppliedTransferFunction()
{
  if (m_AppliedTransferFunction.IsNull())
  {
    m_BaseColorFn = nullptr;
    return;
  }

  // Keep the untouched copy of the color function to resample from:
  // DeepCopy preserves the color space (HSV) and clamping, so windowing
  // stays faithful to the preset. Sampling bare RGB points instead would
  // interpolate in the wrong color space and shift the colors on the
  // first slider move.
  m_BaseColorFn = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_BaseColorFn->DeepCopy(m_AppliedTransferFunction->GetColorTransferFunction());
}

void QmitkVolumeVisualizationV2View::ResetShiftWidthControls()
{
  const QSignalBlocker blockShift(m_Controls->shiftSlider);
  const QSignalBlocker blockWidth(m_Controls->widthSlider);

  // Scale the sliders to the image's own value range -- the same axis the canvas
  // draws -- so the window can be moved and sized across everything you see.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);

  double colorSpan = dataWidth;
  if (m_BaseColorFn != nullptr && m_BaseColorFn->GetSize() > 0)
  {
    const double *colorRange = m_BaseColorFn->GetRange();
    colorSpan = std::max(1.0, colorRange[1] - colorRange[0]);
  }

  // Shift moves the window center (level); 0 keeps the preset's own center.
  m_Controls->shiftSlider->setMinimum(-dataWidth);
  m_Controls->shiftSlider->setMaximum(dataWidth);
  m_Controls->shiftSlider->setValue(0.0);

  // Width is the window size in intensity units; default to the preset's color
  // span so a fresh preset maps 1:1, and allow narrowing/widening around it.
  m_Controls->widthSlider->setMinimum(1.0);
  m_Controls->widthSlider->setMaximum(std::max(2.0 * colorSpan, dataWidth));
  m_Controls->widthSlider->setValue(colorSpan);
}

void QmitkVolumeVisualizationV2View::OnShiftOrWidthChanged()
{
  auto selectedNode = m_SelectedNode.Lock();

  if (m_AppliedTransferFunction.IsNull() || selectedNode.IsNull() || m_BaseColorFn == nullptr)
    return;

  if (m_DataRange[1] <= m_DataRange[0]) // no valid histogram range to span
    return;

  const double shift = m_Controls->shiftSlider->value();
  const double width = m_Controls->widthSlider->value();

  // Window/level around the preset's own color center; shift moves it, width sizes it.
  const double *colorRange = m_BaseColorFn->GetRange();
  const double colorCenter = 0.5 * (colorRange[0] + colorRange[1]);
  const double level = colorCenter + shift;
  const double windowMin = level - 0.5 * width;
  const double windowMax = level + 0.5 * width;

  m_AppliedTransferFunction->SetRGBPoints(
    mitk::ResampleColorWindow(m_BaseColorFn, m_DataRange[0], m_DataRange[1], windowMin, windowMax));

  m_AppliedTransferFunction->Modified();
  this->RequestRenderWindowUpdate();

  m_Controls->tfControlPanelsWidget->OnUpdateCanvas();
  m_Controls->combinedTfCanvas->update();
}

void QmitkVolumeVisualizationV2View::OnResetTransferFunction()
{
  // Reload the current preset: restores both the color map and the opacity curve
  // to the authored defaults and re-seeds the canvas + sliders.
  this->OnTransferFunctionPresetSelected(m_Controls->presetComboBox->currentText());
}

void QmitkVolumeVisualizationV2View::OnCanvasOpacityChanged()
{
  if (m_AppliedTransferFunction.IsNotNull())
    m_AppliedTransferFunction->Modified();

  // Keep the Advanced per-point editor in sync with the canvas edit.
  m_Controls->tfControlPanelsWidget->OnUpdateCanvas();
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
