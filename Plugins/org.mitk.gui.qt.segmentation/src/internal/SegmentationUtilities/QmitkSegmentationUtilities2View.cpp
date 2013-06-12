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

#include "QmitkSegmentationUtilities2View.h"
#include "BooleanOperations/QmitkBooleanOperationsWidget.h"
#include "ImageMasking/QmitkImageMaskingWidget.h"
#include "MorphologicalOperations/QmitkMorphologicalOPerationsWidget.h"
#include "SurfaceToImage/QmitkSurfaceToImageWidget.h"

QmitkSegmentationUtilities2View::QmitkSegmentationUtilities2View()
  : m_BooleanOperationsWidget(NULL),
    m_ImageMaskingWidget(NULL),
    m_MorphologicalOperationsWidget(NULL),
    m_SurfaceToImageWidget(NULL)
{
}

QmitkSegmentationUtilities2View::~QmitkSegmentationUtilities2View()
{
}

void QmitkSegmentationUtilities2View::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();

  mitk::SliceNavigationController* timeNavigationController = renderWindowPart != NULL
    ? renderWindowPart->GetTimeNavigationController()
    : NULL;

  m_BooleanOperationsWidget = new QmitkBooleanOperationsWidget(timeNavigationController, parent);
  m_ImageMaskingWidget = new QmitkImageMaskingWidget(timeNavigationController, parent);
  m_MorphologicalOperationsWidget = new QmitkMorphologicalOperationsWidget(timeNavigationController, parent);
  m_SurfaceToImageWidget = new QmitkSurfaceToImageWidget(timeNavigationController, parent);


  this->AddUtilityWidget(m_BooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  this->AddUtilityWidget(m_ImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  this->AddUtilityWidget(m_MorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
  this->AddUtilityWidget(m_SurfaceToImageWidget, QIcon(":/SegmentationUtilities/utilities_surface2image.png"), "Surface To Image");
}

void QmitkSegmentationUtilities2View::AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text)
{
  m_Controls.toolBox->addItem(widget, icon, text);
}

void QmitkSegmentationUtilities2View::SetFocus()
{
  m_Controls.toolBox->setFocus();
}

void QmitkSegmentationUtilities2View::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  mitk::SliceNavigationController* timeNavigationController = renderWindowPart->GetTimeNavigationController();

  m_BooleanOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_ImageMaskingWidget->SetTimeNavigationController(timeNavigationController);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(timeNavigationController);
  m_SurfaceToImageWidget->SetTimeNavigationController(timeNavigationController);
}

void QmitkSegmentationUtilities2View::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  m_BooleanOperationsWidget->SetTimeNavigationController(NULL);
  m_ImageMaskingWidget->SetTimeNavigationController(NULL);
  m_MorphologicalOperationsWidget->SetTimeNavigationController(NULL);
  m_SurfaceToImageWidget->SetTimeNavigationController(NULL);
}
