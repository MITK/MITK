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

#include "QmitkLevelWindowRangeChangeDialog.h"

#include <QMessageBox>

QmitkLevelWindowRangeChangeDialog::QmitkLevelWindowRangeChangeDialog(QWidget* parent, Qt::WindowFlags f)
 : QDialog(parent, f)
{
  this->setupUi(this);

  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(inputValidator()));
}

double QmitkLevelWindowRangeChangeDialog::getLowerLimit()
{
  return rangeMinDoubleSpinBox->value();
}

double QmitkLevelWindowRangeChangeDialog::getUpperLimit()
{
  return rangeMaxDoubleSpinBox->value();
}

void QmitkLevelWindowRangeChangeDialog::setLowerLimit( double rangeMin )
{
  rangeMinDoubleSpinBox->setValue(rangeMin);
}

void QmitkLevelWindowRangeChangeDialog::setUpperLimit( double rangeMax )
{
  rangeMaxDoubleSpinBox->setValue(rangeMax);
}

void QmitkLevelWindowRangeChangeDialog::inputValidator()
{
  if (!(rangeMinDoubleSpinBox->value() < rangeMaxDoubleSpinBox->value()))
  {
    QMessageBox::critical( this, "Change Range",
    "Upper limit has to be greater than lower limit.\n"
    "Please enter limits again." );
  }
  else
    this->accept();
}
