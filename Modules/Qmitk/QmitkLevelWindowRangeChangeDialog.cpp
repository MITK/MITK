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

int QmitkLevelWindowRangeChangeDialog::getLowerLimit()
{
  return rangeMinSpinBox->value();
}

int QmitkLevelWindowRangeChangeDialog::getUpperLimit()
{
  return rangeMaxSpinBox->value();
}

void QmitkLevelWindowRangeChangeDialog::setLowerLimit( int rangeMin )
{
  rangeMinSpinBox->setValue(rangeMin);
}

void QmitkLevelWindowRangeChangeDialog::setUpperLimit( int rangeMax )
{
  rangeMaxSpinBox->setValue(rangeMax);
}

void QmitkLevelWindowRangeChangeDialog::inputValidator()
{
  if (!(rangeMinSpinBox->value() < rangeMaxSpinBox->value()))
  {
    QMessageBox::critical( this, "Change Range",
    "Upper limit has to be greater than lower limit.\n"
    "Please enter limits again." );
  }
  else
    this->accept();
}
