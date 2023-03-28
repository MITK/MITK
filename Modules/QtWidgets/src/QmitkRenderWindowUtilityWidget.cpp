/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowUtilityWidget.h"

#include <QWidgetAction>

// mitk core
#include <mitkDataStorage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// mitk qt widgets
#include <QmitkRenderWindow.h>
#include <QmitkStyleManager.h>

// itk
#include <itkSpatialOrientationAdapter.h>

QmitkRenderWindowUtilityWidget::QmitkRenderWindowUtilityWidget(
  QWidget* parent/* = nullptr */,
  QmitkRenderWindow* renderWindow/* = nullptr */,
  mitk::DataStorage* dataStorage/* = nullptr */)
  : m_NodeSelectionWidget(nullptr)
  , m_SliceNavigationWidget(nullptr)
  , m_StepperAdapter(nullptr)
  , m_ViewDirectionSelector(nullptr)
{
  this->setParent(parent);
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);

  mitk::NodePredicateAnd::Pointer noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_BaseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());

  m_NodeSelectionWidget = new QmitkSynchronizedNodeSelectionWidget(parent);
  m_NodeSelectionWidget->SetBaseRenderer(m_BaseRenderer);
  m_NodeSelectionWidget->SetDataStorage(dataStorage);
  m_NodeSelectionWidget->SetNodePredicate(noHelperObjects);

  auto menuBar = new QMenuBar(this);
  menuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  auto dataMenu = menuBar->addMenu("Data");
  QWidgetAction* dataAction = new QWidgetAction(dataMenu);
  dataAction->setDefaultWidget(m_NodeSelectionWidget);
  dataMenu->addAction(dataAction);
  layout->addWidget(menuBar);

  auto* synchPushButton = new QPushButton(this);
  auto* synchIcon = new QIcon();
  auto synchronizeSvg = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg"));
  auto desynchronizeSvg = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg"));
  synchIcon->addPixmap(synchronizeSvg.pixmap(64), QIcon::Normal, QIcon::On);
  synchIcon->addPixmap(desynchronizeSvg.pixmap(64), QIcon::Normal, QIcon::Off);
  synchPushButton->setIcon(*synchIcon);
  synchPushButton->setToolTip("Synchronize / desynchronize data management");
  synchPushButton->setCheckable(true);
  synchPushButton->setChecked(true);
  connect(synchPushButton, &QPushButton::clicked,
    this, &QmitkRenderWindowUtilityWidget::ToggleSynchronization);
  layout->addWidget(synchPushButton);

  auto* sliceNavigationController = m_BaseRenderer->GetSliceNavigationController();
  m_SliceNavigationWidget = new QmitkSliceNavigationWidget(this);
  m_StepperAdapter =
    new QmitkStepperAdapter(m_SliceNavigationWidget, sliceNavigationController->GetSlice());
  layout->addWidget(m_SliceNavigationWidget);

  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer{ m_BaseRenderer };
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_ViewDirectionSelector = new QComboBox(this);
  QStringList viewDirections{ "axial", "coronal", "sagittal"};
  m_ViewDirectionSelector->insertItems(0, viewDirections);
  connect(m_ViewDirectionSelector, &QComboBox::currentTextChanged, this, &QmitkRenderWindowUtilityWidget::ChangeViewDirection);
  UpdateViewPlaneSelection();

  layout->addWidget(m_ViewDirectionSelector);

  // finally add observer, after all relevant objects have been created / initialized
  sliceNavigationController->ConnectGeometrySendEvent(this);
}

QmitkRenderWindowUtilityWidget::~QmitkRenderWindowUtilityWidget()
{
}

void QmitkRenderWindowUtilityWidget::ToggleSynchronization(bool synchronized)
{
  m_NodeSelectionWidget->SetSynchronized(synchronized);
  emit SynchronizationToggled(m_NodeSelectionWidget);
}

void QmitkRenderWindowUtilityWidget::SetGeometry(const itk::EventObject& event)
{
  if (!mitk::SliceNavigationController::GeometrySendEvent(nullptr, 0).CheckEvent(&event))
  {
    return;
  }

  const auto* sliceNavigationController = m_BaseRenderer->GetSliceNavigationController();
  auto viewDirection = sliceNavigationController->GetViewDirection();
  unsigned int axis = 0;
  switch (viewDirection)
  {
  case mitk::AnatomicalPlane::Original:
    return;
  case mitk::AnatomicalPlane::Axial:
  {
    axis = 2;
    break;
  }
  case mitk::AnatomicalPlane::Coronal:
  {
    axis = 1;
    break;
  }
  case mitk::AnatomicalPlane::Sagittal:
  {
    axis = 0;
    break;
  }
  }

  const auto* inputTimeGeometry = sliceNavigationController->GetInputWorldTimeGeometry();
  const mitk::BaseGeometry* rendererGeometry = m_BaseRenderer->GetCurrentWorldGeometry();

  mitk::TimeStepType timeStep = sliceNavigationController->GetTime()->GetPos();
  mitk::BaseGeometry::ConstPointer geometry = inputTimeGeometry->GetGeometryForTimeStep(timeStep);

  mitk::AffineTransform3D::MatrixType matrix = geometry->GetIndexToWorldTransform()->GetMatrix();
  matrix.GetVnlMatrix().normalize_columns();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetInverse();

  int dominantAxis = itk::Function::Max3(inverseMatrix[0][axis], inverseMatrix[1][axis], inverseMatrix[2][axis]);

  bool referenceGeometryAxisInverted = inverseMatrix[dominantAxis][axis] < 0;
  bool rendererZAxisInverted = rendererGeometry->GetAxisVector(2)[axis] < 0;

  m_SliceNavigationWidget->SetInverseDirection(referenceGeometryAxisInverted != rendererZAxisInverted);
}

void QmitkRenderWindowUtilityWidget::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString());
}

void QmitkRenderWindowUtilityWidget::UpdateViewPlaneSelection()
{
  const auto sliceNavigationController = m_BaseRenderer->GetSliceNavigationController();
  const auto viewDirection = sliceNavigationController->GetDefaultViewDirection();
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
}
