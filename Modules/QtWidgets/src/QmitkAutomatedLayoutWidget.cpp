/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutomatedLayoutWidget.h"

QmitkAutomatedLayoutWidget::QmitkAutomatedLayoutWidget(QWidget* parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelector->SetInvalidInfo(QStringLiteral("Please select images for the layout"));
  m_Controls.dataSelector->SetPopUpTitel(QStringLiteral("Select input images"));
  m_Controls.dataSelector->SetPopUpHint(QStringLiteral("Select as many images as you want to compare"));

  m_Controls.setLayoutButton->setEnabled(false);

  connect(m_Controls.dataSelector, &QmitkMultiNodeSelectionWidget::DialogClosed, this, &QmitkAutomatedLayoutWidget::OnSelectionDialogClosed);
  connect(m_Controls.setLayoutButton, &QPushButton::clicked, this, &QmitkAutomatedLayoutWidget::OnSetLayoutClicked);
}

void QmitkAutomatedLayoutWidget::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  // smart pointer to data storage needs to be saved here because dataSelector makes it a regular pointer
  m_DataStorage = dataStorage;
  m_Controls.dataSelector->SetDataStorage(m_DataStorage);
}

void QmitkAutomatedLayoutWidget::OnSetLayoutClicked()
{
  auto selectedNodes = m_Controls.dataSelector->GetSelectedNodes();
  emit SetDataBasedLayout(selectedNodes);
  this->hide();
}

void QmitkAutomatedLayoutWidget::OnSelectionDialogClosed()
{
  bool setLayoutEnabled = m_Controls.dataSelector->GetSelectedNodes().size() > 0;
  m_Controls.setLayoutButton->setEnabled(setLayoutEnabled);
  this->show();
}
