/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBooleanOperationsWidget.h"
#include "../../Common/QmitkDataSelectionWidget.h"
#include <mitkException.h>
#include <mitkSliceNavigationController.h>
#include <cassert>

static const char* const HelpText = "Select two different masks above";

std::string GetPrefix(mitk::BooleanOperation::Type type)
{
  switch (type)
  {
    case mitk::BooleanOperation::Difference:
      return "DifferenceFrom_";

    case mitk::BooleanOperation::Intersection:
      return "IntersectionWith_";

    case mitk::BooleanOperation::Union:
      return "UnionWith_";

    default:
      assert(false && "Unknown boolean operation type");
      return "UNKNOWN_BOOLEAN_OPERATION_WITH_";
  }
}

void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation, const std::string& name, mitk::DataNode::Pointer parent = nullptr)
{
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

  dataNode->SetBoolProperty("binary", true);
  dataNode->SetName(name);
  dataNode->SetData(segmentation);

  dataStorage->Add(dataNode, parent);
}

QmitkBooleanOperationsWidget::QmitkBooleanOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::MaskPredicate);
  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::MaskPredicate);

  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
  connect(m_Controls.differenceButton, SIGNAL(clicked()), this, SLOT(OnDifferenceButtonClicked()));
  connect(m_Controls.intersectionButton, SIGNAL(clicked()), this, SLOT(OnIntersectionButtonClicked()));
  connect(m_Controls.unionButton, SIGNAL(clicked()), this, SLOT(OnUnionButtonClicked()));
}

QmitkBooleanOperationsWidget::~QmitkBooleanOperationsWidget()
{
}

void QmitkBooleanOperationsWidget::OnSelectionChanged(unsigned int, const mitk::DataNode*)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNotNull() && node1.IsNotNull() && node0 != node1)
  {
    dataSelectionWidget->SetHelpText("");
    this->EnableButtons();
  }
  else
  {
    dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
}

void QmitkBooleanOperationsWidget::EnableButtons(bool enable)
{
  m_Controls.differenceButton->setEnabled(enable);
  m_Controls.intersectionButton->setEnabled(enable);
  m_Controls.unionButton->setEnabled(enable);
}

void QmitkBooleanOperationsWidget::OnDifferenceButtonClicked()
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
  assert(timeNavigationController != nullptr);

  mitk::Image::Pointer segmentation0 = static_cast<mitk::Image*>(m_Controls.dataSelectionWidget->GetSelection(0)->GetData());
  mitk::Image::Pointer segmentation1 = static_cast<mitk::Image*>(m_Controls.dataSelectionWidget->GetSelection(1)->GetData());
  mitk::Image::Pointer result;

  try
  {
    mitk::BooleanOperation booleanOperation(type, segmentation0, segmentation1, timeNavigationController->GetTime()->GetPos());
    result = booleanOperation.GetResult();
  }
  catch (const mitk::Exception &exception)
  {
    MITK_ERROR << "Boolean operation failed: " << exception.GetDescription();
  }

  assert(result.IsNotNull());

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  AddToDataStorage(
    dataSelectionWidget->GetDataStorage(),
    result,
    GetPrefix(type) + dataSelectionWidget->GetSelection(1)->GetName(),
    dataSelectionWidget->GetSelection(0));
}
