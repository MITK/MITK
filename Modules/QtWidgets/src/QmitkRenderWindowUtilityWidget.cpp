/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowUtilityWidget.h"

#include <QWidgetAction>

QmitkRenderWindowUtilityWidget::QmitkRenderWindowUtilityWidget(
  QWidget* parent/* = nullptr */,
  QmitkRenderWindow* renderWindow/* = nullptr */,
  mitk::DataStorage* dataStorage/* = nullptr */)
  : m_Layout(nullptr)
  , m_MenuBar(nullptr)
  , m_RenderWindow(renderWindow)
  , m_DataStorage(dataStorage)
  , m_RenderWindowInspector(nullptr)
  , m_SliceNavigationWidget(nullptr)
  , m_StepperAdapter(nullptr)
  , m_ViewDirectionSelector(nullptr)
{
  m_Layout = new QHBoxLayout(this);
  m_Layout->setMargin(0);

  auto* baseRenderer = mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow());
  auto* sliceNavigationController = m_RenderWindow->GetSliceNavigationController();
  m_RenderWindowInspector = new QmitkRenderWindowContextDataStorageInspector(parent, baseRenderer);
  m_RenderWindowInspector->SetDataStorage(m_DataStorage);
  m_RenderWindowInspector->setObjectName(QStringLiteral("m_RenderWindowManipulatorWidget"));
  connect(m_RenderWindowInspector, &QmitkRenderWindowContextDataStorageInspector::ReinitAction,
    this, &QmitkRenderWindowUtilityWidget::ReinitAction);
  connect(m_RenderWindowInspector, &QmitkRenderWindowContextDataStorageInspector::ResetAction,
    this, &QmitkRenderWindowUtilityWidget::ResetAction);

  m_MenuBar = new QMenuBar(this);
  m_MenuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  auto menu = m_MenuBar->addMenu("Data");
  QWidgetAction* newAct = new QWidgetAction(menu);
  newAct->setDefaultWidget(m_RenderWindowInspector);
  menu->addAction(newAct);
  m_Layout->addWidget(m_MenuBar);

  m_SliceNavigationWidget = new QmitkSliceNavigationWidget(this);
  m_StepperAdapter =
    new QmitkStepperAdapter(m_SliceNavigationWidget, sliceNavigationController->GetSlice());
  m_Layout->addWidget(m_SliceNavigationWidget);

  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer{ baseRenderer };
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_ViewDirectionSelector = new QComboBox(this);
  QStringList viewDirections{ "axial", "coronal", "sagittal"};
  m_ViewDirectionSelector->insertItems(0, viewDirections);
  connect(m_ViewDirectionSelector, &QComboBox::currentTextChanged, this, &QmitkRenderWindowUtilityWidget::ChangeViewDirection);
  auto viewDirection = sliceNavigationController->GetDefaultViewDirection();
  switch (viewDirection)
  {
  case mitk::AnatomicalPlane::Axial:
    m_ViewDirectionSelector->setCurrentIndex(0);
    break;
  case mitk::AnatomicalPlane::Coronal:
    m_ViewDirectionSelector->setCurrentIndex(1);
    break;
  case mitk::AnatomicalPlane::Sagittal:
    m_ViewDirectionSelector->setCurrentIndex(2);
    break;
  default:
    break;
  }
  m_Layout->addWidget(m_ViewDirectionSelector);
}

QmitkRenderWindowUtilityWidget::~QmitkRenderWindowUtilityWidget()
{
}

void QmitkRenderWindowUtilityWidget::SetInvertedSliceNavigation(bool inverted)
{
  m_SliceNavigationWidget->SetInverseDirection(inverted);
}

void QmitkRenderWindowUtilityWidget::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString());
}
