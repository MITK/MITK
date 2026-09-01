/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeLightingWidget.h"

#include <mitkVolumeRenderingLightingModel.h>
#include <mitkVolumeRenderingMaterial.h>

#include <ui_QmitkVolumeLightingWidgetControls.h>

#include <ctkSliderWidget.h>

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

namespace
{
  void ConfigureSlider(ctkSliderWidget *slider, int decimals, double minimum, double maximum, double step)
  {
    slider->setDecimals(decimals);
    slider->setRange(minimum, maximum);
    slider->setSingleStep(step);
  }

  /** Assign without emitting valueChanged, so that pushing node state into the
   * controls does not echo back through OnMaterialChanged and write it to the
   * node again - rounded to the slider's decimals on the way.
   */
  void SetSliderValueSilently(ctkSliderWidget *slider, float value)
  {
    const QSignalBlocker blocker(slider);
    slider->setValue(value);
  }
}

QmitkVolumeLightingWidget::QmitkVolumeLightingWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_Controls(std::make_unique<Ui::QmitkVolumeLightingWidget>())
{
  m_Controls->setupUi(this);

  ConfigureSlider(m_Controls->ambientSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->diffuseSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->specularSlider, 2, 0.0, 1.0, 0.01);
  ConfigureSlider(m_Controls->specularPowerSlider, 1, 1.0, 128.0, 1.0);

  // The label is for the reader, the id for the code: carrying the id on the row
  // keeps every lookup independent of the fill order.
  for (const auto &model : mitk::VolumeRenderingLightingModel::GetAllModels())
  {
    m_Controls->lightingModelComboBox->addItem(
      QString::fromStdString(model.label), QString::fromStdString(model.id));
  }

  connect(m_Controls->shadeCheckBox, &QCheckBox::toggled,
    this, &QmitkVolumeLightingWidget::OnMaterialChanged);
  connect(m_Controls->ambientSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeLightingWidget::OnMaterialChanged);
  connect(m_Controls->diffuseSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeLightingWidget::OnMaterialChanged);
  connect(m_Controls->specularSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeLightingWidget::OnMaterialChanged);
  connect(m_Controls->specularPowerSlider, &ctkSliderWidget::valueChanged,
    this, &QmitkVolumeLightingWidget::OnMaterialChanged);
  connect(m_Controls->lightingModelComboBox, &QComboBox::currentIndexChanged,
    this, &QmitkVolumeLightingWidget::OnModelChanged);
  connect(m_Controls->resetButton, &QPushButton::clicked,
    this, &QmitkVolumeLightingWidget::OnReset);
}

QmitkVolumeLightingWidget::~QmitkVolumeLightingWidget() = default;

void QmitkVolumeLightingWidget::SetDataNode(mitk::DataNode *node)
{
  m_DataNode = node;

  this->UpdateControls();
}

void QmitkVolumeLightingWidget::UpdateControls()
{
  auto node = m_DataNode.Lock();

  const auto material = mitk::VolumeRenderingMaterial::FromNode(node.GetPointer());

  // Phong parameters; with shading off they do nothing. Reset restores those
  // same values, so with shading off it has nothing to do either.
  m_Controls->ambientSlider->setEnabled(material.shade);
  m_Controls->diffuseSlider->setEnabled(material.shade);
  m_Controls->specularSlider->setEnabled(material.shade);
  m_Controls->specularPowerSlider->setEnabled(material.shade);
  m_Controls->lightingModelComboBox->setEnabled(material.shade);
  m_Controls->resetButton->setEnabled(material.shade);

  const QSignalBlocker blockShade(m_Controls->shadeCheckBox);
  m_Controls->shadeCheckBox->setChecked(material.shade);

  SetSliderValueSilently(m_Controls->ambientSlider, material.ambient);
  SetSliderValueSilently(m_Controls->diffuseSlider, material.diffuse);
  SetSliderValueSilently(m_Controls->specularSlider, material.specular);
  SetSliderValueSilently(m_Controls->specularPowerSlider, material.specularPower);

  // Derived from the node rather than remembered, so the node stays the single
  // source of truth. Matched by the id each row carries rather than by row
  // number, so nothing here depends on the combo's fill order.
  const auto *model = mitk::VolumeRenderingLightingModel::FromNode(node.GetPointer());

  const QSignalBlocker blockModel(m_Controls->lightingModelComboBox);
  m_Controls->lightingModelComboBox->setCurrentIndex(model != nullptr
    ? m_Controls->lightingModelComboBox->findData(QString::fromStdString(model->id))
    : -1);
}

void QmitkVolumeLightingWidget::OnMaterialChanged()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  mitk::VolumeRenderingMaterial material;

  material.shade = m_Controls->shadeCheckBox->isChecked();
  material.ambient = static_cast<float>(m_Controls->ambientSlider->value());
  material.diffuse = static_cast<float>(m_Controls->diffuseSlider->value());
  material.specular = static_cast<float>(m_Controls->specularSlider->value());
  material.specularPower = static_cast<float>(m_Controls->specularPowerSlider->value());

  material.ApplyTo(node);

  // Only the shade flag changes what the other controls may do, but one path for
  // "the node changed, re-read it" is worth more than the saved work.
  this->UpdateControls();

  emit LightingChanged();
}

void QmitkVolumeLightingWidget::OnModelChanged(int index)
{
  auto node = m_DataNode.Lock();

  // UpdateControls reports -1 for a node naming no model, but does so behind a
  // QSignalBlocker, so a signal always names a real row.
  if (node.IsNull() || index < 0)
    return;

  const auto *model = mitk::VolumeRenderingLightingModel::FromId(
    m_Controls->lightingModelComboBox->itemData(index).toString().toStdString());

  // A row carrying an id no model claims is not something this widget can build,
  // so it is a programming error rather than a state to handle.
  if (model == nullptr)
    return;

  model->ApplyTo(node);

  this->UpdateControls();

  emit LightingChanged();
}

void QmitkVolumeLightingWidget::OnReset()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  // Back to the values the selected model dictates rather than to a fixed set:
  // the model is a choice, the slider positions are what was moved afterwards,
  // and only the latter is what reset undoes. ApplyTo writes exactly the four
  // material values and the scattering parameters the model owns, and leaves
  // the shade flag alone unless the model needs it on.
  const auto *model = mitk::VolumeRenderingLightingModel::FromNode(node.GetPointer());

  // A node bound here normally names a model, since the view applies one when
  // rendering is switched on. One configured elsewhere need not, and then the
  // first model is the same fallback the view uses.
  if (model == nullptr)
  {
    const auto &models = mitk::VolumeRenderingLightingModel::GetAllModels();

    if (models.empty())
      return;

    model = &models.front();
  }

  model->ApplyTo(node);

  this->UpdateControls();

  emit LightingChanged();
}
