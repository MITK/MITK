/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowWidget.h"

#include <mitkImage.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// itk
#include <itkSpatialOrientationAdapter.h>

// vtk
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>

QmitkRenderWindowWidget::QmitkRenderWindowWidget(QWidget* parent/* = nullptr*/,
                                                 const QString& widgetName/* = ""*/,
                                                 mitk::DataStorage* dataStorage/* = nullptr*/,
                                                 bool windowControls/* = false */)
  : QFrame(parent)
  , m_WidgetName(widgetName)
  , m_DataStorage(dataStorage)
  , m_RenderWindow(nullptr)
  , m_CrosshairManager(nullptr)
  , m_UtilityWidget(nullptr)
  , m_WindowControls(windowControls)
{
  this->InitializeGUI();
}

QmitkRenderWindowWidget::~QmitkRenderWindowWidget()
{
  auto sliceNavigationController = this->GetSliceNavigationController();
  if (nullptr != sliceNavigationController)
  {
    sliceNavigationController->SetCrosshairEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkRenderWindowWidget, const mitk::Point3D &>(
        this, &QmitkRenderWindowWidget::SetCrosshairPosition));
  }
}

void QmitkRenderWindowWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (dataStorage == m_DataStorage)
  {
    return;
  }

  m_DataStorage = dataStorage;
  if (nullptr != m_RenderWindow)
  {
    mitk::BaseRenderer::GetInstance(m_RenderWindow->renderWindow())->SetDataStorage(dataStorage);
  }

  m_CrosshairManager->SetDataStorage(m_DataStorage);
}

mitk::SliceNavigationController* QmitkRenderWindowWidget::GetSliceNavigationController() const
{
  return m_RenderWindow->GetSliceNavigationController();
}

void QmitkRenderWindowWidget::RequestUpdate()
{
  mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow->renderWindow());
}

void QmitkRenderWindowWidget::ForceImmediateUpdate()
{
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(m_RenderWindow->renderWindow());
}

void QmitkRenderWindowWidget::SetGradientBackgroundColors(const mitk::Color& upper, const mitk::Color& lower)
{
  vtkRenderer* vtkRenderer = m_RenderWindow->GetRenderer()->GetVtkRenderer();
  if (nullptr == vtkRenderer)
  {
    return;
  }

  m_GradientBackgroundColors.first = upper;
  m_GradientBackgroundColors.second = lower;
  vtkRenderer->SetBackground(lower[0], lower[1], lower[2]);
  vtkRenderer->SetBackground2(upper[0], upper[1], upper[2]);

  ShowGradientBackground(true);
}

void QmitkRenderWindowWidget::ShowGradientBackground(bool show)
{
  m_RenderWindow->GetRenderer()->GetVtkRenderer()->SetGradientBackground(show);
}

bool QmitkRenderWindowWidget::IsGradientBackgroundOn() const
{
  return m_RenderWindow->GetRenderer()->GetVtkRenderer()->GetGradientBackground();
}

void QmitkRenderWindowWidget::SetDecorationColor(const mitk::Color& color)
{
  m_DecorationColor = color;
  m_CornerAnnotation->GetTextProperty()->SetColor(m_DecorationColor[0], m_DecorationColor[1], m_DecorationColor[2]);

  QColor hexColor(m_DecorationColor[0] * 255, m_DecorationColor[1] * 255, m_DecorationColor[2] * 255);
  setStyleSheet("QmitkRenderWindowWidget { border: 2px solid " + hexColor.name(QColor::HexRgb) + "; }");
}

void QmitkRenderWindowWidget::ShowColoredRectangle(bool show)
{
  if (show)
  {
    setFrameStyle(QFrame::Box | QFrame::Plain);
  }
  else
  {
    setFrameStyle(NoFrame);
  }
}

bool QmitkRenderWindowWidget::IsColoredRectangleVisible() const
{
  return frameStyle() > 0;
}

void QmitkRenderWindowWidget::ShowCornerAnnotation(bool show)
{
  m_CornerAnnotation->SetVisibility(show);
}

bool QmitkRenderWindowWidget::IsCornerAnnotationVisible() const
{
  return m_CornerAnnotation->GetVisibility() > 0;
}

void QmitkRenderWindowWidget::SetCornerAnnotationText(const std::string& cornerAnnotation)
{
  m_CornerAnnotation->SetText(0, cornerAnnotation.c_str());
}

std::string QmitkRenderWindowWidget::GetCornerAnnotationText() const
{
  return std::string(m_CornerAnnotation->GetText(0));
}

bool QmitkRenderWindowWidget::IsRenderWindowMenuActivated() const
{
  return m_RenderWindow->GetActivateMenuWidgetFlag();
}

