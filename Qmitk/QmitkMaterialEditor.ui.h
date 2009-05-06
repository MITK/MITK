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
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/




/**
 * The widget QmitkMaterialEditor gives the user the possibility to edit
 * materials associated with an object. Some things are simplified to make it
 * easier for the user. The specular and color coefficients may not be edited
 * separately, since it is hard to get good-looking results. Thus, they are
 * modified together via a single slider. Additionally, the specular color may
 * not be changed.
 * Note that not only one material property may be edited at once, but also
 * a whole list of material properties. THis may be useful for editing visua-
 * lization properties of vessels.
 * Please note also, that one of the 2 Initialize(...) methods MUST be called
 * after creation of the dialog, to initialize the showcases!
 */




#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <qgrid.h>
#include <QmitkMaterialShowcase.h>
#include <qlayout.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include <mitkRenderingManager.h>


/**
 * Contructor.
 */
void QmitkMaterialEditor::init()
{
    m_MaterialProperties = mitk::MaterialPropertyVectorContainer::New();
    m_OriginalMaterialProperties = mitk::MaterialPropertyVectorContainer::New();
    m_ActiveShowcase = 0;
    m_Inline = false; // normally not inside some popup menu with live feedback
}


/**
 * Destructor
 */
void QmitkMaterialEditor::destroy()
{
    ClearMaterialProperties();
}


/**
 * Slot which reacts on a color change by the user. Fills the current material
 * property with the new color and forwards it to the current showcase.
 * @param value the slider value, which represents the new color.
 */
void QmitkMaterialEditor::OnColorChanged( int value )
{
    mitk::MaterialProperty::Color color = this->ValueToColor( value );
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetColor( color );
    m_Showcases[ m_ActiveShowcase ]->SetColor( color );

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}



/**
 * Slot which reacts on a specular power change by the user. Fills the current
 * material property with the new coefficient and forwards it to the current
 * showcase.
 * @param value the slider value, which represents the new coefficient. The slider
 * value is normalized to be in range of the specular powers value range
 */
