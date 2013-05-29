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

#include "QmitkBooleanOperationsWidget.h"
#include "../../Common/QmitkDataSelectionWidget.h"
#include <mitkBooleanOperation.h>
#include <mitkException.h>

static const char* const HelpText = "Select two different segmentations above";

QmitkBooleanOperationsWidget::QmitkBooleanOperationsWidget(QWidget* parent)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);

  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
}

QmitkBooleanOperationsWidget::~QmitkBooleanOperationsWidget()
{
}

void QmitkBooleanOperationsWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  mitk::DataNode::Pointer node0 = m_Controls.dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = m_Controls.dataSelectionWidget->GetSelection(1);

  if (node0.IsNotNull() && node1.IsNotNull() && node0 != node1)
  {
    m_Controls.dataSelectionWidget->SetHelpText("");
  }
  else
  {
    m_Controls.dataSelectionWidget->SetHelpText(HelpText);
  }
}
