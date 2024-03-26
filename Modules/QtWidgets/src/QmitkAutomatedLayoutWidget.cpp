/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutomatedLayoutWidget.h"

//#include <QmitkNodeSelec>

QmitkAutomatedLayoutWidget::QmitkAutomatedLayoutWidget(QWidget* parent)
{
	m_Controls.setupUi(this);

	connect(m_Controls.dataSelector, &QmitkMultiNodeSelectionWidget::DialogClosed, this, &QmitkAutomatedLayoutWidget::OnSelectionDialogClosed);
}

void QmitkAutomatedLayoutWidget::OnSelectDataClicked()
{
	
}

void QmitkAutomatedLayoutWidget::OnSetLayoutClicked()
{

}

void QmitkAutomatedLayoutWidget::OnSelectionDialogClosed()
{
	this->show();
}
