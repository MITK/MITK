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

#include "QmitkSegmentationUtilitiesView.h"
#include "BooleanOperations/QmitkBooleanOperationsWidget.h"
#include "ContourModelToImage/QmitkContourModelToImageWidget.h"
#include "ImageMasking/QmitkImageMaskingWidget.h"
#include "MorphologicalOperations/QmitkMorphologicalOperationsWidget.h"
#include "SurfaceToImage/QmitkSurfaceToImageWidget.h"
#include "CTKWidgets/QmitkCTKWidgetsWidget.h"

QmitkSegmentationUtilitiesView::QmitkSegmentationUtilitiesView()
  : m_BooleanOperationsWidget(NULL),
    m_ContourModelToImageWidget(NULL),
    m_ImageMaskingWidget(NULL),
    m_MorphologicalOperationsWidget(NULL),
    m_SurfaceToImageWidget(NULL),
    m_CTKWidgetsWidget(NULL)
{
}

QmitkSegmentationUtilitiesView::~QmitkSegmentationUtilitiesView()
{
}

void QmitkSegmentationUtilitiesView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  mitk::SliceNavigationController* timeNavigationController = renderWindowPart != NULL
    ? renderWindowPart->GetTimeNavigationController()
    : NULL;

  m_BooleanOperationsWidget = new QmitkBooleanOperationsWidget(timeNavigationController, parent);
  m_ContourModelToImageWidget = new QmitkContourModelToImageWidget(timeNavigationController, parent);
  m_ImageMaskingWidget = new QmitkImageMaskingWidget(timeNavigationController, parent);
  m_MorphologicalOperationsWidget = new QmitkMorphologicalOperationsWidget(timeNavigationController, parent);
  m_SurfaceToImageWidget = new QmitkSurfaceToImageWidget(timeNavigationController, parent);

  this->AddUtilityWidget(m_BooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  this->AddUtilityWidget(m_ContourModelToImageWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Contour Model To Image");
  this->AddUtilityWidget(m_ImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  this->AddUtilityWidget(m_MorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
  this->AddUtilityWidget(m_SurfaceToImageWidget, QIcon(":/SegmentationUtilities/SurfaceToImage_48x48.png"), "Surface To Image");

#ifdef ENABLE_CTK_WIDGETS_WIDGET
  m_CTKWidgetsWidget = new QmitkCTKWidgetsWidget(timeNavigationController, parent);
  this->AddUtilityWidget(m_CTKWidgetsWidget, QIcon(":/SegmentationUtilities/CTKWidgets_48x48.png"), "CTK Widgets");
#endif
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

#ifdef ENABLE_CTK_WIDGETS_WIDGET
  m_CTKWidgetsWidget->SetTimeNavigationController(timeNavigationController);
#endif
}

void QmitkSegmentationUtilitiesView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  m_BooleanOperationsWidget->SetTimeNavigationController(NULL);
  m_ContourModelToImageWidget->SetTimeNavigationController(NULL);
  m_ImageMaskingWidget->SetTimeNavigationController(NULL);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(NULL);
  m_SurfaceToImageWidget->SetTimeNavigationController(NULL);

#ifdef ENABLE_CTK_WIDGETS_WIDGET
  m_CTKWidgetsWidget->SetTimeNavigationController(NULL);
#endif
}
