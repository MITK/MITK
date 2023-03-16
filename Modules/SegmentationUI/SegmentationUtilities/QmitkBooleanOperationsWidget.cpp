/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBooleanOperationsWidget.h"
#include <ui_QmitkBooleanOperationsWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkSliceNavigationController.h>

#include <QMessageBox>

#include <cassert>

static const char* const HelpText = "Select two different segmentations above";

namespace
{
  static std::string GetPrefix(mitk::BooleanOperation::Type type)
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

  static void AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation, const std::string& name, mitk::DataNode::Pointer parent = nullptr)
  {
    if (dataStorage.IsNull())
    {
      std::string exception = "Cannot add result to the data storage. Data storage invalid.";
      MITK_ERROR << "Boolean operation failed: " << exception;
      QMessageBox::information(nullptr, "Boolean operation failed", QString::fromStdString(exception));
    }

    auto dataNode = mitk::DataNode::New();
    dataNode->SetName(name);
    dataNode->SetData(segmentation);

    dataStorage->Add(dataNode, parent);
  }
}

QmitkBooleanOperationsWidget::QmitkBooleanOperationsWidget(mitk::DataStorage* dataStorage,
                                                           mitk::SliceNavigationController* timeNavigationController,
                                                           QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls = new Ui::QmitkBooleanOperationsWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->dataSelectionWidget->SetDataStorage(dataStorage);
  m_Controls->dataSelectionWidget->AddDataSelection("<img width=16 height=16 src=\":/Qmitk/BooleanLabelA_32x32.png\"/>", "Select 1st segmentation", "Select 1st segmentation", "", QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls->dataSelectionWidget->AddDataSelection("<img width=16 height=16 src=\":/Qmitk/BooleanLabelB_32x32.png\"/>", "Select 2nd segmentation", "Select 2nd segmentation", "", QmitkDataSelectionWidget::SegmentationPredicate);

  m_Controls->dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls->dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
  connect(m_Controls->differenceButton, SIGNAL(clicked()), this, SLOT(OnDifferenceButtonClicked()));
  connect(m_Controls->intersectionButton, SIGNAL(clicked()), this, SLOT(OnIntersectionButtonClicked()));
  connect(m_Controls->unionButton, SIGNAL(clicked()), this, SLOT(OnUnionButtonClicked()));
}

QmitkBooleanOperationsWidget::~QmitkBooleanOperationsWidget()
{
}

void QmitkBooleanOperationsWidget::OnSelectionChanged(unsigned int, const mitk::DataNode*)
{
  auto dataSelectionWidget = m_Controls->dataSelectionWidget;

  auto nodeA = dataSelectionWidget->GetSelection(0);
  auto nodeB = dataSelectionWidget->GetSelection(1);

  if (nodeA.IsNotNull() && nodeB.IsNotNull() && nodeA != nodeB)
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
  m_Controls->differenceButton->setEnabled(enable);
  m_Controls->intersectionButton->setEnabled(enable);
  m_Controls->unionButton->setEnabled(enable);
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
  auto timeNavigationController = this->GetTimeNavigationController();
  assert(timeNavigationController != nullptr);

  mitk::Image::Pointer segmentationA = dynamic_cast<mitk::Image*>(m_Controls->dataSelectionWidget->GetSelection(0)->GetData());
  mitk::Image::Pointer segmentationB = dynamic_cast<mitk::Image*>(m_Controls->dataSelectionWidget->GetSelection(1)->GetData());
  mitk::Image::Pointer result;

  try
  {
    mitk::BooleanOperation booleanOperation(type, segmentationA, segmentationB, timeNavigationController->GetSelectedTimePoint());
    result = booleanOperation.GetResult();

    assert(result.IsNotNull());

    auto dataSelectionWidget = m_Controls->dataSelectionWidget;

    AddToDataStorage(
      dataSelectionWidget->GetDataStorage(),
      result,
      GetPrefix(type) + dataSelectionWidget->GetSelection(1)->GetName(),
      dataSelectionWidget->GetSelection(0));
  }
  catch (const mitk::Exception& exception)
  {
    MITK_ERROR << "Boolean operation failed: " << exception.GetDescription();
    QMessageBox::information(nullptr, "Boolean operation failed", exception.GetDescription());
  }
}
