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

  m_Controls.toolBox->addItem(new QmitkBooleanOperationsWidget, QIcon(":/SegmentationUtilities/BooleanOperations_48x48.png"), "Boolean Operations");
  m_Controls.toolBox->addItem(new QmitkImageMaskingWidget, QIcon(":/SegmentationUtilities/ImageMasking_48x48.png"), "Image Masking");
  m_Controls.toolBox->addItem(new QmitkMorphologicalOperationsWidget, QIcon(":/SegmentationUtilities/MorphologicalOperations_48x48.png"), "Morphological Operations");
}

void QmitkSegmentationUtilities2View::SetFocus()
{
  m_Controls.toolBox->setFocus();
}
