/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkRenderWindowWidget.h"

// mitk qt widgets
#include <QmitkMxNMultiWidget.h>

// vtk
#include <vtkTextProperty.h>

QmitkRenderWindowWidget::QmitkRenderWindowWidget(QWidget* parent/* = nullptr*/,
                                                 const QString& widgetName/* = ""*/,
                                                 mitk::DataStorage* dataStorage/* = nullptr*/,
                                                 mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/)
  : QWidget(parent)
  , m_WidgetName(widgetName)
  , m_DataStorage(dataStorage)
  , m_RenderWindow(nullptr)
  , m_RenderingMode(renderingMode)
  , m_PointSetNode(nullptr)
  , m_PointSet(nullptr)
{
  InitializeGUI();
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
    mitk::BaseRenderer::GetInstance(m_RenderWindow->GetRenderWindow())->SetDataStorage(dataStorage);
  }
}

mitk::SliceNavigationController* QmitkRenderWindowWidget::GetSliceNavigationController() const
{
  return m_RenderWindow->GetSliceNavigationController();
}

void QmitkRenderWindowWidget::RequestUpdate()
{
  m_RenderingManager->RequestUpdate(m_RenderWindow->GetRenderWindow());
}

void QmitkRenderWindowWidget::ForceImmediateUpdate()
{
  m_RenderingManager->ForceImmediateUpdate(m_RenderWindow->GetRenderWindow());
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
  m_RectangleProp->SetColor(m_DecorationColor[0], m_DecorationColor[1], m_DecorationColor[2]);
  m_CornerAnnotation->GetTextProperty()->SetColor(color[0], color[1], color[2]);
}

void QmitkRenderWindowWidget::ShowColoredRectangle(bool show)
{
  m_RectangleProp->SetVisibility(show);
}

bool QmitkRenderWindowWidget::IsColoredRectangleVisible() const
{
  return m_RectangleProp->GetVisibility() > 0;
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

  // create render window for this render window widget
  m_RenderingManager = mitk::RenderingManager::GetInstance();
  //m_RenderingManager = mitk::RenderingManager::New();
  m_RenderingManager->SetDataStorage(m_DataStorage);

  m_RenderWindow = new QmitkRenderWindow(this, m_WidgetName, nullptr, m_RenderingManager, m_RenderingMode);
  m_RenderWindow->SetLayoutIndex(mitk::SliceNavigationController::Sagittal);
  m_RenderWindow->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);
  m_RenderWindow->GetSliceNavigationController()->SetRenderingManager(m_RenderingManager);
  m_RenderWindow->GetSliceNavigationController()->SetCrosshairEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindowWidget, mitk::Point3D>(this, &QmitkRenderWindowWidget::SetCrosshair));

  mitk::TimeGeometry::ConstPointer timeGeometry = m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll());
  m_RenderingManager->InitializeViews(timeGeometry);
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
  m_RectangleProp = vtkSmartPointer<vtkMitkRectangleProp>::New();
  m_RectangleProp->SetColor(m_DecorationColor[0], m_DecorationColor[1], m_DecorationColor[2]);
  if (0 == vtkRenderer->HasViewProp(m_RectangleProp))
  {
    vtkRenderer->AddViewProp(m_RectangleProp);
  }

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
  mitk::RenderingManager::GetInstance()->RequestUpdate(m_RenderWindow->GetRenderWindow());
}
