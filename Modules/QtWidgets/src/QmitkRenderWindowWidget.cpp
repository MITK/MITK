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

// vtk
#include <vtkTextProperty.h>

// qt widgets module
#include <QmitkCustomMultiWidget.h>

QmitkRenderWindowWidget::QmitkRenderWindowWidget(QWidget* parent/* = nullptr*/, const QString& UID/* = ""*/, mitk::DataStorage* dataStorage/* = nullptr*/)
  : QWidget(parent)
  , m_UID(UID)
  , m_DataStorage(dataStorage)
  , m_RenderWindow(nullptr)
  , m_LevelWindowWidget(nullptr)
{
  InitializeGUI();
  // TODO: signals / slots
}

QmitkRenderWindowWidget::~QmitkRenderWindowWidget()
{
  // nothing here
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
  if (nullptr == m_RenderWindow)
  {
    return nullptr;
  }

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

void QmitkRenderWindowWidget::SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower)
{
  vtkRenderer* vtkRenderer = m_RenderWindow->GetRenderer()->GetVtkRenderer();
  if (nullptr == vtkRenderer)
  {
    return;
  }

  m_BackgroundColorGradient.first = upper;
  m_BackgroundColorGradient.second = lower;
  vtkRenderer->SetBackground(lower[0], lower[1], lower[2]);
  vtkRenderer->SetBackground2(upper[0], upper[1], upper[2]);

  ShowBackgroundColorGradient(true);
}

void QmitkRenderWindowWidget::ShowBackgroundColorGradient(bool show)
{
  if (show)
  {
    m_RenderWindow->GetRenderer()->GetVtkRenderer()->GradientBackgroundOn();
  }
  else
  {
    m_RenderWindow->GetRenderer()->GetVtkRenderer()->GradientBackgroundOff();

  }
  m_BackgroundColorGradientFlag = show;
}

void QmitkRenderWindowWidget::ShowLevelWindowWidget(bool show)
{
  m_LevelWindowWidget->disconnect(this);
  if (show)
  {
    m_LevelWindowWidget->SetDataStorage(m_DataStorage);
    m_LevelWindowWidget->show();
  }
  else
  {
    m_LevelWindowWidget->disconnect(this);
    m_LevelWindowWidget->hide();
  }
}

void QmitkRenderWindowWidget::ShowDepartmentLogo(bool show)
{
  m_LogoAnnotation->SetVisibility(show);
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

void QmitkRenderWindowWidget::InitializeGUI()
{
  m_Layout = new QHBoxLayout(this);
  m_Layout->setMargin(0);
  setLayout(m_Layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // create render window for this render window widget
  m_RenderingManager = mitk::RenderingManager::GetInstance(); // TODO: do not always use the standard global rendering manager
  m_RenderingMode = mitk::BaseRenderer::RenderingMode::Standard; // TODO: do not always use the standard rendering mode
  m_RenderWindow = new QmitkRenderWindow(this, m_UID, nullptr, m_RenderingManager, m_RenderingMode);
  m_RenderWindow->SetLayoutIndex(QmitkCustomMultiWidget::SAGITTAL); // TODO: allow to change layout type later
  m_RenderWindow->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Sagittal);

  // create level window widget for this render window widget
  m_LevelWindowWidget = new QmitkLevelWindowWidget(this);
  //m_LevelWindowWidget->setObjectName(QString::fromUtf8("levelWindowWidget"));
  QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(m_LevelWindowWidget->sizePolicy().hasHeightForWidth());
  m_LevelWindowWidget->setSizePolicy(sizePolicy);
  m_LevelWindowWidget->setMaximumWidth(50);

  m_Layout->addWidget(m_RenderWindow);
  m_Layout->addWidget(m_LevelWindowWidget);

  // set colors, add logo etc.
  InitializeDecorations();

  if (m_RenderingManager == nullptr)
  {
    m_RenderingManager = mitk::RenderingManager::GetInstance();
  }
  m_TimeNavigationController = m_RenderingManager->GetTimeNavigationController();

  // connect to the "time navigation controller": send time via sliceNavigationControllers
  m_TimeNavigationController->ConnectGeometryTimeEvent(GetRenderWindow()->GetSliceNavigationController(), false);

  // reverse connection between sliceNavigationControllers and m_TimeNavigationController
  GetRenderWindow()->GetSliceNavigationController()->ConnectGeometryTimeEvent(m_TimeNavigationController, false);
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
  SetBackgroundColorGradient(black, black);

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