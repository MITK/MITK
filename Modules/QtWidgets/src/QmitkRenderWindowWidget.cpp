/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowWidget.h"

// vtk
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>

QmitkRenderWindowWidget::QmitkRenderWindowWidget(QWidget* parent/* = nullptr*/,
                                                 const QString& widgetName/* = ""*/,
                                                 mitk::DataStorage* dataStorage/* = nullptr*/)
  : QFrame(parent)
  , m_WidgetName(widgetName)
  , m_DataStorage(dataStorage)
  , m_RenderWindow(nullptr)
  , m_PointSetNode(nullptr)
  , m_PointSet(nullptr)
{
  this->InitializeGUI();
}

QmitkRenderWindowWidget::~QmitkRenderWindowWidget()
{
  auto sliceNavigationController = m_RenderWindow->GetSliceNavigationController();
  if (nullptr != sliceNavigationController)
  {
    sliceNavigationController->SetCrosshairEvent.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowWidget, mitk::Point3D>(this, &QmitkRenderWindowWidget::SetCrosshair));
  }
  if (nullptr != m_DataStorage)
  {
    m_DataStorage->Remove(m_PointSetNode);
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

void QmitkRenderWindowWidget::ActivateCrosshair(bool activate)
{
  if (nullptr == m_DataStorage)
  {
    return;
  }

  if (activate)
  {
    try
    {
      m_DataStorage->Add(m_PointSetNode);
    }
    catch(std::invalid_argument& /*e*/)
    {
      // crosshair already existing
      return;
    }
  }
  else
  {
    m_DataStorage->Remove(m_PointSetNode);
  }
}

void QmitkRenderWindowWidget::InitializeGUI()
{
  m_Layout = new QHBoxLayout(this);
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
  m_RenderWindow->SetLayoutIndex(mitk::BaseRenderer::ViewDirection::SAGITTAL);
  m_RenderWindow->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
  m_RenderWindow->GetSliceNavigationController()->SetCrosshairEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindowWidget, mitk::Point3D>(this, &QmitkRenderWindowWidget::SetCrosshair));

  connect(m_RenderWindow, &QmitkRenderWindow::mouseEvent, this, &QmitkRenderWindowWidget::MouseEvent);

  mitk::TimeGeometry::ConstPointer timeGeometry = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(timeGeometry);
  m_Layout->addWidget(m_RenderWindow);

  // add point set as a crosshair
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->SetProperty("name", mitk::StringProperty::New("Crosshair of render window " + m_WidgetName.toStdString()));
  m_PointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true)); // crosshair-node should typically be invisible

  // set the crosshair only visible for this specific renderer
  m_PointSetNode->SetBoolProperty("fixedLayer", true, m_RenderWindow->GetRenderer());
  m_PointSetNode->SetVisibility(true, m_RenderWindow->GetRenderer());
  m_PointSetNode->SetVisibility(false);

  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);

  // set colors and corner annotation
  InitializeDecorations();
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

  // initialize decoration color, rectangle and annotation text
  float white[3] = { 1.0f, 1.0f, 1.0f };
  m_DecorationColor = white;

  setFrameStyle(QFrame::Box | QFrame::Plain);
  QColor hexColor(m_DecorationColor[0] * 255, m_DecorationColor[1] * 255, m_DecorationColor[2] * 255);
  setStyleSheet("border: 2px solid " + hexColor.name(QColor::HexRgb));

  m_CornerAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
  m_CornerAnnotation->SetText(0, "Sagittal");
  m_CornerAnnotation->SetMaximumFontSize(12);
  m_CornerAnnotation->GetTextProperty()->SetColor(m_DecorationColor[0], m_DecorationColor[1], m_DecorationColor[2]);
  if (0 == vtkRenderer->HasViewProp(m_CornerAnnotation))
  {
    vtkRenderer->AddViewProp(m_CornerAnnotation);
  }
}

void QmitkRenderWindowWidget::SetCrosshair(mitk::Point3D selectedPoint)
{
  m_PointSet->SetPoint(1, selectedPoint, 0);
  mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow->renderWindow());
}
