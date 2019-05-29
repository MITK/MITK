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

// #define ENABLE_CTK_WIDGETS_WIDGET

#include "QmitkMultiLabelSegmentationUtilitiesView.h"
#include "BooleanOperations/QmitkBooleanOperationsWidget.h"
#include "MorphologicalOperations/QmitkMorphologicalOperationsWidget.h"
#include "SurfaceToImage/QmitkSurfaceToImageWidget.h"
#include "ImageMasking/QmitkImageMaskingWidget.h"
#include "ConvertToMl/QmitkConvertToMlWidget.h"

QmitkMultiLabelSegmentationUtilitiesView::QmitkMultiLabelSegmentationUtilitiesView() :
  m_BooleanOperationsWidget(nullptr),
  m_MorphologicalOperationsWidget(nullptr),
  m_SurfaceToImageWidget(nullptr),
  m_ImageMaskingWidget(nullptr),
  m_ConvertToMlWidget(nullptr)
{
}

QmitkMultiLabelSegmentationUtilitiesView::~QmitkMultiLabelSegmentationUtilitiesView()
{
}

void QmitkMultiLabelSegmentationUtilitiesView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  mitk::SliceNavigationController* timeNavigationController = renderWindowPart != nullptr
    ? renderWindowPart->GetTimeNavigationController()
    : nullptr;

  m_BooleanOperationsWidget = new QmitkBooleanOperationsWidget(timeNavigationController, parent);

  m_MorphologicalOperationsWidget = new QmitkMorphologicalOperationsWidget(timeNavigationController, parent);

  m_SurfaceToImageWidget = new QmitkSurfaceToImageWidget(timeNavigationController, parent);

  m_ImageMaskingWidget = new QmitkImageMaskingWidget(timeNavigationController, parent);

  m_ConvertToMlWidget = new QmitkConvertToMlWidget(timeNavigationController, parent);

  this->AddUtilityWidget(m_BooleanOperationsWidget, QIcon(":/MultiLabelSegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");

  this->AddUtilityWidget(m_MorphologicalOperationsWidget, QIcon(":/MultiLabelSegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");

  this->AddUtilityWidget(m_SurfaceToImageWidget, QIcon(":/MultiLabelSegmentationUtilities/SurfaceToImage_48x48.png"), "Surface To Image");

  this->AddUtilityWidget(m_ImageMaskingWidget, QIcon(":/MultiLabelSegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");

  this->AddUtilityWidget(m_ConvertToMlWidget, QIcon(":/MultiLabelSegmentationUtilities/multilabelsegmentation.svg"), "Convert To MultiLabel");
}

void QmitkMultiLabelSegmentationUtilitiesView::AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text)
{
  m_Controls.toolBox->addItem(widget, icon, text);
}

void QmitkMultiLabelSegmentationUtilitiesView::SetFocus()
{
  m_Controls.toolBox->setFocus();
}

void QmitkMultiLabelSegmentationUtilitiesView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  mitk::SliceNavigationController* timeNavigationController = renderWindowPart->GetTimeNavigationController();

  m_BooleanOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_SurfaceToImageWidget->SetTimeNavigationController(timeNavigationController);
  m_ImageMaskingWidget->SetTimeNavigationController(timeNavigationController);
  m_ConvertToMlWidget->SetTimeNavigationController(timeNavigationController);
}

void QmitkMultiLabelSegmentationUtilitiesView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  m_BooleanOperationsWidget->SetTimeNavigationController(nullptr);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(nullptr);
  m_SurfaceToImageWidget->SetTimeNavigationController(nullptr);
  m_ImageMaskingWidget->SetTimeNavigationController(nullptr);
  m_ConvertToMlWidget->SetTimeNavigationController(nullptr);
}
