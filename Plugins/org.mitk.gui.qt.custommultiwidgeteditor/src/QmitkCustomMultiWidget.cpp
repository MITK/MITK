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

// mitk qt widgets module
#include "QmitkCustomMultiWidget.h"
#include "QmitkRenderWindowWidget.h"

// qt
#include <QGridLayout>

QmitkCustomMultiWidget::QmitkCustomMultiWidget(QWidget* parent,
                                               Qt::WindowFlags f/* = 0*/,
                                               mitk::RenderingManager* renderingManager/* = nullptr*/,
                                               mitk::BaseRenderer::RenderingMode::Type renderingMode/* = mitk::BaseRenderer::RenderingMode::Standard*/,
                                               const QString& multiWidgetName/* = "custommulti"*/)
  : QmitkAbstractMultiWidget(parent, f, renderingManager, renderingMode, multiWidgetName)
  , m_GridLayout(nullptr)
{
  // nothing here
}

QmitkCustomMultiWidget::~QmitkCustomMultiWidget()
{
  // nothing here
}

void QmitkCustomMultiWidget::InitializeRenderWindowWidgets()
{
  SetLayout(1, 1);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidget::InitializeGUI()
{
  delete m_GridLayout;
  m_GridLayout = new QGridLayout(this);
  m_GridLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_GridLayout);

  FillMultiWidgetLayout();
  resize(QSize(364, 477).expandedTo(minimumSizeHint()));
}

void QmitkCustomMultiWidget::SetLayoutImpl()
{
  int requiredRenderWindowWidgets = GetRowCount() * GetColumnCount();
  int existingRenderWindowWidgets = GetRenderWindowWidgets().size();

  int difference = requiredRenderWindowWidgets - existingRenderWindowWidgets;
  while (0 < difference)
  {
    // more render window widgets needed
    CreateRenderWindowWidget();
    --difference;
  }
  while (0 > difference)
  {
    // less render window widgets needed
    RemoveRenderWindowWidget();
    ++difference;
  }

  InitializeGUI();
}

void QmitkCustomMultiWidget::CreateRenderWindowWidget(const std::string& cornerAnnotation/* = ""*/)
{
  // create the render window widget and connect signals / slots
  QString renderWindowWidgetName = GetNameFromIndex(GetRenderWindowWidgets().size());
  RenderWindowWidgetPointer renderWindowWidget = std::make_shared<QmitkRenderWindowWidget>(this, renderWindowWidgetName, GetDataStorage());
  renderWindowWidget->SetCornerAnnotationText(renderWindowWidgetName.toStdString()/*cornerAnnotation*/);

  AddRenderWindowWidget(renderWindowWidgetName, renderWindowWidget);
}

void QmitkCustomMultiWidget::FillMultiWidgetLayout()
{
  for (int row = 0; row < GetRowCount(); ++row)
  {
    for (int column = 0; column < GetColumnCount(); ++column)
    {
      RenderWindowWidgetPointer renderWindowWidget = GetRenderWindowWidget(row, column);
      if (nullptr != renderWindowWidget)
      {
        m_GridLayout->addWidget(renderWindowWidget.get(), row, column);
        SetActiveRenderWindowWidget(renderWindowWidget);
      }
    }
  }
}
