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

#include <QmitkDataNodeComponentAction.h>

// mitk core
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkRenderingManager.h>

// mitk gui common plugin
#include <mitkDataNodeSelection.h>

// qt
#include <QHBoxLayout>
#include <QLabel>

QmitkDataNodeComponentAction::QmitkDataNodeComponentAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchpartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(workbenchpartSite)
{
  InitializeAction();
}

QmitkDataNodeComponentAction::QmitkDataNodeComponentAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchpartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchpartSite))
{
  InitializeAction();
}

void QmitkDataNodeComponentAction::InitializeAction()
{
  setCheckable(true);

  m_ComponentSlider = new QmitkNumberPropertySlider;
  m_ComponentSlider->setOrientation(Qt::Horizontal);

  QLabel* componentLabel = new QLabel(tr("Component: "));
  QHBoxLayout* componentWidgetLayout = new QHBoxLayout;
  componentWidgetLayout->setContentsMargins(4, 4, 4, 4);
  componentWidgetLayout->addWidget(componentLabel);
  componentWidgetLayout->addWidget(m_ComponentSlider);
  QLabel* componentValueLabel = new QLabel();
  componentWidgetLayout->addWidget(componentValueLabel);
  connect(m_ComponentSlider, &QmitkNumberPropertySlider::valueChanged, componentValueLabel, static_cast<void (QLabel::*)(int)>(&QLabel::setNum));

  QWidget* componentWidget = new QWidget;
  componentWidget->setLayout(componentWidgetLayout);

  setDefaultWidget(componentWidget);

  connect(this, &QmitkDataNodeComponentAction::changed, this, &QmitkDataNodeComponentAction::OnActionChanged);
}

void QmitkDataNodeComponentAction::InitializeWithDataNode(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    m_ComponentSlider->SetProperty(static_cast<mitk::IntProperty*>(nullptr));
    return;
  }

  mitk::Image* img = dynamic_cast<mitk::Image*>(dataNode->GetData());
  if (nullptr == img)
  {
    m_ComponentSlider->SetProperty(static_cast<mitk::IntProperty*>(nullptr));
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  int numComponents = 0;
  numComponents = img->GetPixelType().GetNumberOfComponents();
  mitk::IntProperty* componentProperty = dynamic_cast<mitk::IntProperty*>(dataNode->GetProperty("Image.Displayed Component", baseRenderer));
  if (numComponents <= 1 || nullptr == componentProperty)
  {
    m_ComponentSlider->SetProperty(static_cast<mitk::IntProperty*>(nullptr));
    return;
  }

  m_ComponentSlider->SetProperty(componentProperty);
  m_ComponentSlider->setMinValue(0);
  m_ComponentSlider->setMaxValue(numComponents - 1);
}

void QmitkDataNodeComponentAction::OnActionChanged()
{
  auto dataNode = GetSelectedNode();

  InitializeWithDataNode(dataNode);
}
