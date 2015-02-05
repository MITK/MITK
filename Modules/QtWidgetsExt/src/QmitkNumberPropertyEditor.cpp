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
#include "QmitkNumberPropertyEditor.h"
#include <mitkRenderingManager.h>
#include <QTextStream>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4

#define ROUND(x)       (((x) > 0) ?   int((x) + 0.5) :   int((x) - 0.5))
#define ROUND_SHORT(x) (((x) > 0) ? short((x) + 0.5) : short((x) - 0.5))

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::IntProperty* property, QWidget* parent )
: QSpinBox( parent ),
  PropertyEditor( property ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::FloatProperty* property, QWidget* parent )
: QSpinBox( parent ),
  PropertyEditor( property ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::DoubleProperty* property, QWidget* parent )
: QSpinBox( parent ),
  PropertyEditor( property ),
  m_DoubleProperty(property),
  m_DataType(DT_DOUBLE)
{
  initialize();
}

QmitkNumberPropertyEditor::~QmitkNumberPropertyEditor()
{
}

void QmitkNumberPropertyEditor::initialize()
{ // only to be called from constructors

  // spinbox settings
  setSuffix("");

  // protected
  m_DecimalPlaces = 0;
  m_FactorPropertyToSpinbox = 1.0;
  m_FactorSpinboxToDisplay = 1.0;
  m_ShowPercents = false;

  // private
  m_SelfChangeLock = false;

  connect( this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)) );

  // display current value of our property
  DisplayNumber();
}

void QmitkNumberPropertyEditor::adjustFactors(short newDecimalPlaces, bool newShowPercents)
{
  int oldMax = maxValue();
  int oldMin = minValue();

  m_DecimalPlaces = newDecimalPlaces;
  m_ShowPercents = newShowPercents;

  m_FactorPropertyToSpinbox = pow(10.0,m_DecimalPlaces);
  m_FactorSpinboxToDisplay = 1.0 / m_FactorPropertyToSpinbox;

  if ( m_ShowPercents )
  {
    m_FactorPropertyToSpinbox *= 100.0;
    setSuffix("%");
  }
  else
  {
    setSuffix("");
  }

  setMinValue(oldMin);
  setMaxValue(oldMax);
}

short QmitkNumberPropertyEditor::getDecimalPlaces() const
{
  return m_DecimalPlaces;
}

void QmitkNumberPropertyEditor::setDecimalPlaces(short places)
{
  switch (m_DataType)
  {
    case DT_FLOAT:
    case DT_DOUBLE:
      {
        adjustFactors( places, m_ShowPercents );
        DisplayNumber();
        break;
      }
    default:
      break;
  }
}

bool QmitkNumberPropertyEditor::getShowPercent() const
{
  return m_ShowPercents;
}

void QmitkNumberPropertyEditor::setShowPercent(bool showPercent)
{
  if ( showPercent == m_ShowPercents ) return; // nothing to change

  switch (m_DataType)
  {
    case DT_FLOAT:
    case DT_DOUBLE:
    {
      adjustFactors( m_DecimalPlaces, showPercent );
      break;
    }
    default:
    {
      break;
    }
  }

  DisplayNumber();
}

int QmitkNumberPropertyEditor::minValue() const
{
  return ROUND( QSpinBox::minimum() / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setMinValue(int value)
{
  QSpinBox::setMinimum( ROUND(value * m_FactorPropertyToSpinbox ) );
}

int QmitkNumberPropertyEditor::maxValue() const
{
  return ROUND( QSpinBox::maximum() / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setMaxValue(int value)
{
  QSpinBox::setMaximum( ROUND( value * m_FactorPropertyToSpinbox ) );
}

double QmitkNumberPropertyEditor::doubleValue() const
{
  return static_cast<double>((QSpinBox::value()) / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setDoubleValue(double value)
{
  QSpinBox::setValue( ROUND( value * m_FactorPropertyToSpinbox ) );
}

QString QmitkNumberPropertyEditor::textFromValue(int value) const
{
  QString displayedText;
  QTextStream stream(&displayedText);

  double d( value * m_FactorSpinboxToDisplay );

  if ( m_DecimalPlaces > 0 )
  {
    stream.setRealNumberPrecision(m_DecimalPlaces);
    stream << d;
  }
  else
  {
    stream << ROUND(d);
  }

  return displayedText;
}

int QmitkNumberPropertyEditor::valueFromText(const QString& text) const
{
  return ROUND( text.toDouble() / m_FactorSpinboxToDisplay );
}

void QmitkNumberPropertyEditor::onValueChanged(int value)
{
  if (m_SelfChangeLock) return; // valueChanged is even emitted, when this widget initiates a change to its value
                                // this may be useful some times, but in this use, it would be wrong:
                                //   (A) is an editor with 3 decimal places
                                //   (B) is an editor with 2 decimal places
                                //   User changes A's displayed value to 4.002
                                //   A's onValueChanged gets called, sets the associated Property to 4.002
                                //   B's onPropertyChanged gets called, sets its display to 4.00
                                //   B's onValueChanged gets called and sets the associated Property to 4.00
                                //   A's onPropertyChanged gets called, sets its display to 4.000

  BeginModifyProperty();

  double newValue( value / m_FactorPropertyToSpinbox );

  switch (m_DataType)
  {
    case DT_INT:
      {
        m_IntProperty->SetValue(ROUND(newValue));
        break;
      }
    case DT_FLOAT:
      {
        m_FloatProperty->SetValue(newValue);
        break;
      }
    case DT_DOUBLE:
      {
        m_DoubleProperty->SetValue(newValue);
        break;
      }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  EndModifyProperty();
}

void QmitkNumberPropertyEditor::PropertyChanged()
{
  DisplayNumber();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNumberPropertyEditor::PropertyRemoved()
{
  m_Property = NULL;
}

void QmitkNumberPropertyEditor::DisplayNumber()
{
  if (! m_Property ) return;

  m_SelfChangeLock = true;
  switch (m_DataType)
  {
    case DT_INT:
      {
        int i = m_IntProperty->GetValue();
        QSpinBox::setValue( i );
        break;
      }
    case DT_FLOAT:
      {
        float f = m_FloatProperty->GetValue();
        setDoubleValue( f );
        break;
      }
    case DT_DOUBLE:
      {
        double d = m_DoubleProperty->GetValue();
        setDoubleValue( d );
        break;
      }
    default:
      break;
  }
  m_SelfChangeLock = false;
}

