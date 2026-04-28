/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderWindowUtilityWidget.h>

#include <QToolButton>
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
  mitk::DataStorage* dataStorage/* = nullptr */,
  const int nSyncGroups/* = 1 */)
  : m_NodeSelectionWidget(nullptr)
  , m_SyncGroupSelector(nullptr)
  , m_NewSyncGroupButton(nullptr)
  , m_SliceNavigationWidget(nullptr)
  , m_StepperAdapter(nullptr)
  , m_ViewDirectionSelector(nullptr)
{
  this->setParent(parent);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});

  mitk::NodePredicateAnd::Pointer noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

  m_BaseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());

  m_NodeSelectionWidget = new QmitkSynchronizedNodeSelectionWidget(parent);
  m_NodeSelectionWidget->SetBaseRenderer(m_BaseRenderer);
  m_NodeSelectionWidget->SetDataStorage(dataStorage);
  m_NodeSelectionWidget->SetNodePredicate(noHelperObjects);
  connect(this, &QmitkRenderWindowUtilityWidget::SetDataSelection, m_NodeSelectionWidget, &QmitkSynchronizedNodeSelectionWidget::SetSelection);

  auto menuBar = new QMenuBar(this);
  menuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  menuBar->setNativeMenuBar(false);
  auto dataMenu = menuBar->addMenu("Data");
  QWidgetAction* dataAction = new QWidgetAction(dataMenu);
  dataAction->setDefaultWidget(m_NodeSelectionWidget);
  dataMenu->addAction(dataAction);
  layout->addWidget(menuBar);

  m_SyncGroupSelector = new QComboBox(this);
  // Each combobox row carries its group index as userData (QVariant), so that
  // sparse / non-monotonic group indices map correctly. Row position is
  // never used as a proxy for the group number.
  for (int i = 0; i < nSyncGroups; ++i)
  {
    const GroupSyncIndexType groupIndex = i + 1;
    m_SyncGroupSelector->insertItem(i, QString("Group %1").arg(groupIndex), QVariant(groupIndex));
  }
  m_SyncGroupSelector->setMinimumContentsLength(8);
  connect(m_SyncGroupSelector, &QComboBox::currentIndexChanged,
    this, &QmitkRenderWindowUtilityWidget::OnSyncGroupSelectionChanged);
  layout->addWidget(m_SyncGroupSelector);

  // The combobox is a passive view of existing groups. New-group creation goes
  // through a separate button so the combobox no longer drives lifecycle.
  m_NewSyncGroupButton = new QToolButton(this);
  m_NewSyncGroupButton->setText("+");
  m_NewSyncGroupButton->setToolTip(tr("Create a new synchronization group"));
  connect(m_NewSyncGroupButton, &QToolButton::clicked, this, [this]() {
    emit CreateNewSyncGroupRequested(m_NodeSelectionWidget);
  });
  layout->addWidget(m_NewSyncGroupButton);

  auto* sliceNavigationController = m_BaseRenderer->GetSliceNavigationController();
  m_SliceNavigationWidget = new QmitkSliceNavigationWidget(this);
  m_StepperAdapter =
    new QmitkStepperAdapter(m_SliceNavigationWidget, sliceNavigationController->GetStepper());
  layout->addWidget(m_SliceNavigationWidget);

  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer{ m_BaseRenderer };
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_ViewDirectionSelector = new QComboBox(this);
  QStringList viewDirections{ "axial", "coronal", "sagittal"};
  m_ViewDirectionSelector->insertItems(0, viewDirections);
  m_ViewDirectionSelector->setMinimumContentsLength(7);
  connect(m_ViewDirectionSelector, &QComboBox::currentTextChanged, this, &QmitkRenderWindowUtilityWidget::ChangeViewDirection);
  UpdateViewPlaneSelection();

  layout->addWidget(m_ViewDirectionSelector);

  // finally add observer, after all relevant objects have been created / initialized
  sliceNavigationController->ConnectGeometrySendEvent(this);
}

QmitkRenderWindowUtilityWidget::~QmitkRenderWindowUtilityWidget()
{
}

void QmitkRenderWindowUtilityWidget::SetSyncGroup(const GroupSyncIndexType index)
{
  if (index == 0)
  {
    MITK_ERROR << "Invalid call to SetSyncGroup. Group index can't be 0.";
    return;
  }
  // Locate the combobox row that carries this group index in its userData and
  // select it. setCurrentIndex(-1) (no match) is a deliberate no-op: the group
  // exists in the model but not yet in this combobox's view, in which case
  // OnSyncGroupAdded will add it later and the caller can reissue.
  const int row = m_SyncGroupSelector->findData(QVariant(index));
  m_SyncGroupSelector->setCurrentIndex(row);
}

QmitkRenderWindowUtilityWidget::GroupSyncIndexType QmitkRenderWindowUtilityWidget::GetSyncGroup() const
{
  // Read the group index from the selected row's userData rather than from the
  // row position (which would conflate combobox layout with the group's logical
  // identifier when groups are sparse).
  const QVariant data = m_SyncGroupSelector->currentData();
  return data.isValid() ? data.toInt() : -1;
}

void QmitkRenderWindowUtilityWidget::OnSyncGroupSelectionChanged(int index)
{
  // Pure follower: report the selection. Group creation goes through the
  // '+' button (CreateNewSyncGroupRequested), not through the combobox.
  if (index < 0)
  {
    return;
  }
  const QVariant data = m_SyncGroupSelector->itemData(index);
  if (!data.isValid())
  {
    return;
  }
  emit SyncGroupChanged(m_NodeSelectionWidget, data.toInt());
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

  mitk::TimeStepType timeStep = sliceNavigationController->GetStepper()->GetPos();
  mitk::BaseGeometry::ConstPointer geometry = inputTimeGeometry->GetGeometryForTimeStep(timeStep);
  if (geometry == nullptr)
    return;

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

QmitkSynchronizedNodeSelectionWidget* QmitkRenderWindowUtilityWidget::GetNodeSelectionWidget() const
{
  return m_NodeSelectionWidget;
}

void QmitkRenderWindowUtilityWidget::OnSyncGroupAdded(const GroupSyncIndexType index)
{
  // Reactive growth: append a row carrying this group index as userData. We
  // de-dupe by data (not by position) so sparse / non-monotonic group indices
  // map correctly. Appending preserves currentIndex and does not fire a
  // selection signal.
  if (m_SyncGroupSelector->findData(QVariant(index)) >= 0)
  {
    return;
  }
  m_SyncGroupSelector->addItem(QString("Group %1").arg(index), QVariant(index));
}
