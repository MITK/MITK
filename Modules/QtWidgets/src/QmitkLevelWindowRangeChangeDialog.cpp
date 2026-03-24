/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkLevelWindowRangeChangeDialog.h>
#include <ui_QmitkLevelWindowRangeChange.h>

#include <QMessageBox>

QmitkLevelWindowRangeChangeDialog::QmitkLevelWindowRangeChangeDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_Controls(std::make_unique<Ui::QmitkLevelWindowRangeChange>())
{
  m_Controls->setupUi(this);

  QObject::connect(m_Controls->okButton, SIGNAL(clicked()), this, SLOT(inputValidator()));
}

QmitkLevelWindowRangeChangeDialog::~QmitkLevelWindowRangeChangeDialog()
{
}

double QmitkLevelWindowRangeChangeDialog::getLowerLimit()
{
  return m_Controls->rangeMinDoubleSpinBox->value();
}

double QmitkLevelWindowRangeChangeDialog::getUpperLimit()
{
  return m_Controls->rangeMaxDoubleSpinBox->value();
}

void QmitkLevelWindowRangeChangeDialog::setLowerLimit(double rangeMin)
{
  m_Controls->rangeMinDoubleSpinBox->setValue(rangeMin);
}

void QmitkLevelWindowRangeChangeDialog::setUpperLimit(double rangeMax)
{
  m_Controls->rangeMaxDoubleSpinBox->setValue(rangeMax);
}

void QmitkLevelWindowRangeChangeDialog::inputValidator()
{
  if (!(m_Controls->rangeMinDoubleSpinBox->value() < m_Controls->rangeMaxDoubleSpinBox->value()))
  {
    QMessageBox::critical(this,
                          "Change Range",
                          "Upper limit has to be greater than lower limit.\n"
                          "Please enter limits again.");
  }
  else
    this->accept();
}
