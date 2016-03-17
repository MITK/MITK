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

#include "QmlMitkSliderNavigatorItem.h"

QmlMitkSliderNavigatorItem::QmlMitkSliderNavigatorItem(QQuickItem* parent)
 : QQuickItem(parent)
{

  // this avoids trying to use m_Stepper until it is set to something != NULL
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = true;
  m_HasLabels = false;
  m_HasLabelUnit = true;
  m_InverseDirection = false;
}

void QmlMitkSliderNavigatorItem::setMin(double min)
{
    this->m_Min = min;
}

void QmlMitkSliderNavigatorItem::setMax(double max)
{
    this->m_Max = max;
}

void QmlMitkSliderNavigatorItem::setValue(double value)
{
    this->m_Value = value;

    if(!m_InRefetch)
    {
        if (m_InverseDirection)
        {
            m_Stepper->SetPos( m_Stepper->GetSteps()-1-value );
        }
        else
        {
            m_Stepper->SetPos( value );
        }
        this->Refetch();
    }
}

double QmlMitkSliderNavigatorItem::getMax()
{
    return this->m_Max;
}

double QmlMitkSliderNavigatorItem::getMin()
{
    return this->m_Min;
}

double QmlMitkSliderNavigatorItem::getValue()
{
    return this->m_Value;
}

void QmlMitkSliderNavigatorItem::Refetch()
{
    if (!m_InRefetch)
    {
        m_InRefetch = true;

        this->m_Min = 0;
        this->m_Max = m_Stepper->GetSteps()-1;

        if(m_InverseDirection)
            this->m_Value = m_Stepper->GetSteps()-1-m_Stepper->GetPos();
        else
            this->m_Value = m_Stepper->GetPos();

        m_InRefetch = false;

        emit this->sync();
    }
}


void QmlMitkSliderNavigatorItem::SetStepper( mitk::Stepper * stepper)
{
  m_Stepper = stepper;

  // this avoids trying to use m_Stepper until it is set to something != NULL
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = (stepper==nullptr);
}

void QmlMitkSliderNavigatorItem::ShowLabels( bool show )
{
  m_HasLabels = show;
}

void QmlMitkSliderNavigatorItem::ShowLabelUnit( bool show )
{
  m_HasLabelUnit = show;
}

void QmlMitkSliderNavigatorItem::SetLabelValues( float min, float max )
{
  m_MinValue = min;
  m_MaxValue = max;
}

void QmlMitkSliderNavigatorItem::SetLabelValuesValid( bool minValid, bool maxValid )
{
  m_MinValueValid = minValid;
  m_MaxValueValid = maxValid;
}

void QmlMitkSliderNavigatorItem::SetLabelUnit( const char *unit )
{
  m_LabelUnit = unit;
}

QString QmlMitkSliderNavigatorItem::GetLabelUnit()
{
  return m_LabelUnit;
}

QString QmlMitkSliderNavigatorItem::ClippedValueToString( float value )
{
  if ( value < -10000000.0 )
  {
    return "-INF";
  }
  else if ( value > 10000000.0 )
  {
    return "+INF";
  }
  else
  {
    return QString::number( value, 'f', 2 );
  }
}

QString QmlMitkSliderNavigatorItem::GetMinValueLabel()
{
  if ( m_MinValueValid )
  {
    return this->ClippedValueToString( m_MinValue );
  }
  else
  {
    return "N/A";
  }
}

QString QmlMitkSliderNavigatorItem::GetMaxValueLabel()
{
  if ( m_MaxValueValid )
  {
    return this->ClippedValueToString( m_MaxValue );
  }
  else
  {
    return "N/A";
  }
}

int QmlMitkSliderNavigatorItem::GetPos()
{
  return m_Stepper->GetPos();
}

void QmlMitkSliderNavigatorItem::SetPos(int val)
{
  if (!m_InRefetch)
  {
    m_Stepper->SetPos( val );
  }
}

void QmlMitkSliderNavigatorItem::SetInverseDirection(bool inverseDirection)
{
  m_InverseDirection = inverseDirection;
}

void QmlMitkSliderNavigatorItem::create()
{
    qmlRegisterType<QmlMitkSliderNavigatorItem>("Mitk.Views", 1, 0, "SliderNavigator");
}