void QmitkRenderWindowWidget::SetCrosshairVisibility(bool visible)
{
  m_CrosshairManager->SetCrosshairVisibility(visible);
  this->RequestUpdate();
}

bool QmitkRenderWindowWidget::GetCrosshairVisibility()
{
  return m_CrosshairManager->GetCrosshairVisibility();
}

void QmitkRenderWindowWidget::SetCrosshairGap(unsigned int gapSize)
{
  m_CrosshairManager->SetCrosshairGap(gapSize);
}

void QmitkRenderWindowWidget::AddPlanesToDataStorage()
{
  m_CrosshairManager->AddPlanesToDataStorage();
}

void QmitkRenderWindowWidget::RemovePlanesFromDataStorage()
{
  m_CrosshairManager->RemovePlanesFromDataStorage();
}

void QmitkRenderWindowWidget::InitializeGUI()
{
  m_Layout = new QVBoxLayout(this);
  m_Layout->setMargin(0);
  setLayout(m_Layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setContentsMargins(0, 0, 0, 0);

  if (nullptr == m_DataStorage)
  {
    return;
  }

  mitk::RenderingManager::GetInstance()->SetDataStorage(m_DataStorage);

  // create render window for this render window widget
  m_RenderWindow = new QmitkRenderWindow(this, m_WidgetName, nullptr);
  m_RenderWindow->SetLayoutIndex(mitk::AnatomicalPlane::Sagittal);

  auto sliceNavigationController = this->GetSliceNavigationController();
  sliceNavigationController->SetDefaultViewDirection(mitk::AnatomicalPlane::Sagittal);

  if (m_WindowControls)
  {
    m_UtilityWidget = new QmitkRenderWindowUtilityWidget(this, m_RenderWindow, m_DataStorage);
    m_Layout->addWidget(m_UtilityWidget);
    connect(m_UtilityWidget, &QmitkRenderWindowUtilityWidget::ReinitAction,
      this, &QmitkRenderWindowWidget::OnReinitAction);
    connect(m_UtilityWidget, &QmitkRenderWindowUtilityWidget::ResetAction,
      this, &QmitkRenderWindowWidget::OnResetAction);
  }

  m_Layout->addWidget(m_RenderWindow);

  // set colors and corner annotation
  InitializeDecorations();

  // use crosshair manager
  m_CrosshairManager = mitk::CrosshairManager::New(m_DataStorage, m_RenderWindow->GetRenderer());
  sliceNavigationController->SetCrosshairEvent.AddListener(
    mitk::MessageDelegate1<QmitkRenderWindowWidget, const mitk::Point3D &>(
      this, &QmitkRenderWindowWidget::SetCrosshairPosition));

  // finally add observer, after all relevant objects have been created / initialized
  sliceNavigationController->ConnectGeometrySendEvent(this);
  sliceNavigationController->ConnectGeometrySliceEvent(this);

  mitk::TimeGeometry::ConstPointer timeGeometry = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeView(m_RenderWindow->GetVtkRenderWindow(), timeGeometry);
}

void QmitkRenderWindowWidget::InitializeDecorations()
{
  vtkRenderer* vtkRenderer = m_RenderWindow->GetRenderer()->GetVtkRenderer();
  if (nullptr == vtkRenderer)
  {
    return;
  }

  // initialize background color gradients
  float black[3] = { 0.0f, 0.0f, 0.0f };
  SetGradientBackgroundColors(black, black);

  // initialize annotation text and decoration color
  setFrameStyle(QFrame::Box | QFrame::Plain);

  m_CornerAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
  m_CornerAnnotation->SetText(0, "Sagittal");
  m_CornerAnnotation->SetMaximumFontSize(12);
  if (0 == vtkRenderer->HasViewProp(m_CornerAnnotation))
  {
    vtkRenderer->AddViewProp(m_CornerAnnotation);
  }

  float white[3] = { 1.0f, 1.0f, 1.0f };
  SetDecorationColor(mitk::Color(white));
}

void QmitkRenderWindowWidget::SetCrosshairPosition(const mitk::Point3D& newPosition)
{
  m_CrosshairManager->SetCrosshairPosition(newPosition);
  this->RequestUpdate();
}

mitk::Point3D QmitkRenderWindowWidget::GetCrosshairPosition() const
{
  return m_CrosshairManager->GetCrosshairPosition();
}

void QmitkRenderWindowWidget::SetGeometry(const itk::EventObject& event)
{
  if (!mitk::SliceNavigationController::GeometrySendEvent(nullptr, 0).CheckEvent(&event))
  {
    return;
  }

  auto sliceNavigationController = this->GetSliceNavigationController();
  const auto* inputTimeGeometry = sliceNavigationController->GetInputWorldTimeGeometry();
  m_CrosshairManager->ComputeOrientedTimeGeometries(inputTimeGeometry);

  if (m_WindowControls)
  {
    this->ComputeInvertedSliceNavigation();
  }
}

void QmitkRenderWindowWidget::SetGeometrySlice(const itk::EventObject& event)
{
  if (!mitk::SliceNavigationController::GeometrySliceEvent(nullptr, 0).CheckEvent(&event))
  {
    return;
  }

  auto sliceNavigationController = this->GetSliceNavigationController();
  m_CrosshairManager->UpdateSlice(sliceNavigationController);
}

void QmitkRenderWindowWidget::ComputeInvertedSliceNavigation()
{
  auto sliceNavigationController = this->GetSliceNavigationController();
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
  const mitk::BaseGeometry* rendererGeometry = m_RenderWindow->GetRenderer()->GetCurrentWorldGeometry();

  // todo: check timepoint / timestep
  mitk::TimeStepType timeStep = sliceNavigationController->GetTime()->GetPos();
  mitk::BaseGeometry::ConstPointer geometry = inputTimeGeometry->GetGeometryForTimeStep(timeStep);

  mitk::AffineTransform3D::MatrixType matrix = geometry->GetIndexToWorldTransform()->GetMatrix();
  matrix.GetVnlMatrix().normalize_columns();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetInverse();

  int dominantAxis = itk::Function::Max3(inverseMatrix[0][axis], inverseMatrix[1][axis], inverseMatrix[2][axis]);

  bool referenceGeometryAxisInverted = inverseMatrix[dominantAxis][axis] < 0;
  bool rendererZAxisInverted = rendererGeometry->GetAxisVector(2)[axis] < 0;

  m_UtilityWidget->SetInvertedSliceNavigation(referenceGeometryAxisInverted != rendererZAxisInverted);
}

void QmitkRenderWindowWidget::OnReinitAction(QList<mitk::DataNode::Pointer> selectedNodes)
{
  if (selectedNodes.empty())
  {
    return;
  }

  auto* baseRenderer = mitk::BaseRenderer::GetInstance(m_RenderWindow->renderWindow());
  auto boundingBoxPredicate = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false), baseRenderer));
  mitk::DataStorage::SetOfObjects::Pointer nodes = mitk::DataStorage::SetOfObjects::New();
  for (const auto& dataNode : selectedNodes)
  {
    if (boundingBoxPredicate->CheckNode(dataNode))
    {
      nodes->InsertElement(nodes->Size(), dataNode);
    }
  }

  if (nodes->empty())
  {
    return;
  }

  if (1 == nodes->Size())
  {
    auto selectedImage = dynamic_cast<mitk::Image*>(nodes->ElementAt(0)->GetData());

    if (nullptr != selectedImage)
    {
      mitk::RenderingManager::GetInstance()->InitializeView(baseRenderer->GetRenderWindow(), selectedImage->GetTimeGeometry());
      return;
    }
  }

  auto boundingGeometry = m_DataStorage->ComputeBoundingGeometry3D(nodes, "visible", baseRenderer);
  mitk::RenderingManager::GetInstance()->InitializeView(baseRenderer->GetRenderWindow(), boundingGeometry);
}

