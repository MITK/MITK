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
#include <mitkException.h>
#include <mitkSliceNavigationController.h>
#include <cassert>

static const char* const HelpText = "Select two different segmentations above";

QmitkBooleanOperationsWidget::QmitkBooleanOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);

  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
  connect(m_Controls.differenceButton, SIGNAL(clicked()), this, SLOT(OnDifferenceButtonClicked()));
  connect(m_Controls.intersectionButton, SIGNAL(clicked()), this, SLOT(OnIntersectionButtonClicked()));
  connect(m_Controls.unionButton, SIGNAL(clicked()), this, SLOT(OnUnionButtonClicked()));
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
    this->EnableButtons();
  }
  else
  {
    m_Controls.dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
}

void QmitkBooleanOperationsWidget::EnableButtons(bool enable)
{
  m_Controls.differenceButton->setEnabled(enable);
  m_Controls.intersectionButton->setEnabled(enable);
  m_Controls.unionButton->setEnabled(enable);
}

void QmitkBooleanOperationsWidget::OnDiffernceButtonClicked()
{
  this->DoBooleanOperation(mitk::BooleanOperation::Difference);
}

void QmitkBooleanOperationsWidget::OnIntersectionButtonClicked()
{
  this->DoBooleanOperation(mitk::BooleanOperation::Intersection);
}

void QmitkBooleanOperationsWidget::OnUnionButtonClicked()
{
  this->DoBooleanOperation(mitk::BooleanOperation::Union);
}

void QmitkBooleanOperationsWidget::DoBooleanOperation(mitk::BooleanOperation::Type type)
{
  mitk::SliceNavigationController* timeNavigationController = this->GetTimeNavigationController();
  assert(timeNavigationController != NULL);

  mitk::Image::ConstPointer segmentation0 = static_cast<mitk::Image*>(m_Controls.dataSelectionWidget->GetSelection(0)->GetData());
  mitk::Image::ConstPointer segmentation1 = static_cast<mitk::Image*>(m_Controls.dataSelectionWidget->GetSelection(1)->GetData());

  try
  {
    unsigned int time = timeNavigationController->GetTime()->GetPos();
    mitk::BooleanOperation booleanOperation(type, segmentation0, segmentation1, time);
  }
  catch (const mitk::Exception &exception)
  {
    MITK_ERROR << "Boolean operation failed: " << exception.GetDescription();
  }
}
