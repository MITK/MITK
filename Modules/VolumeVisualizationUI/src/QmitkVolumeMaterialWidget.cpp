/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeMaterialWidget.h"

#include <mitkVolumeRenderingLightingModel.h>

#include <QmitkIconTheme.h>

#include <ui_QmitkVolumeMaterialWidgetControls.h>

#include <QPushButton>
#include <QSlider>

#include <algorithm>
#include <cmath>

namespace
{
  /** Positions per slider. For the coefficients, which run from 0 to 1, that is
   * a resolution of 0.01.
   */
  constexpr int SLIDER_STEPS = 100;

  constexpr double MAX_SPECULAR_POWER = 128.0;

  int CoefficientToPosition(float coefficient)
  {
    return qRound(coefficient * SLIDER_STEPS);
  }

  float PositionToCoefficient(int position)
  {
    return static_cast<float>(position) / SLIDER_STEPS;
  }

  /** Geometric from 1 to MAX_SPECULAR_POWER rather than linear: the highlight
   * narrows with roughly the square root of the power, so it is equal ratios
   * that look like equal changes, and a linear slider would crowd nearly all of
   * them into its first quarter.
   */
  int SpecularPowerToPosition(float specularPower)
  {
    return qRound(std::log(std::max(specularPower, 1.0f)) / std::log(MAX_SPECULAR_POWER) * SLIDER_STEPS);
  }

  float PositionToSpecularPower(int position)
  {
    return static_cast<float>(std::pow(MAX_SPECULAR_POWER, static_cast<double>(position) / SLIDER_STEPS));
  }

  /** Assign without emitting valueChanged, so that pushing node state into the
   * controls does not echo back and write it to the node again - rounded to a
   * slider position on the way.
   */
  void SetSliderPositionSilently(QSlider *slider, int position)
  {
    const QSignalBlocker blocker(slider);
    slider->setValue(position);
  }
}

QmitkVolumeMaterialWidget::QmitkVolumeMaterialWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_Controls(std::make_unique<Ui::QmitkVolumeMaterialWidget>())
{
  m_Controls->setupUi(this);

  for (auto *slider : { m_Controls->ambientSlider, m_Controls->diffuseSlider,
                        m_Controls->specularSlider, m_Controls->specularPowerSlider })
  {
    slider->setRange(0, SLIDER_STEPS);
  }

  // Set here rather than in the .ui: the resource is authored with a
  // placeholder fill that QmitkIconTheme swaps for the theme's icon color,
  // so a direct reference from the .ui would draw it in that placeholder.
  m_Controls->resetButton->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/reset.svg")));

  connect(m_Controls->ambientSlider, &QSlider::valueChanged, this, [this](int position)
    { this->SetMaterialValue(&mitk::VolumeRenderingMaterial::ambient, PositionToCoefficient(position)); });
  connect(m_Controls->diffuseSlider, &QSlider::valueChanged, this, [this](int position)
    { this->SetMaterialValue(&mitk::VolumeRenderingMaterial::diffuse, PositionToCoefficient(position)); });
  connect(m_Controls->specularSlider, &QSlider::valueChanged, this, [this](int position)
    { this->SetMaterialValue(&mitk::VolumeRenderingMaterial::specular, PositionToCoefficient(position)); });
  connect(m_Controls->specularPowerSlider, &QSlider::valueChanged, this, [this](int position)
    { this->SetMaterialValue(&mitk::VolumeRenderingMaterial::specularPower, PositionToSpecularPower(position)); });
  connect(m_Controls->resetButton, &QPushButton::clicked,
    this, &QmitkVolumeMaterialWidget::OnReset);
}

QmitkVolumeMaterialWidget::~QmitkVolumeMaterialWidget() = default;

void QmitkVolumeMaterialWidget::SetDataNode(mitk::DataNode *node)
{
  m_DataNode = node;

  this->UpdateControls();
}

void QmitkVolumeMaterialWidget::SetTitleSuffix(const QString &suffix)
{
  m_Controls->materialGroupBox->setTitle(suffix.isEmpty()
    ? QStringLiteral("Material")
    : QStringLiteral("Material - %1").arg(suffix));
}

void QmitkVolumeMaterialWidget::UpdateControls()
{
  auto node = m_DataNode.Lock();

  const auto material = mitk::VolumeRenderingMaterial::FromNode(node.GetPointer());

  SetSliderPositionSilently(m_Controls->ambientSlider, CoefficientToPosition(material.ambient));
  SetSliderPositionSilently(m_Controls->diffuseSlider, CoefficientToPosition(material.diffuse));
  SetSliderPositionSilently(m_Controls->specularSlider, CoefficientToPosition(material.specular));
  SetSliderPositionSilently(m_Controls->specularPowerSlider, SpecularPowerToPosition(material.specularPower));
}

void QmitkVolumeMaterialWidget::SetMaterialValue(float mitk::VolumeRenderingMaterial::*field, float value)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  // Only the moved value is taken from the controls. The others would come
  // back rounded to their slider positions, which need not be the values the
  // node holds - specular power almost never is - so moving one slider would
  // shift the rest.
  auto material = mitk::VolumeRenderingMaterial::FromNode(node.GetPointer());
  material.*field = value;

  // Asserted rather than carried over from the node: with shading off VTK
  // ignores the material values and both scattering parameters, so there is
  // no state here worth preserving, and a node that arrives with it off - set
  // through the Properties view, or saved by a view that offered the choice -
  // is brought into line as soon as its material is touched.
  material.shade = true;

  material.ApplyTo(node);

  emit LightingChanged();
}

void QmitkVolumeMaterialWidget::OnReset()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  // Back to the values the selected model dictates rather than to a fixed set:
  // the model is a choice, the slider positions are what was moved afterwards,
  // and only the latter is what reset undoes. ApplyTo writes exactly the four
  // material values and the scattering parameters the model owns.
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