void QmitkMaterialEditor::OnSpecularPowerChanged( int value )
{
    vtkFloatingPointType power = ( vtkFloatingPointType ) value;
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetSpecularPower( power );
      m_Showcases[ index ]->SetSpecularPower( power );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


/**
 * Slot which reacts on a opacity change by the user. Fills the current
 * material property with the new coefficient and forwards it to the current
 * showcase.
 * @param value the slider value, which represents the new opacity. The slider
 * value is normalized to be in range of the opacity value range
 */
void QmitkMaterialEditor::OnOpacityChanged( int value )
{
    vtkFloatingPointType opacity = ( ( vtkFloatingPointType ) ( value - m_SlOpacity->minValue() ) ) / ( ( vtkFloatingPointType ) ( m_SlOpacity->maxValue() - m_SlOpacity->minValue() ) );
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetOpacity( opacity );
      m_Showcases[ index ]->SetOpacity( opacity );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

/**
 * Slot which reacts on a line width change by the user. Fills the current
 * material property with the new line width and forwards it to the current
 * showcase.
 * @param value the slider value, which represents the line width.
 */
void QmitkMaterialEditor::OnLineWidthChanged( int value )
{
    float lineWidth = static_cast<float>(value);
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetLineWidth( lineWidth );
      m_Showcases[ index ]->SetLineWidth( lineWidth );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
/**
 * Slot which reacts on a button-OK click. The temporary materials are copied
 * to the ones provided by the user of the dialog. The dialog will be accepted
 */
void QmitkMaterialEditor::OnOKClicked()
{
    //
    // if we accepted the settings, materials are copied from m_MaterialProperties to m_OriginalMaterialProperties
    //
    assert ( m_OriginalMaterialProperties.IsNotNull() && m_MaterialProperties.IsNotNull() );
    assert ( m_OriginalMaterialProperties->size() == m_MaterialProperties->size() );
    for ( unsigned int i = 0 ; i < m_MaterialProperties->size() ; ++i )
    {
        assert ( m_OriginalMaterialProperties->GetElement( i ).IsNotNull() );
        assert ( m_MaterialProperties->GetElement( i ).IsNotNull() );
        m_OriginalMaterialProperties->GetElement( i )->Initialize( * ( m_MaterialProperties->GetElement( i ) ), false ) ;
    }

    if (m_Inline)
    {
      emit ChangesAccepted(this);
      if ( isVisible() && parentWidget() && parentWidget()->inherits("QPopupMenu") )
      {
        parentWidget()->close();
      }
    }
    else
    {
      this->accept();
    }
}


/**
 * Slot which reats on a button-Cancel click. The dialog will be closed and the
 * results rejected.
 */
void QmitkMaterialEditor::OnCancelClicked()
{
    this->reject();
}


/**
 * Slot which reacts on a button-Preview click. Not implemented yet.
 */
void QmitkMaterialEditor::OnPreviewClicked()
{
    std::cout << "Sorry, preview not yet implemented!" << std::endl;
}


/**
 * Initializes the dialog with the materials given in material property
 * @param materialProperty property the materials which should be edited. Note that
 * if the user presses OK, materialProperty will be modified. If Cancel is pressed.
 * the object remains unchanged.
 */
void QmitkMaterialEditor::Initialize( mitk::MaterialProperty* materialProperty )
{
    assert ( materialProperty != NULL );
    ClearMaterialProperties();
    mitk::MaterialProperty::Pointer internalProperty;
    if (m_Inline)
    {
      internalProperty = materialProperty;
    }
    else
    {
      internalProperty = mitk::MaterialProperty::New( *materialProperty );
      internalProperty->SetDataTreeNode( NULL );
      internalProperty->SetRenderer( NULL );
    }
    m_MaterialProperties->push_back( internalProperty );
    m_OriginalMaterialProperties->push_back( materialProperty );
    Initialize();
}


/**
 * Initializes the dialog with the materials given in the materialPropertyVectorContainer
 * @param materialPropertyVectorContainer the materials which should be edited. Note that
 * if the user presses OK, materialPropertyVectorContainer will be modified. If Cancel is pressed.
 * the object remains unchanged.
 */
void QmitkMaterialEditor::Initialize( mitk::MaterialPropertyVectorContainer::Pointer materialPropertyVectorContainer )
{
    assert ( materialPropertyVectorContainer.IsNotNull() );
    ClearMaterialProperties();
    for ( unsigned int i = 0 ; i < materialPropertyVectorContainer->size() ; ++i )
    {
        mitk::MaterialProperty::Pointer materialProperty = materialPropertyVectorContainer->GetElement( i );
        mitk::MaterialProperty::Pointer internalProperty = mitk::MaterialProperty::New( *materialProperty );
        internalProperty->SetDataTreeNode( NULL );
        internalProperty->SetRenderer( NULL );
        m_MaterialProperties->push_back( mitk::MaterialProperty::New( *materialProperty ) );
        m_OriginalMaterialProperties->push_back( materialProperty );
    }
    Initialize();

    if ( m_Inline )
    {
      m_PbOK->setText(tr("Apply"));
      m_PbOK->setPaletteBackgroundColor( Qt::green ); // be more offensive
      m_PbOK->show();
    }
}

/**
 * Fills a grid with a set of showcases, one for each object in m_MaterialProperties.
 */
void QmitkMaterialEditor::Initialize()
{
    QVBoxLayout * layout = new QVBoxLayout( m_GridContainer );
    QGrid * grid = new QGrid( static_cast<int>( sqrt( (double)m_MaterialProperties->size() ) ), Qt::Horizontal, m_GridContainer );
    layout->addWidget( grid );
    m_Showcases.resize( m_MaterialProperties->size() );
    for ( unsigned int i = 0 ; i < m_MaterialProperties->size() ; ++i )
    {
        QmitkMaterialShowcase* showcase = new QmitkMaterialShowcase( grid );
        showcase->SetShowcaseNumber( i );
        showcase->SetMaterialProperty( m_MaterialProperties->GetElement( i ) );
        connect( showcase, SIGNAL(Selected(QmitkMaterialShowcase*)), this, SLOT(OnMaterialShowcaseSelected(QmitkMaterialShowcase*) ) );
        m_Showcases[ i ] = showcase;
    }
    SetActiveShowcase( 0 );
}


/**
 * Deletes all temporary objects.
 */
void QmitkMaterialEditor::ClearMaterialProperties()
{
    m_MaterialProperties->clear();
    m_OriginalMaterialProperties->clear();
    m_Showcases.clear();
}

/**
 * Performs everything which is needed for a change of the active showcase
 * @param number the new active showcase
 */
void QmitkMaterialEditor::SetActiveShowcase( unsigned int number )
{
    if ( m_MaterialProperties.IsNull() )
    {
        std::cout << "Warning: m_MaterialProperties is NULL!" << std::endl;
        return ;
    }
    if ( number < m_MaterialProperties->size() )
        m_ActiveShowcase = number;
    else
        return ;
    //
    // update sliders
    //
    mitk::MaterialProperty* materialProperty = m_MaterialProperties->GetElement( number );
    assert ( materialProperty != NULL );
    int value = this->ColorToValue( materialProperty->GetColor() );
    m_SlColor->blockSignals(true);
    if ( value != -1 )
    {
        m_SlColor->setValue( value );
    }
    else
    {
        m_SlColor->setValue( m_SlColor->minValue() );
    }
    m_SlColor->blockSignals(false);
    m_SlCoefficients->blockSignals(true);
    m_SlCoefficients->setValue( static_cast<int> ( materialProperty->GetColorCoefficient() * ( m_SlCoefficients->maxValue() - m_SlCoefficients->minValue() ) + m_SlCoefficients->minValue() ) );
    m_SlCoefficients->blockSignals(false);
    m_SlSpecularPower->blockSignals(true);
    m_SlSpecularPower->setValue( static_cast<int> ( materialProperty->GetSpecularPower() ) );
    m_SlSpecularPower->blockSignals(false);
    m_SlOpacity->blockSignals(true);
    m_SlOpacity->setValue( static_cast<int> ( materialProperty->GetOpacity() * ( m_SlOpacity->maxValue() - m_SlOpacity->minValue() ) + m_SlOpacity->minValue() ) );
    m_SlOpacity->blockSignals(false);
    m_SlLineWidth->blockSignals(true);
    m_SlLineWidth->setValue( static_cast<int> ( materialProperty->GetLineWidth() ) );
    m_SlLineWidth->blockSignals(false);
    m_CbInterpolation->blockSignals(true);
    m_CbInterpolation->setCurrentItem( materialProperty->GetVtkInterpolation() );
    m_CbInterpolation->blockSignals(false);
    m_CbRepresentation->blockSignals(true);
    m_CbRepresentation->setCurrentItem( materialProperty->GetVtkRepresentation() );
    m_CbRepresentation->blockSignals(false);
}




/**
 * Converts a slider index into a color value.
 */
mitk::MaterialProperty::Color QmitkMaterialEditor::ValueToColor( int value )
{
    mitk::MaterialProperty::Color color;
    vtkFloatingPointType val = value;
    vtkFloatingPointType red = 0.0f;
    vtkFloatingPointType green = 0.0f;
    vtkFloatingPointType blue = 0.0f;
    if ( val < 85.0f )
    {
        red = ( 85.0f - val ) / 85.0f;
        green = val / 85.0f;
    }
    else if ( val < 170.0f )
    {
        green = ( 170.0f - val ) / 85.0f;
        blue = ( val - 85.0f ) / 85.0f;
    }
    else
    {
        blue = ( 255.0f - val ) / 85.0f;
        red = ( val - 170.0f ) / 85.0f ;
    }

    color.SetRed( red );
    color.SetGreen( green );
    color.SetBlue( blue );

    //std::cout << "value : " << value << ", color: " << color << std::endl;

    return color;
}

/**
 * Converts a color value into a slider index.
 */
int QmitkMaterialEditor::ColorToValue( mitk::MaterialProperty::Color color )
{
    int value = 0;
    vtkFloatingPointType* rgba = color.GetDataPointer();

    //
    // Mindestens ein Wert muss 0 sein und die Summe der anderen
    // Beiden 1. Ansonsten kommt die Farbe nicht aus dem Farbraum.
    //

    // check that at least one value is 0
    if ( rgba[0] > 0.00001 &&
         rgba[1] > 0.00001 &&
         rgba[2] > 0.00001
       )
    {
      return -1;
    }

    // check that the sum is 1
    if ( fabs ( rgba[0] + rgba[1] + rgba[2] - 1.0 ) > 0.00001 )
    {
        return -1;
    }

    //
    // now we know, that the color is from the given color space an can convert
    // it back to a slider value
    //
    if ( rgba[ 1 ] == 0.0f && rgba[ 2 ] == 0.0f )
        value = 0;
    else if ( rgba[ 0 ] == 0.0f && rgba[ 2 ] == 0.0f )
        value = 85;
    else if ( rgba[ 0 ] == 0.0f && rgba[ 1 ] == 0.0f )
        value = 170;
    // then the rest:
    else if ( rgba[ 0 ] > 0.0f && rgba[ 1 ] > 0.0f )
        value = ( int ) ( rgba[ 1 ] * 85.0f );
    else if ( rgba[ 1 ] > 0.0f && rgba[ 2 ] > 0.0f )
        value = ( int ) ( rgba[ 2 ] * 85.0f ) + 85;
    else
        value = ( int ) ( rgba[ 0 ] * 85.0f ) + 170;
    //std::cout << "color: " << color << ", value : " << value << std::endl;
    return value;
}



/**
 * Slot to react on a button-Select Color click. A qcolordialog is displayed,
 * and the result is forwarded to the active material and showcase
 */
void QmitkMaterialEditor::OnColorSelectButtonClickedClicked()
{
    mitk::MaterialProperty::Color color = m_MaterialProperties->GetElement( m_ActiveShowcase )->GetColor( );
    QColor result = QColorDialog::getColor( QColor( ( int ) ( color.GetRed() * 255 ), ( int ) ( color.GetGreen() * 255 ), ( int ) ( color.GetBlue() * 255 ) ), this );
    if ( result.isValid() )
    {
      color.SetRed( result.red() / 255.0 );
      color.SetGreen( result.green() / 255.0 );
      color.SetBlue( result.blue() / 255.0 );
      m_MaterialProperties->GetElement( m_ActiveShowcase )->SetColor( color );
      m_Showcases[ m_ActiveShowcase ]->SetColor( color );

      if (m_Inline )
      {
        OnOKClicked(); // here we are being closed because inside some popup menu, so the user can't possibly hit the accept button anymore
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
}


/**
 * Slot which reacts on a coefficient change by the user. Fills the current
 * material property with the new coefficient and forwards it to the current
 * showcase. The coefficient whill be used for BOTH color coefficient and specular
 * coefficient.
 * @param value the slider value, which represents the new coefficient. The slider
 * value is normalized to be in range of the coefficient value type
 */
void QmitkMaterialEditor::OnCoefficientsChanged( int value )
{
    //
    // convert value into color coefficient in range [0..1]
    //
    vtkFloatingPointType coefficient = ( ( vtkFloatingPointType ) ( value - m_SlCoefficients->minValue() ) ) / ( ( vtkFloatingPointType ) ( m_SlCoefficients->maxValue() - m_SlCoefficients->minValue() ) );
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetColorCoefficient( coefficient );
      m_MaterialProperties->GetElement( index )->SetSpecularCoefficient( coefficient );
      m_Showcases[ index ]->SetColorCoefficient( coefficient );
      m_Showcases[ index ]->SetSpecularCoefficient( coefficient );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

/**
 * Reacts on a change of the interpolation type by the user.
 * The new inerpolation type is forwarded to the active material and the
 * current showcase.
 */
void QmitkMaterialEditor::OnInterpolationChanged( int item )
{
    assert ( item >= 0 && item <= 2 );
    mitk::MaterialProperty::InterpolationType interpolation = static_cast<mitk::MaterialProperty::InterpolationType>( item );
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetInterpolation( interpolation );
      m_Showcases[ index ]->SetInterpolation( interpolation );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


/**
 * Reacts on a change of the representation type by the user.
 * The new inerpolation type is forwarded to the active material and the
 * current showcase.
 */
void QmitkMaterialEditor::OnRepresentationChanged( int item )
{
    assert ( item >= 0 && item <= 2 );
    mitk::MaterialProperty::RepresentationType representation = static_cast<mitk::MaterialProperty::RepresentationType>( item );
    for ( unsigned int index = 0 ; index < m_MaterialProperties->Size() ; ++index )
    {
      m_MaterialProperties->GetElement( index )->SetRepresentation( representation );
      m_Showcases[ index ]->SetRepresentation( representation );
    }

    if (m_Inline) mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkMaterialEditor::OnMaterialShowcaseSelected( QmitkMaterialShowcase * showcase )
{
    if ( showcase != NULL )
    {
        unsigned int showcaseNumber = showcase->GetShowcaseNumber();
        if ( showcaseNumber != m_ActiveShowcase )
        {
            assert ( showcaseNumber < m_Showcases.size() );
            SetActiveShowcase( showcaseNumber );
        }
    }
    else
    {
        std::cout << "Warning: showcase is NULL!" << std::endl;
    }
}


mitk::MaterialPropertyVectorContainer* QmitkMaterialEditor::GetMaterialProperties()
{
  return m_OriginalMaterialProperties.GetPointer();
}


void QmitkMaterialEditor::setInline( bool i )
{
  m_Inline = i;
  if (i)
  {
    m_PbOK->hide();
    m_PbCancel->hide();
    //m_PbSelectColor->hide();
  }
  else
  {
    m_PbOK->show();
    m_PbCancel->show();
    //m_PbSelectColor->show();
  }
}
