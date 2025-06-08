/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataNodeOpacityAction.h>

#include <mitkRenderingManager.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

QmitkDataNodeOpacityAction::QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QWidgetAction(parent),
    QmitkAbstractDataNodeAction(workbenchPartSite),
    m_Slider(nullptr)
{
  this->InitializeAction();
}

QmitkDataNodeOpacityAction::QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QmitkDataNodeOpacityAction(parent, berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
}

void QmitkDataNodeOpacityAction::InitializeAction()
{
  auto label = new QLabel("Opacity: ");

  m_Slider = new QSlider;
  m_Slider->setMinimum(0);
  m_Slider->setMaximum(100);
  m_Slider->setOrientation(Qt::Horizontal);

  auto layout = new QHBoxLayout;
  layout->setContentsMargins(4, 4, 4, 4);
  layout->addWidget(label);
  layout->addWidget(m_Slider);

  auto widget = new QWidget;
  widget->setLayout(layout);

  this->setDefaultWidget(widget);

  connect(m_Slider, &QSlider::valueChanged, this, &QmitkDataNodeOpacityAction::OnOpacityChanged);
}

void QmitkDataNodeOpacityAction::InitializeWithDataNode(const mitk::DataNode*)
{
  // Get all selected nodes that have an opacity property.
  auto nodes = this->GetSelectedOpacityNodes();

  // Disable the opacity slider if no nodes would be affected anyway.
  m_Slider->setDisabled(nodes.empty());

  // For the initially shown opacity value, choose the maximum opacity of all
  // potentially affected nodes. Also works nicely with a single node or
  // in particular with all nodes having the same opacity anyway.

  float opacity = 0.0f;

  for (auto node : nodes)
    opacity = std::max(opacity, this->GetOpacity(node).value());

  // Do not trigger the valueChanged signal of the slider when initializing
  // its value since affected nodes may have different opacities in the
  // beginning and we only want to change them when the opacity is actively
  // changed through user interaction.

  m_Slider->blockSignals(true);
  m_Slider->setValue(static_cast<int>(opacity * 100));
  m_Slider->blockSignals(false);
}

void QmitkDataNodeOpacityAction::OnOpacityChanged(int value)
{
  float opacity = value * 0.01f;

  for (auto node : this->GetSelectedOpacityNodes())
    node->SetOpacity(opacity);

  auto renderer = this->GetBaseRenderer();

  if (renderer.IsNull())
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    auto renderWindow = renderer->GetRenderWindow();
    mitk::RenderingManager::GetInstance()->RequestUpdate(renderWindow);
  }
}

std::optional<float> QmitkDataNodeOpacityAction::GetOpacity(mitk::DataNode* node)
{
  if (float opacity = 0.0f; node != nullptr && node->GetOpacity(opacity, this->GetBaseRenderer()))
    return opacity;

  return std::nullopt;
}

std::vector<mitk::DataNode*> QmitkDataNodeOpacityAction::GetSelectedOpacityNodes()
{
  auto nodes = this->GetSelectedNodes();
  std::vector<mitk::DataNode*> opacityNodes;

  std::copy_if(nodes.begin(), nodes.end(), std::back_inserter(opacityNodes),
    [this](mitk::DataNode* node) { return this->GetOpacity(node).has_value(); });

  return opacityNodes;
}
