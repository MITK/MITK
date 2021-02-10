/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationUtilitiesView.h"
#include "BooleanOperations/QmitkBooleanOperationsWidget.h"
#include "ContourModelToImage/QmitkContourModelToImageWidget.h"
#include "ImageMasking/QmitkImageMaskingWidget.h"
#include "MorphologicalOperations/QmitkMorphologicalOperationsWidget.h"
#include "SurfaceToImage/QmitkSurfaceToImageWidget.h"

QmitkSegmentationUtilitiesView::QmitkSegmentationUtilitiesView()
  : m_BooleanOperationsWidget(nullptr),
    m_ContourModelToImageWidget(nullptr),
    m_ImageMaskingWidget(nullptr),
    m_MorphologicalOperationsWidget(nullptr),
    m_SurfaceToImageWidget(nullptr)
{
}

QmitkSegmentationUtilitiesView::~QmitkSegmentationUtilitiesView()
{
}

void QmitkSegmentationUtilitiesView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  mitk::SliceNavigationController* timeNavigationController = renderWindowPart != nullptr
    ? renderWindowPart->GetTimeNavigationController()
    : nullptr;

  m_BooleanOperationsWidget = new QmitkBooleanOperationsWidget(timeNavigationController, parent);
  m_ContourModelToImageWidget = new QmitkContourModelToImageWidget(timeNavigationController, parent);
  m_ImageMaskingWidget = new QmitkImageMaskingWidget(timeNavigationController, parent);
  m_MorphologicalOperationsWidget = new QmitkMorphologicalOperationsWidget(timeNavigationController, parent);
  m_SurfaceToImageWidget = new QmitkSurfaceToImageWidget(timeNavigationController, parent);

  this->AddUtilityWidget(m_BooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  this->AddUtilityWidget(m_ContourModelToImageWidget, QIcon(":/SegmentationUtilities/ContourModelSetToImage_48x48.png"), "Contour to Image");
  this->AddUtilityWidget(m_ImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  this->AddUtilityWidget(m_MorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
  this->AddUtilityWidget(m_SurfaceToImageWidget, QIcon(":/SegmentationUtilities/SurfaceToImage_48x48.png"), "Surface to Image");
}

void QmitkSegmentationUtilitiesView::AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text)
{
  m_Controls.toolBox->addItem(widget, icon, text);
}

void QmitkSegmentationUtilitiesView::SetFocus()
{
  m_Controls.toolBox->setFocus();
}

void QmitkSegmentationUtilitiesView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  mitk::SliceNavigationController* timeNavigationController = renderWindowPart->GetTimeNavigationController();

  m_BooleanOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_ContourModelToImageWidget->SetTimeNavigationController(timeNavigationController);
  m_ImageMaskingWidget->SetTimeNavigationController(timeNavigationController);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_SurfaceToImageWidget->SetTimeNavigationController(timeNavigationController);
}

void QmitkSegmentationUtilitiesView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  m_BooleanOperationsWidget->SetTimeNavigationController(nullptr);
  m_ContourModelToImageWidget->SetTimeNavigationController(nullptr);
  m_ImageMaskingWidget->SetTimeNavigationController(nullptr);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(nullptr);
  m_SurfaceToImageWidget->SetTimeNavigationController(nullptr);
}
