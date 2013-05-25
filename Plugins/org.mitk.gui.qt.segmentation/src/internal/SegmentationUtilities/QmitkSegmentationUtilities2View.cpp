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

QmitkSegmentationUtilities2View::QmitkSegmentationUtilities2View()
{
}

QmitkSegmentationUtilities2View::~QmitkSegmentationUtilities2View()
{
}

void QmitkSegmentationUtilities2View::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);

  this->AddUtilityWidget(new QmitkBooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  this->AddUtilityWidget(new QmitkImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  this->AddUtilityWidget(new QmitkMorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
}

void QmitkSegmentationUtilities2View::AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text)
{
  m_Controls.toolBox->addItem(widget, icon, text);
}

void QmitkSegmentationUtilities2View::SetFocus()
{
  m_Controls.toolBox->setFocus();
}
