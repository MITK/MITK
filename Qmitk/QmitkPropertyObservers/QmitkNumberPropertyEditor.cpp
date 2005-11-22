/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <QmitkNumberPropertyEditor.h>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::GenericProperty<short>* property, QWidget* parent, const char* name )
: PropertyEditor( property ),
  QSpinBox( parent, name ),
  m_ShortProperty(property),
  m_DataType(DT_SHORT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::IntProperty* property, QWidget* parent, const char* name )
: PropertyEditor( property ),
  QSpinBox( parent, name ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::FloatProperty* property, QWidget* parent, const char* name )
: PropertyEditor( property ),
  QSpinBox( parent, name ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::DoubleProperty* property, QWidget* parent, const char* name )
: PropertyEditor( property ),
  QSpinBox( parent, name ),
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
  setValidator(0);
  setSuffix("");
  m_DisplayFactor = 1.0;
  m_DecimalPlaces = 0;

  DisplayNumber();
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
        int oldMax = maxValue();
        int oldMin = minValue();

        m_DecimalPlaces = places;
 
        setMinValue(oldMin);
        setMaxValue(oldMax);

        setLineStep( static_cast<int>(pow(10, m_DecimalPlaces)) );

        DisplayNumber();
        break;
      }
    default:
      break;
  }
}

bool QmitkNumberPropertyEditor::getShowPercent() const
{
//  return m_DisplayFactor == 100.0;
}

void QmitkNumberPropertyEditor::setShowPercent(bool show)
{
/*
  switch (m_DataType)
  {
    case DT_FLOAT:
    case DT_DOUBLE:
      if (show)
      {
        m_DisplayFactor = 100.0;
        setSuffix("%");
      }
      else
      {
        m_DisplayFactor = 1.0;
        setSuffix("");
      }
      break;
    default:
      break;
  }
  
  DisplayNumber();
*/
}

int QmitkNumberPropertyEditor::minValue() const
{
  return static_cast<int>( (QSpinBox::minValue() / m_DisplayFactor / pow(10,m_DecimalPlaces)) );
}

void QmitkNumberPropertyEditor::setMinValue(int value)
{
  QSpinBox::setMinValue( static_cast<int>( value / m_DisplayFactor * pow(10,m_DecimalPlaces) ) );
}

int QmitkNumberPropertyEditor::maxValue() const
{
  return static_cast<int>( (QSpinBox::maxValue() / m_DisplayFactor / pow(10,m_DecimalPlaces)) );
}

void QmitkNumberPropertyEditor::setMaxValue(int value)
{
  QSpinBox::setMaxValue( static_cast<int>( value / m_DisplayFactor * pow(10,m_DecimalPlaces) ) );
}

double QmitkNumberPropertyEditor::doubleValue() const
{
  return static_cast<double>((QSpinBox::value()) / pow(10,m_DecimalPlaces) );
}

void QmitkNumberPropertyEditor::setDoubleValue(double value)
{
  switch (m_DataType)
  {
    case DT_SHORT:
    case DT_INT:
      QSpinBox::setValue( static_cast<int>(value) );
      break;
    default:
      QSpinBox::setValue( static_cast<int>( value * m_DisplayFactor * pow(10,m_DecimalPlaces) ) );
      break;
  }
  
  QSpinBox::updateDisplay();
}

QString QmitkNumberPropertyEditor::mapValueToText(int value)
{
  QString displayedText;

  double d( value / pow(10,m_DecimalPlaces) );
  if ( m_DecimalPlaces > 0 )
  {
    QString formatString;
    formatString.sprintf("%%.%if", m_DecimalPlaces); // do copy before sprintf
    displayedText.sprintf( formatString , d * m_DisplayFactor);
  }
  else
    displayedText.sprintf( "%i" , static_cast<int>(d) );

  return displayedText;
}

int QmitkNumberPropertyEditor::mapTextToValue(bool* ok)
{
  if (ok) *ok = true; // indicate successful parsing
  return static_cast<int>( cleanText().toDouble() / m_DisplayFactor * pow(10,m_DecimalPlaces) );
}

void QmitkNumberPropertyEditor::PropertyChanged()
{
  DisplayNumber();
}

void QmitkNumberPropertyEditor::PropertyRemoved()
{
  m_Property = NULL;
}

void QmitkNumberPropertyEditor::DisplayNumber()
{
  if (! m_Property ) return;

  switch (m_DataType)
  {
    case DT_SHORT:
      {
        short s = m_ShortProperty->GetValue();
        QSpinBox::setValue( s );
        break;
      }
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
}

