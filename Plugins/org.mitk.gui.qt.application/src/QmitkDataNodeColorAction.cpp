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

#include <QmitkDataNodeColorAction.h>

// mitk core
#include <mitkRenderingManager.h>

// qt
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>

QmitkDataNodeColorAction::QmitkDataNodeColorAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(workbenchPartSite)
{
  InitializeAction();
}

QmitkDataNodeColorAction::QmitkDataNodeColorAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite)
  : QWidgetAction(parent)
  , QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  InitializeAction();
}

void QmitkDataNodeColorAction::InitializeAction()
{
  m_ColorButton = new QPushButton;
  m_ColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  connect(m_ColorButton, &QPushButton::clicked, this, &QmitkDataNodeColorAction::OnColorChanged);

  QLabel* colorLabel = new QLabel(tr("Color: "));
  colorLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  QHBoxLayout* colorWidgetLayout = new QHBoxLayout;
  colorWidgetLayout->setContentsMargins(4, 4, 4, 4);
  colorWidgetLayout->addWidget(colorLabel);
  colorWidgetLayout->addWidget(m_ColorButton);

  QWidget* colorWidget = new QWidget;
  colorWidget->setLayout(colorWidgetLayout);

  setDefaultWidget(colorWidget);

  connect(this, &QmitkDataNodeColorAction::changed, this, &QmitkDataNodeColorAction::OnActionChanged);
}

void QmitkDataNodeColorAction::InitializeWithDataNode(const mitk::DataNode* dataNode)
{
  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  float rgb[3];
  if (dataNode->GetColor(rgb, baseRenderer))
  {
    QColor color(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
    QString styleSheet = QString("background-color: ") + color.name(QColor::HexRgb);
    m_ColorButton->setAutoFillBackground(true);
    m_ColorButton->setStyleSheet(styleSheet);
  }
}

void QmitkDataNodeColorAction::OnColorChanged()
{
  auto dataNodes = GetSelectedNodes();
  if (dataNodes.isEmpty())
  {
    return;
  }

  mitk::BaseRenderer::Pointer baseRenderer = GetBaseRenderer();

  bool selectedColor = false;
  QColor newColor;

  for (auto& dataNode : dataNodes)
  {
    if (dataNode.IsNull())
    {
      continue;
    }

    float rgb[3];
    if (dataNode->GetColor(rgb, baseRenderer))
    {
      if (!selectedColor)
      {
        QColor initial(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
        newColor = QColorDialog::getColor(initial, nullptr, QString(tr("Change color")));

        if (newColor.isValid())
        {
          selectedColor = true;
        }
        else
        {
          return;
        }
      }

      dataNode->SetProperty("color", mitk::ColorProperty::New(newColor.redF(), newColor.greenF(), newColor.blueF()), baseRenderer);
      if (dataNode->GetProperty("binaryimage.selectedcolor", baseRenderer))
      {
        dataNode->SetProperty("binaryimage.selectedcolor", mitk::ColorProperty::New(newColor.redF(), newColor.greenF(), newColor.blueF()), baseRenderer);
      }
    }
  }

  if (nullptr == baseRenderer)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
  }
}

void QmitkDataNodeColorAction::OnActionChanged()
{
  auto dataNode = GetSelectedNode();
  if (dataNode.IsNull())
  {
    return;
  }

  InitializeWithDataNode(dataNode);
}
