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
#include <QmitkNumberPropertySlider.h>
#include <mitkRenderingManager.h>

#define DT_SHORT   1
#define DT_INT     2
#define DT_FLOAT   3
#define DT_DOUBLE  4

#define ROUND(x)       (((x) > 0) ?   int((x) + 0.5) :   int((x) - 0.5))
#define ROUND_SHORT(x) (((x) > 0) ? short((x) + 0.5) : short((x) - 0.5))

QmitkNumberPropertySlider::QmitkNumberPropertySlider( mitk::IntProperty* property, QWidget* parent, const char* name )
: QSlider( parent, name ),
  PropertyEditor( property ),
  m_IntProperty(property),
  m_DataType(DT_INT)
{
  initialize();
}

QmitkNumberPropertySlider::QmitkNumberPropertySlider( mitk::FloatProperty* property, QWidget* parent, const char* name )
: QSlider( parent, name ),
  PropertyEditor( property ),
  m_FloatProperty(property),
  m_DataType(DT_FLOAT)
{
  initialize();
}

QmitkNumberPropertySlider::QmitkNumberPropertySlider( mitk::DoubleProperty* property, QWidget* parent, const char* name )
: QSlider( parent, name ),
  PropertyEditor( property ),
  m_DoubleProperty(property),
  m_DataType(DT_DOUBLE)
{
  initialize();
}

QmitkNumberPropertySlider::~QmitkNumberPropertySlider()
{
}

void QmitkNumberPropertySlider::initialize()
{ // only to be called from constructors
  
  // spinbox settings
  //setValidator(0);
  //setSuffix("");
  setOrientation( Qt::Horizontal );
  
  // protected
  m_DecimalPlaces = 0;
  m_FactorPropertyToSlider = 1.0;
  m_FactorSliderToDisplay = 1.0;
  m_ShowPercents = false;
 
  // private
  m_SelfChangeLock = false;

  connect( this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)) );
 
  // display current value of our property
  DisplayNumber();
}

void QmitkNumberPropertySlider::adjustFactors(short newDecimalPlaces, bool newShowPercents)
{
  int oldMax = maxValue();
  int oldMin = minValue();

  m_DecimalPlaces = newDecimalPlaces;
  m_ShowPercents = newShowPercents;

  m_FactorPropertyToSlider = pow(10.0,m_DecimalPlaces);
  m_FactorSliderToDisplay = 1.0 / m_FactorPropertyToSlider;

  // commented line would set the default increase/decrease to 1.0, no matter how many decimal places are available
  //setLineStep( ROUND(m_FactorPropertyToSlider) );
       
  if ( m_ShowPercents )
  {
    m_FactorPropertyToSlider *= 100.0;
    //setLineStep( ROUND(0.01 *m_FactorPropertyToSlider) );
    //setSuffix("%");
  }
  else
  {
    //setSuffix("");
  }
  
  setMinValue(oldMin);
  setMaxValue(oldMax);
}

short QmitkNumberPropertySlider::getDecimalPlaces() const
{
  return m_DecimalPlaces;
}

void QmitkNumberPropertySlider::setDecimalPlaces(short places)
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

bool QmitkNumberPropertySlider::getShowPercent() const
{
  return m_ShowPercents;
}

void QmitkNumberPropertySlider::setShowPercent(bool showPercent)
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

int QmitkNumberPropertySlider::minValue() const
{
  return ROUND( QSlider::minValue() / m_FactorPropertyToSlider );
}

void QmitkNumberPropertySlider::setMinValue(int value)
{
  QSlider::setMinValue( ROUND(value * m_FactorPropertyToSlider ) );
}

int QmitkNumberPropertySlider::maxValue() const
{
  return ROUND( QSlider::maxValue() / m_FactorPropertyToSlider );
}

void QmitkNumberPropertySlider::setMaxValue(int value)
{
  QSlider::setMaxValue( ROUND( value * m_FactorPropertyToSlider ) );
}

double QmitkNumberPropertySlider::doubleValue() const
{
  return static_cast<double>((QSlider::value()) / m_FactorPropertyToSlider );
}

void QmitkNumberPropertySlider::setDoubleValue(double value)
{
  QSlider::setValue( ROUND( value * m_FactorPropertyToSlider ) );
  //QSlider::updateDisplay();
}

void QmitkNumberPropertySlider::onValueChanged(int value)
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
  
  double newValue( value / m_FactorPropertyToSlider );
    
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

void QmitkNumberPropertySlider::PropertyChanged()
{
  DisplayNumber();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNumberPropertySlider::PropertyRemoved()
{
  m_Property = NULL;
}

void QmitkNumberPropertySlider::DisplayNumber()
{
  if (! m_Property ) return;

  m_SelfChangeLock = true;
  switch (m_DataType)
  {
  /*
    case DT_SHORT:
      {
        short s = m_ShortProperty->GetValue();
        QSlider::setValue( s );
        break;
      }
  */
    case DT_INT:
      {
        int i = m_IntProperty->GetValue();
        QSlider::setValue( i );
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

