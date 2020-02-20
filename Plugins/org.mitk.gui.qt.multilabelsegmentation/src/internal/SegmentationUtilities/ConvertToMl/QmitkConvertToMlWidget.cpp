/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkConvertToMlWidget.h"
#include "../../Common/QmitkDataSelectionWidget.h"
#include <mitkException.h>
#include <mitkSliceNavigationController.h>
#include <cassert>
#include <mitkImage.h>
#include "src/internal/QmitkConvertToMultiLabelSegmentationAction.h"

QmitkConvertToMlWidget::QmitkConvertToMlWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ImagePredicate);

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(SelectionChanged()));
  connect(m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(Convert()));
}

QmitkConvertToMlWidget::~QmitkConvertToMlWidget()
{
}

void QmitkConvertToMlWidget::SelectionChanged()
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);

  if (node0.IsNotNull())
  {
    this->EnableButtons();
  }
  else
  {
    this->EnableButtons(false);
  }
}

void QmitkConvertToMlWidget::EnableButtons(bool enable)
{
  m_Controls.pushButton->setEnabled(enable);
}

void QmitkConvertToMlWidget::Convert()
{
  auto node = m_Controls.dataSelectionWidget->GetSelection(0);

  QmitkConvertToMultiLabelSegmentationAction converter;
  QList<mitk::DataNode::Pointer> list; list.append(node);
  converter.SetDataStorage(m_Controls.dataSelectionWidget->GetDataStorage());
  converter.Run(list);
}
