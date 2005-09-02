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
#include <mitkSurface.h>
#include <mitkDataTreeNode.h>
#include <qgrid.h>
#include <QmitkMaterialShowcase.h>
#include <qlayout.h>
#include <qcolordialog.h>
#include <qcolor.h>


/**
 * Contructor.
 */
void QmitkMaterialEditor::init()
{
    m_MaterialProperties = mitk::MaterialPropertyVectorContainer::New();
    m_OriginalMaterialProperties = mitk::MaterialPropertyVectorContainer::New();
    m_ActiveShowcase = 0;
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
}


/**
 * Slot which reacts on a color coefficient change by the user. Fills the current 
 * material property with the new coefficient and forwards it to the current 
 * showcase.
 * @param value the slider value, which represents the new coefficient. The slider
 * value is normalized to be in range of the color coefficient value type
 */
void QmitkMaterialEditor::OnColorCoefficientChanged( int value )
{
    //
    // convert value into color coefficient ni range [0..1]
    //
    /*
    VTK_FLOAT_TYPE coefficient = ( ( VTK_FLOAT_TYPE ) ( value - m_SlColorCoefficient->minValue() ) ) / ( ( VTK_FLOAT_TYPE ) ( m_SlColorCoefficient->maxValue() - m_SlColorCoefficient->minValue() ) );

    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetColorCoefficient( coefficient );
    m_Showcases[ m_ActiveShowcase ]->SetColorCoefficient( coefficient );
    */
}


/**
 * Slot which reacts on a specular coefficient change by the user. Fills the current 
 * material property with the new coefficient and forwards it to the current 
 * showcase.
 * @param value the slider value, which represents the new coefficient. The slider
 * value is normalized to be in range of the specular coefficient value type
 */
void QmitkMaterialEditor::OnSpecularCoefficientChanged( int value ) 
{
    //
    // convert value into specular coefficient ni range [0..1]
    //
    /*
    VTK_FLOAT_TYPE coefficient = ( ( VTK_FLOAT_TYPE ) ( value - m_SlSpecularCoefficient->minValue() ) ) / ( ( VTK_FLOAT_TYPE ) ( m_SlSpecularCoefficient->maxValue() - m_SlSpecularCoefficient->minValue() ) );

    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetSpecularCoefficient( coefficient );
    m_Showcases[ m_ActiveShowcase ]->SetSpecularCoefficient( coefficient );
    */
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
    VTK_FLOAT_TYPE power = ( VTK_FLOAT_TYPE ) value;
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetSpecularPower( power );
    m_Showcases[ m_ActiveShowcase ]->SetSpecularPower( power );
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
    VTK_FLOAT_TYPE opacity = ( ( VTK_FLOAT_TYPE ) ( value - m_SlOpacity->minValue() ) ) / ( ( VTK_FLOAT_TYPE ) ( m_SlOpacity->maxValue() - m_SlOpacity->minValue() ) );
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetOpacity( opacity );
    m_Showcases[ m_ActiveShowcase ]->SetOpacity( opacity );
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
        assert ( m_OriginalMaterialProperties->GetElement( i ) != NULL );
        assert ( m_MaterialProperties->GetElement( i ) != NULL );
        m_OriginalMaterialProperties->GetElement( i )->Initialize( * ( m_MaterialProperties->GetElement( i ) ) ) ;
    }
    this->accept();
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
 * @param material property the materials which should be edited. Note that
 * if the user presses OK, materialProperty will be modified. If Cancel is pressed.
 * the object remains unchanged.
 */
void QmitkMaterialEditor::Initialize( mitk::MaterialProperty* materialProperty )
{
    assert ( materialProperty != NULL );
    ClearMaterialProperties();
    m_OriginalMaterialProperties->push_back( materialProperty );
    m_MaterialProperties->push_back( new mitk::MaterialProperty( *materialProperty ) );
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
        m_OriginalMaterialProperties->push_back( materialPropertyVectorContainer->GetElement( i ) );
        m_MaterialProperties->push_back( new mitk::MaterialProperty( *( materialPropertyVectorContainer->GetElement( i ) ) ) );
    }
    Initialize();
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
    for ( unsigned int i = 0 ; i < m_MaterialProperties->size() ; ++i )
    {
        delete( m_MaterialProperties->GetElement( i ) );
    }
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
    //m_SlColor->SetValue();
    mitk::MaterialProperty* materialProperty = m_MaterialProperties->GetElement( number );
    assert ( materialProperty != NULL );
    m_SlColor->setValue( this->ColorToValue( materialProperty->GetColor() ) );
    m_SlCoefficients->setValue( static_cast<int> ( materialProperty->GetColorCoefficient() * ( m_SlCoefficients->maxValue() - m_SlCoefficients->minValue() ) + m_SlCoefficients->minValue() ) );
    //m_SlColorCoefficient->setValue( static_cast<int> ( materialProperty->GetColorCoefficient() * ( m_SlColorCoefficient->maxValue() - m_SlColorCoefficient->minValue() ) + m_SlColorCoefficient->minValue() ) );
    //m_SlSpecularCoefficient->setValue( static_cast<int> ( materialProperty->GetSpecularCoefficient() * ( m_SlSpecularCoefficient->maxValue() - m_SlSpecularCoefficient->minValue() ) + m_SlSpecularCoefficient->minValue() ) );
    m_SlSpecularPower->setValue( static_cast<int> ( materialProperty->GetSpecularPower() ) );
    m_SlOpacity->setValue( static_cast<int> ( materialProperty->GetOpacity() * ( m_SlOpacity->maxValue() - m_SlOpacity->minValue() ) + m_SlOpacity->minValue() ) );
}




/**
 * Converts a slider index into a color value.
 */
mitk::MaterialProperty::Color QmitkMaterialEditor::ValueToColor( int value )
{
    mitk::MaterialProperty::Color color;
    VTK_FLOAT_TYPE val = value;
    VTK_FLOAT_TYPE red = 0.0f;
    VTK_FLOAT_TYPE green = 0.0f;
    VTK_FLOAT_TYPE blue = 0.0f;
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
    VTK_FLOAT_TYPE* rgba = color.GetDataPointer();
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
    QColor result = QColorDialog::getColor( QColor( ( int ) ( color.GetRed() * 255 ), ( int ) ( color.GetGreen() * 255 ), ( int ) ( color.GetBlue() * 255 ) ) );
    if ( result.isValid() )
    {
        color.SetRed( result.red() / 255.0 );
        color.SetGreen( result.green() / 255.0 );
        color.SetBlue( result.blue() / 255.0 );
        m_MaterialProperties->GetElement( m_ActiveShowcase )->SetColor( color );
        m_Showcases[ m_ActiveShowcase ]->SetColor( color );
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
    VTK_FLOAT_TYPE coefficient = ( ( VTK_FLOAT_TYPE ) ( value - m_SlCoefficients->minValue() ) ) / ( ( VTK_FLOAT_TYPE ) ( m_SlCoefficients->maxValue() - m_SlCoefficients->minValue() ) );

    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetColorCoefficient( coefficient );
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetSpecularCoefficient( coefficient );
    m_Showcases[ m_ActiveShowcase ]->SetColorCoefficient( coefficient );
    m_Showcases[ m_ActiveShowcase ]->SetSpecularCoefficient( coefficient );
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
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetInterpolation( interpolation );
    m_Showcases[ m_ActiveShowcase ]->SetInterpolation( interpolation );
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
    m_MaterialProperties->GetElement( m_ActiveShowcase )->SetRepresentation( representation );
    m_Showcases[ m_ActiveShowcase ]->SetRepresentation( representation );
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
