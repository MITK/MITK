/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationUtilitiesView.h"

#include <QmitkBooleanOperationsWidget.h>
#include <QmitkImageMaskingWidget.h>
#include <QmitkMorphologicalOperationsWidget.h>
#include <QmitkConvertToMultiLabelSegmentationWidget.h>
#include <QmitkExtractFromMultiLabelSegmentationWidget.h>

QmitkSegmentationUtilitiesView::QmitkSegmentationUtilitiesView()
  : m_BooleanOperationsWidget(nullptr),
    m_ImageMaskingWidget(nullptr),
    m_MorphologicalOperationsWidget(nullptr),
    m_ConvertToSegWidget(nullptr),
    m_ExtractFromSegWidget(nullptr)
{
}

QmitkSegmentationUtilitiesView::~QmitkSegmentationUtilitiesView()
{
}

void QmitkSegmentationUtilitiesView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  auto dataStorage = this->GetDataStorage();
  m_BooleanOperationsWidget = new QmitkBooleanOperationsWidget(dataStorage, parent);
  m_ImageMaskingWidget = new QmitkImageMaskingWidget(dataStorage, parent);
  m_MorphologicalOperationsWidget = new QmitkMorphologicalOperationsWidget(dataStorage, parent);
  m_ConvertToSegWidget = new QmitkConvertToMultiLabelSegmentationWidget(dataStorage, parent);
  m_ExtractFromSegWidget = new QmitkExtractFromMultiLabelSegmentationWidget(dataStorage, parent);

  this->AddUtilityWidget(m_BooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  this->AddUtilityWidget(m_ImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  this->AddUtilityWidget(m_MorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
  this->AddUtilityWidget(m_ConvertToSegWidget, QIcon(":/SegmentationUtilities/SurfaceToImage_48x48.png"), "Convert to Segmentation");
  this->AddUtilityWidget(m_ExtractFromSegWidget, QIcon(":/SegmentationUtilities/Extract_48x48.png"), "Extract from Segmentation");
}

void QmitkSegmentationUtilitiesView::AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text)
{
  m_Controls.toolBox->addItem(widget, icon, text);
}

void QmitkSegmentationUtilitiesView::SetFocus()
{
  m_Controls.toolBox->setFocus();
}

void QmitkSegmentationUtilitiesView::RenderWindowPartActivated(mitk::IRenderWindowPart*)
{
}

void QmitkSegmentationUtilitiesView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
}
