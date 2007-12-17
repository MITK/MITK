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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

/**
 * \brief Updates the slider with the recent changes applied to the navigator.
 *
 * Intended to be called via event mechanism, e.g. if the connected
 * mitk::Stepper is modified.
 */
void QmitkSliderNavigator::Refetch()
{
  if ( !m_InRefetch )
  {
    m_InRefetch = true;
    
    m_Slider->setMinValue( 0 );
    m_Slider->setMaxValue( m_Stepper->GetSteps() - 1 );
    m_Slider->setValue( m_Stepper->GetPos() );
    
    m_SpinBox->setMinValue( 0 );
    m_SpinBox->setMaxValue( m_Stepper->GetSteps() - 1 );
    m_SpinBox->setValue( m_Stepper->GetPos() );

    if ( m_Stepper->HasRange() && m_HasLabels )
    {
      // Show slider with labels according to below settings
      m_SliderLabelLeft->setHidden( false );
      m_SliderLabelRight->setHidden( false );
    
      if ( m_Stepper->HasValidRange() )
      {
        this->SetLabelValuesValid( true, true );
        this->SetLabelValues( 
          m_Stepper->GetRangeMin(),
          m_Stepper->GetRangeMax() );
      }
      else
      {
        this->SetLabelValuesValid( false, false );
      }

      if ( m_Stepper->HasUnitName() )
      {
        this->SetLabelUnit( m_Stepper->GetUnitName() );
      }
    }
    else
    {
      // Show slider without any labels
      m_SliderLabelLeft->setHidden( true );
      m_SliderLabelRight->setHidden( true );
    }
  
    // Update GUI according to above settings
    this->SetLabels();
    
    m_InRefetch=false;
  }
}


void QmitkSliderNavigator::SetStepper( mitk::Stepper * stepper)
{
  m_Stepper = stepper;
  
  // this avoids trying to use m_Stepper until it is set to something != NULL 
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = (stepper==NULL);
}


void QmitkSliderNavigator::slider_valueChanged( int )
{
  if (!m_InRefetch)
  {
    m_Stepper->SetPos(m_Slider->value());
    this->Refetch();
  }
}

void QmitkSliderNavigator::init()
{
  // this avoids trying to use m_Stepper until it is set to something != NULL 
  // (additionally to the avoiding recursions during refetching)
  m_InRefetch = true;

  // Hide slider labeling -- until it is explicitly activated
  this->ShowLabels( false );

  // Set label values as invalid (N/A)
  this->SetLabelValuesValid( false, false );

  m_HasLabels = false;
  m_HasLabelUnit = true;
}

void QmitkSliderNavigator::ShowLabels( bool show )
{
  m_HasLabels = show;
}

/** 
 * \brief En-/disables displaying of the unit label (range will be displayed
 * without unit if enabled).
 */
void QmitkSliderNavigator::ShowLabelUnit( bool show )
{
  m_HasLabelUnit = show;
}

/**
 * \brief Set range minimum and maximum (displayed as labels left and right
 * of slider if enabled)
 */
void QmitkSliderNavigator::SetLabelValues( float min, float max )
{
  m_MinValue = min;
  m_MaxValue = max;
}

void QmitkSliderNavigator::SetLabelValuesValid( bool minValid, bool maxValid )
{
  m_MinValueValid = minValid;
  m_MaxValueValid = maxValid;
}

/**
 * \brief Set range unit (e.g. mm or ms) which will be displayed below range
 * labels if enabled.
 */
void QmitkSliderNavigator::SetLabelUnit( const char *unit )
{
  m_LabelUnit = unit;
}

QString QmitkSliderNavigator::GetLabelUnit()
{
  return m_LabelUnit;
}

/**
 * \brief Converts the passed value to a QString representation.
 *
 * If the value exceeds a certain maximum, "INF" (for "infinity") is displayed 
 * instead.
 */
QString QmitkSliderNavigator::ClippedValueToString( float value )
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

/**
 * \brief Returns range-minimum (displayed as label left of slider if enabled)
 */
QString QmitkSliderNavigator::GetMinValueLabel()
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

QString QmitkSliderNavigator::GetMaxValueLabel()
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

/**
 * \brief Configure slider with labels according to range and unit settings
 */
void QmitkSliderNavigator::SetLabels()
{
  QString minText = "<p align='center'><font size='2'>" 
    + this->GetMinValueLabel();

  QString maxText = "<p align='center'><font size='2'>" 
    + this->GetMaxValueLabel();
  
  if ( m_HasLabelUnit )
  {
    minText += "&nbsp;" + this->GetLabelUnit();
    maxText += "&nbsp;" + this->GetLabelUnit();
  }

  if ( m_MinValueValid )
  {
    minText += "<br>(pos&nbsp;0)";
  }

  if ( m_MaxValueValid )
  {
    maxText += "<br>(pos&nbsp;" + QString::number( m_Stepper->GetSteps() - 1 ) + ")";
  }

  minText += "</font></p>";
  maxText += "</font></p>";

  m_SliderLabelLeft->setText( minText );
  m_SliderLabelRight->setText( maxText );
}

void QmitkSliderNavigator::spinBox_valueChanged( int )
{
  if(!m_InRefetch)
  {
    m_Stepper->SetPos( m_SpinBox->value() );
    this->Refetch();
  }
}

int QmitkSliderNavigator::GetPos()
{
  return m_Stepper->GetPos();
}

void QmitkSliderNavigator::SetPos(int val)
{
  if (!m_InRefetch)    
  {
    m_Stepper->SetPos( val );
  }
}
