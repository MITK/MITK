#include "mitkMaterialProperty.h"
#include <vtkProperty.h>

mitk::MaterialProperty::MaterialProperty( Color color, vtkScalarType opacity )
{
    m_Color = color;
    m_ColorCoefficient = 1.0f;
    m_SpecularColor.Set( 1.0f, 1.0f, 1.0f );
    m_SpecularCoefficient = 1.0f;
    m_SpecularPower = 10.0f;
    m_Opacity = opacity;
    m_Interpolation = Gouraud;
    m_Representation = Surface;
}



mitk::MaterialProperty::MaterialProperty( vtkScalarType red, vtkScalarType green, vtkScalarType blue, vtkScalarType opacity )
{
    m_Color.Set( red, green, blue );
    m_ColorCoefficient = 1.0f;
    m_SpecularColor.Set( 1.0f, 1.0f, 1.0f );
    m_SpecularCoefficient = 1.0f;
    m_SpecularPower = 10.0f;
    m_Opacity = opacity;
    m_Interpolation = Gouraud;
    m_Representation = Surface;
}



mitk::MaterialProperty::MaterialProperty( vtkScalarType red, vtkScalarType green, vtkScalarType blue, vtkScalarType colorCoefficient, vtkScalarType specularCoefficient, vtkScalarType specularPower, vtkScalarType opacity )
{
    m_Color.Set( red, green, blue );
    m_ColorCoefficient = colorCoefficient;
    m_SpecularColor.Set( 1.0f, 1.0f, 1.0f );
    m_SpecularCoefficient = specularCoefficient;
    m_SpecularPower = specularPower;
    m_Opacity = opacity;
    m_Interpolation = Gouraud;
    m_Representation = Surface;
}


mitk::MaterialProperty::MaterialProperty( mitk::MaterialProperty::Color color, vtkScalarType colorCoefficient, vtkScalarType specularCoefficient, vtkScalarType specularPower, vtkScalarType opacity )
{
    m_Color = color;
    m_ColorCoefficient = colorCoefficient;
    m_SpecularColor.Set( 1.0f, 1.0f, 1.0f );
    m_SpecularCoefficient = specularCoefficient;
    m_SpecularPower = specularPower;
    m_Opacity = opacity;
    m_Interpolation = Gouraud;
    m_Representation = Surface;
}


mitk::MaterialProperty::MaterialProperty()
{
    m_Color.Set( 0.5f, 0.5f, 0.0f );
    m_ColorCoefficient = 1.0f;
    m_SpecularColor.Set( 1.0f, 1.0f, 1.0f );
    m_SpecularCoefficient = 1.0f;
    m_SpecularPower = 10.0f;
    m_Opacity = 1.0f;
    m_Interpolation = Gouraud;
    m_Representation = Surface;
}

mitk::MaterialProperty::MaterialProperty( const MaterialProperty& property )
{
    Initialize( property );
}

void mitk::MaterialProperty::SetColor( mitk::MaterialProperty::Color color )
{
    m_Color = color;
}


void mitk::MaterialProperty::SetColor( vtkScalarType red, vtkScalarType green, vtkScalarType blue )
{
    m_Color.Set( red, green, blue );
}


void mitk::MaterialProperty::SetColorCoefficient( vtkScalarType coefficient )
{
    m_ColorCoefficient = coefficient;
}

void mitk::MaterialProperty::SetSpecularColor( mitk::MaterialProperty::Color specularColor )
{
    m_SpecularColor = specularColor;
}


void mitk::MaterialProperty::SetSpecularColor( vtkScalarType red, vtkScalarType green, vtkScalarType blue )
{
    m_SpecularColor.Set( red, green, blue );
}

void mitk::MaterialProperty::SetSpecularCoefficient( vtkScalarType specularCoefficient )
{
    m_SpecularCoefficient = specularCoefficient;
}


void mitk::MaterialProperty::SetSpecularPower( vtkScalarType specularPower )
{
    m_SpecularPower = specularPower;
}


void mitk::MaterialProperty::SetOpacity( vtkScalarType opacity )
{
    m_Opacity = opacity;
}


void mitk::MaterialProperty::SetInterpolation( InterpolationType interpolation )
{
    m_Interpolation = interpolation;
}


void mitk::MaterialProperty::SetRepresentation( RepresentationType representation )
{
    m_Representation = representation;
}


mitk::MaterialProperty::Color mitk::MaterialProperty::GetColor() const
{
    return m_Color;
}


vtkScalarType mitk::MaterialProperty::GetColorCoefficient() const
{
    return m_ColorCoefficient;
}

mitk::MaterialProperty::Color mitk::MaterialProperty::GetSpecularColor() const
{
    return m_SpecularColor;
}

vtkScalarType mitk::MaterialProperty::GetSpecularCoefficient() const
{
    return m_SpecularCoefficient;
}


vtkScalarType mitk::MaterialProperty::GetSpecularPower() const
{
    return m_SpecularPower;
}


vtkScalarType mitk::MaterialProperty::GetOpacity() const
{
    return m_Opacity;
}

mitk::MaterialProperty::InterpolationType mitk::MaterialProperty::GetInterpolation() const
{
    return m_Interpolation;
}

mitk::MaterialProperty::RepresentationType mitk::MaterialProperty::GetRepresentation() const
{
    return m_Representation;
}


int mitk::MaterialProperty::GetVtkInterpolation() const
{
    switch ( m_Interpolation )
    {
      case( Flat ) : return VTK_FLAT;
      case( Gouraud ) : return VTK_GOURAUD;
      case( Phong ) : return VTK_PHONG;
    }
    return VTK_GOURAUD;
}

int mitk::MaterialProperty::GetVtkRepresentation() const
{
    switch ( m_Representation )
    {
      case( Points ) : return VTK_POINTS;
      case( Wireframe ) : return VTK_WIREFRAME;
      case( Surface ) : return VTK_SURFACE;
    }
    return VTK_SURFACE;
}



void mitk::MaterialProperty::Initialize( const MaterialProperty& property )
{
    m_Color = property.GetColor();
    m_ColorCoefficient = property.GetColorCoefficient();
    m_SpecularColor = property.GetSpecularColor();
    m_SpecularCoefficient = property.GetSpecularCoefficient();
    m_SpecularPower = property.GetSpecularPower();
    m_Opacity = property.GetOpacity();
    m_Interpolation = property.GetInterpolation();
    m_Representation = property.GetRepresentation();
}


bool mitk::MaterialProperty::operator==( const BaseProperty& property ) const
{
    const Self * other = dynamic_cast<const Self*>( &property );

    if ( other == NULL )
        return false;
    else
        return ( m_Color == other->GetColor() &&
                 m_ColorCoefficient == other->GetColorCoefficient() &&
                 m_SpecularColor == other->GetSpecularColor() &&
                 m_SpecularCoefficient == other->GetSpecularCoefficient() &&
                 m_SpecularPower == other->GetSpecularPower() &&
                 m_Opacity == other->GetOpacity() &&
                 m_Interpolation == other->GetInterpolation() &&
                 m_Representation == other->GetRepresentation()
               );
}
