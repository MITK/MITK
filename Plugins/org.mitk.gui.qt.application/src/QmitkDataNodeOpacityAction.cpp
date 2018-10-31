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

#include <QmitkDataNodeOpacityAction.h>

// mitk core
#include <mitkRenderingManager.h>

// qt
#include <QHBoxLayout>
#include <QLabel>

QmitkDataNodeOpacityAction::QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  InitializeAction();
}

QmitkDataNodeOpacityAction::QmitkDataNodeOpacityAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  InitializeAction();
}

QmitkDataNodeOpacityAction::~QmitkDataNodeOpacityAction()
{
  // nothing here
}

void QmitkDataNodeOpacityAction::InitializeAction()
{
  m_OpacitySlider = new QSlider;
  m_OpacitySlider->setMinimum(0);
  m_OpacitySlider->setMaximum(100);
  m_OpacitySlider->setOrientation(Qt::Horizontal);
  connect(m_OpacitySlider, &QSlider::valueChanged, this, &QmitkDataNodeOpacityAction::OnOpacityChanged);

  QLabel* opacityLabel = new QLabel(tr("Opacity: "));
  QHBoxLayout* opacityWidgetLayout = new QHBoxLayout;
  opacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
  opacityWidgetLayout->addWidget(opacityLabel);
  opacityWidgetLayout->addWidget(m_OpacitySlider);

  QWidget* opacityWidget = new QWidget;
  opacityWidget->setLayout(opacityWidgetLayout);

  setDefaultWidget(opacityWidget);

  connect(this, &QAction::changed, this, &QmitkDataNodeOpacityAction::OnActionChanged);
}

void QmitkDataNodeOpacityAction::InitializeWithDataNode(const mitk::DataNode* dataNode)
{
  float opacity = 0.0;
  if (dataNode->GetFloatProperty("opacity", opacity))
  {
    m_OpacitySlider->setValue(static_cast<int>(opacity * 100));
  }
}

void QmitkDataNodeOpacityAction::OnOpacityChanged(int value)
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  float opacity = static_cast<float>(value) / 100.0f;
  dataNode->SetFloatProperty("opacity", opacity);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataNodeOpacityAction::OnActionChanged()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  InitializeWithDataNode(dataNode);
}
