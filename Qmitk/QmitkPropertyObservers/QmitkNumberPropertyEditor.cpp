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
#include <QmitkNumberPropertyEditor.h>
#include <mitkRenderingManager.h>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4

#define ROUND(x)       (((x) > 0) ?   int((x) + 0.5) :   int((x) - 0.5))
#define ROUND_SHORT(x) (((x) > 0) ? short((x) + 0.5) : short((x) - 0.5))
/*
QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::GenericProperty<short>* property, QWidget* parent, const char* name )
: QSpinBox( parent, name ),
  PropertyEditor( property ),
  m_ShortProperty(property),
  m_DataType(DT_SHORT)
{
  initialize();
}
*/
QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::IntProperty* property, QWidget* parent, const char* name )
: QSpinBox( parent, name ),
  PropertyEditor( property ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::FloatProperty* property, QWidget* parent, const char* name )
: QSpinBox( parent, name ),
  PropertyEditor( property ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertyEditor::QmitkNumberPropertyEditor( mitk::DoubleProperty* property, QWidget* parent, const char* name )
: QSpinBox( parent, name ),
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
  setValidator(0);
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

  // commented line would set the default increase/decrease to 1.0, no matter how many decimal places are available
  //setLineStep( ROUND(m_FactorPropertyToSpinbox) );
       
  if ( m_ShowPercents )
  {
    m_FactorPropertyToSpinbox *= 100.0;
    //setLineStep( ROUND(0.01 *m_FactorPropertyToSpinbox) );
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
  return ROUND( QSpinBox::minValue() / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setMinValue(int value)
{
  QSpinBox::setMinValue( ROUND(value * m_FactorPropertyToSpinbox ) );
}

int QmitkNumberPropertyEditor::maxValue() const
{
  return ROUND( QSpinBox::maxValue() / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setMaxValue(int value)
{
  QSpinBox::setMaxValue( ROUND( value * m_FactorPropertyToSpinbox ) );
}

double QmitkNumberPropertyEditor::doubleValue() const
{
  return static_cast<double>((QSpinBox::value()) / m_FactorPropertyToSpinbox );
}

void QmitkNumberPropertyEditor::setDoubleValue(double value)
{
  QSpinBox::setValue( ROUND( value * m_FactorPropertyToSpinbox ) );
  QSpinBox::updateDisplay();
}

QString QmitkNumberPropertyEditor::mapValueToText(int value)
{
  QString displayedText;

  double d( value * m_FactorSpinboxToDisplay );

  if ( m_DecimalPlaces > 0 )
  {
    QString formatString;
    formatString.sprintf("%%.%if", m_DecimalPlaces); // do copy before sprintf
    displayedText.sprintf( formatString , d );
  }
  else
    displayedText.sprintf( "%i" , ROUND(d) );

  return displayedText;
}

int QmitkNumberPropertyEditor::mapTextToValue(bool* ok)
{
  if (ok) *ok = true; // indicate successful parsing

  return ROUND( cleanText().toDouble() / m_FactorSpinboxToDisplay );
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
  /*
    case DT_SHORT:
      {
        m_ShortProperty->SetValue(ROUND_SHORT(newValue));
        break;
      }
    */
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
  /*
    case DT_SHORT:
      {
        short s = m_ShortProperty->GetValue();
        QSpinBox::setValue( s );
        break;
      }
  */
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

