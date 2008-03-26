/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
