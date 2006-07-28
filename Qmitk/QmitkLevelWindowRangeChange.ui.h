/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

int QmitkLevelWindowRangeChange::getLowerLimit()
{
  return rangeMinSpinBox->value();
}

int QmitkLevelWindowRangeChange::getUpperLimit()
{
  return rangeMaxSpinBox->value();
}

void QmitkLevelWindowRangeChange::setLowerLimit( int rangeMin )
{
  rangeMinSpinBox->setValue(rangeMin);
}

void QmitkLevelWindowRangeChange::setUpperLimit( int rangeMax )
{
  rangeMaxSpinBox->setValue(rangeMax);
}

void QmitkLevelWindowRangeChange::inputValidator()
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