void QmitkRenderWindowWidget::OnResetAction(QList<mitk::DataNode::Pointer> selectedNodes)
{
  if (selectedNodes.empty())
  {
    return;
  }

  auto selectedImage = dynamic_cast<mitk::Image*>(selectedNodes.front()->GetData());
  if (nullptr == selectedImage)
  {
    return;
  }

  const mitk::TimeGeometry* referenceGeometry = selectedImage->GetTimeGeometry();
  if (nullptr == referenceGeometry)
  {
    return;
  }

  mitk::TimeStepType imageTimeStep = 0;

  // store the current position to set it again later, if the camera should not be reset
  mitk::Point3D currentPosition = this->GetCrosshairPosition();

  // store the current time step to set it again later, if the camera should not be reset
  auto* renderingManager = mitk::RenderingManager::GetInstance();
  const auto currentTimePoint = renderingManager->GetTimeNavigationController()->GetSelectedTimePoint();
  if (referenceGeometry->IsValidTimePoint(currentTimePoint))
  {
    imageTimeStep = referenceGeometry->TimePointToTimeStep(currentTimePoint);
  }

  auto* baseRenderer = mitk::BaseRenderer::GetInstance(m_RenderWindow->renderWindow());
  renderingManager->InitializeView(baseRenderer->GetRenderWindow(), referenceGeometry, false);

  // reset position and time step
  this->GetSliceNavigationController()->SelectSliceByPoint(currentPosition);
  renderingManager->GetTimeNavigationController()->GetTime()->SetPos(imageTimeStep);
}
