#include "mitkMaterialProperty.h"
#include "mitkProperties.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include <vtkProperty.h>
#include "mitkDataTreeNode.h"
#include "mitkBaseRenderer.h"


mitk::MaterialProperty::MaterialProperty( Color color, vtkFloatingPointType opacity, mitk::DataTreeNode* node, mitk::BaseRenderer* renderer )
{
  InitializeStandardValues();
  SetDataTreeNode( node );
  SetRenderer( renderer );
  SetColor( color );
  SetColorCoefficient( GetColorCoefficient() );
  SetSpecularColor( GetSpecularColor() );
  SetSpecularCoefficient( GetSpecularCoefficient() );
  SetSpecularPower( GetSpecularPower() );
  SetOpacity( opacity );
  SetInterpolation( GetInterpolation() );
  SetRepresentation( GetRepresentation() );
  m_Name = "";
}



mitk::MaterialProperty::MaterialProperty( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity, mitk::DataTreeNode* node, mitk::BaseRenderer* renderer )
{
  InitializeStandardValues();
  SetDataTreeNode( node );
  SetRenderer( renderer );
  SetColor( red, green, blue );
  SetColorCoefficient( GetColorCoefficient() );
  SetSpecularColor( GetSpecularColor() );
  SetSpecularCoefficient( GetSpecularCoefficient() );
  SetSpecularPower( GetSpecularPower() );
  SetOpacity( opacity );
  SetInterpolation( GetInterpolation() );
  SetRepresentation( GetRepresentation() );
  m_Name = "";
}



mitk::MaterialProperty::MaterialProperty( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue,
    vtkFloatingPointType colorCoefficient, vtkFloatingPointType specularCoefficient,
    vtkFloatingPointType specularPower, vtkFloatingPointType opacity, mitk::DataTreeNode* node, mitk::BaseRenderer* renderer )
{
  InitializeStandardValues();
  SetDataTreeNode( node );
  SetRenderer( renderer );
  SetColor( red, green, blue );
  SetColorCoefficient( colorCoefficient );
  SetSpecularColor( GetSpecularColor() );
  SetSpecularCoefficient( specularCoefficient );
  SetSpecularPower( specularPower );
  SetOpacity( opacity );
  SetInterpolation( GetInterpolation() );
  SetRepresentation( GetRepresentation() );
  m_Name = "";
}


mitk::MaterialProperty::MaterialProperty( mitk::MaterialProperty::Color color, vtkFloatingPointType colorCoefficient, vtkFloatingPointType specularCoefficient, vtkFloatingPointType specularPower, vtkFloatingPointType opacity, mitk::DataTreeNode* node, mitk::BaseRenderer* renderer )
{
  InitializeStandardValues();
  SetDataTreeNode( node );
  SetRenderer( renderer );
  SetColor( color );
  SetColorCoefficient( colorCoefficient );
  SetSpecularColor( GetSpecularColor() );
  SetSpecularCoefficient( specularCoefficient );
  SetSpecularPower( specularPower );
  SetOpacity( opacity );
  SetInterpolation( GetInterpolation() );
  SetRepresentation( GetRepresentation() );
}


mitk::MaterialProperty::MaterialProperty( mitk::DataTreeNode* node, mitk::BaseRenderer* renderer )
{
  InitializeStandardValues();
  SetDataTreeNode( node );
  SetRenderer( renderer );
  SetColor( GetColor() );
  SetColorCoefficient( GetColorCoefficient() );
  SetSpecularColor( GetSpecularColor() );
  SetSpecularCoefficient( GetSpecularCoefficient() );
  SetSpecularPower( GetSpecularPower() );
  SetOpacity( GetOpacity() );
  SetInterpolation( GetInterpolation() );
  SetRepresentation( GetRepresentation() );
}

mitk::MaterialProperty::MaterialProperty( const MaterialProperty& property ) : mitk::BaseProperty()
{
  Initialize( property );
}

mitk::MaterialProperty::MaterialProperty( const MaterialProperty& property, vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity, std::string name )
{
  Initialize( property );
  SetColor( red, green, blue );
  SetOpacity( opacity );
  SetName( name );
}

bool mitk::MaterialProperty::Assignable(const BaseProperty& other) const
{
  try
  {
    dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away! Thanks.
    return true;
  }
  catch (std::bad_cast)
  {
  }
  return false;
}

mitk::BaseProperty& mitk::MaterialProperty::operator=(const mitk::BaseProperty& other)
{
  try
  {
    const Self& otherProp( dynamic_cast<const Self&>(other) );

    Initialize(otherProp, false);
  }
  catch (std::bad_cast)
  {
    // nothing to do then
  }

  return *this;
}

void mitk::MaterialProperty::SetDataTreeNode( mitk::DataTreeNode* node )
{
  m_DataTreeNode = node;
}

mitk::DataTreeNode* mitk::MaterialProperty::GetDataTreeNode() const
{
  return m_DataTreeNode;
}

void mitk::MaterialProperty::SetRenderer( mitk::BaseRenderer* renderer )
{
  m_Renderer = renderer;
}

mitk::BaseRenderer* mitk::MaterialProperty::GetRenderer( ) const
{
  return m_Renderer;
}

void mitk::MaterialProperty::SetColor( mitk::MaterialProperty::Color color )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetColor( color.GetRed(), color.GetGreen(), color.GetBlue(), m_Renderer );
  }
  m_Color = color;
  Modified();
}


void mitk::MaterialProperty::SetColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetColor( red, green, blue, m_Renderer );
  }
  m_Color.Set( red, green, blue );
  Modified();
}


void mitk::MaterialProperty::SetColorCoefficient( vtkFloatingPointType coefficient )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( COLOR_COEFFICIENT_KEY, new mitk::FloatProperty( coefficient ), m_Renderer );
  }
  m_ColorCoefficient = coefficient;
  Modified();
}

void mitk::MaterialProperty::SetSpecularColor( mitk::MaterialProperty::Color specularColor )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( SPECULAR_COLOR_KEY, new mitk::ColorProperty( specularColor.GetRed(), specularColor.GetGreen(), specularColor.GetBlue() ), m_Renderer );
  }
  m_SpecularColor = specularColor;
  Modified();
}


void mitk::MaterialProperty::SetSpecularColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( SPECULAR_COLOR_KEY, new mitk::ColorProperty( red, green, blue ), m_Renderer );
  }
  m_SpecularColor.Set( red, green, blue );
  Modified();
}

void mitk::MaterialProperty::SetSpecularCoefficient( vtkFloatingPointType specularCoefficient )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( SPECULAR_COEFFICIENT_KEY, new mitk::FloatProperty( specularCoefficient ), m_Renderer );
  }
  m_SpecularCoefficient = specularCoefficient;
  Modified();
}


void mitk::MaterialProperty::SetSpecularPower( vtkFloatingPointType specularPower )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( SPECULAR_POWER_KEY, new mitk::FloatProperty( specularPower ), m_Renderer );
  }
  m_SpecularPower = specularPower;
  Modified();
}


void mitk::MaterialProperty::SetOpacity( vtkFloatingPointType opacity )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetOpacity( opacity, m_Renderer );
  }
  m_Opacity = opacity;
  Modified();
}


void mitk::MaterialProperty::SetInterpolation( InterpolationType interpolation )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( INTERPOLATION_KEY, new mitk::VtkInterpolationProperty( interpolation ), m_Renderer );
  }
  m_Interpolation = interpolation;
  Modified();
}


void mitk::MaterialProperty::SetRepresentation( RepresentationType representation )
{
  if ( ForwardToDataTreeNode() )
  {
    m_DataTreeNode->SetProperty( REPRESENTATION_KEY, new mitk::VtkRepresentationProperty( representation ), m_Renderer );
  }
  m_Representation = representation;
  Modified();
}


mitk::MaterialProperty::Color mitk::MaterialProperty::GetColor() const
{
  if ( ForwardToDataTreeNode() )
  {
    float rgb[ 3 ];
    m_DataTreeNode->GetColor( rgb, m_Renderer );
    Color color;
    color.SetRed( rgb[ 0 ] );
    color.SetGreen( rgb[ 1 ] );
    color.SetBlue( rgb[ 2 ] );
    return color;
  }
  else
    return m_Color;
}


vtkFloatingPointType mitk::MaterialProperty::GetColorCoefficient() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::FloatProperty * colorCoefficient = dynamic_cast<mitk::FloatProperty*>( m_DataTreeNode->GetProperty( COLOR_COEFFICIENT_KEY, m_Renderer ).GetPointer() );
    if ( colorCoefficient != NULL )
      return colorCoefficient->GetValue();
    else
      return m_ColorCoefficient;
  }
  else
    return m_ColorCoefficient;
}

mitk::MaterialProperty::Color mitk::MaterialProperty::GetSpecularColor() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::ColorProperty * colorProperty = dynamic_cast<mitk::ColorProperty*>( m_DataTreeNode->GetProperty( SPECULAR_COLOR_KEY, m_Renderer ).GetPointer() );
    if ( colorProperty != NULL )
    {
      mitk::Color color = colorProperty->GetColor();
      Color tmpColor;
      tmpColor.Set( color[ 0 ], color[ 1 ], color[ 2 ] );
      return tmpColor;
    }
    else
      return m_SpecularColor;
  }
  else
    return m_SpecularColor;
}

vtkFloatingPointType mitk::MaterialProperty::GetSpecularCoefficient() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::FloatProperty * specularCoefficient = dynamic_cast<mitk::FloatProperty*>( m_DataTreeNode->GetProperty( SPECULAR_COEFFICIENT_KEY, m_Renderer ).GetPointer() );
    if ( specularCoefficient != NULL )
    {
      return specularCoefficient->GetValue();
    }
    else
    {
      return m_SpecularCoefficient;
    }
  }
  else
  {
    return m_SpecularCoefficient;
  }
}


vtkFloatingPointType mitk::MaterialProperty::GetSpecularPower() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::FloatProperty * specularPower = dynamic_cast<mitk::FloatProperty*>( m_DataTreeNode->GetProperty( SPECULAR_POWER_KEY, m_Renderer ).GetPointer() );
    if ( specularPower != NULL )
      return specularPower->GetValue();
    else
      return m_SpecularPower;
  }
  else
    return m_SpecularPower;
}


vtkFloatingPointType mitk::MaterialProperty::GetOpacity() const
{
  if ( ForwardToDataTreeNode() )
  {
    float opacity = 1.0;
    m_DataTreeNode->GetOpacity( opacity, m_Renderer );
    return opacity;
  }
  else
    return m_Opacity;
}

mitk::MaterialProperty::InterpolationType mitk::MaterialProperty::GetInterpolation() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::VtkInterpolationProperty * interpolationProperty = dynamic_cast<mitk::VtkInterpolationProperty*>( m_DataTreeNode->GetProperty( INTERPOLATION_KEY, m_Renderer ).GetPointer() );
    if ( interpolationProperty != NULL )
      return static_cast<mitk::MaterialProperty::InterpolationType>( interpolationProperty->GetValueAsId() );
    else
      return m_Interpolation;
  }
  else
    return m_Interpolation;
}

mitk::MaterialProperty::RepresentationType mitk::MaterialProperty::GetRepresentation() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::VtkRepresentationProperty * representationProperty = dynamic_cast<mitk::VtkRepresentationProperty*>( m_DataTreeNode->GetProperty( REPRESENTATION_KEY, m_Renderer ).GetPointer() );
    if ( representationProperty != NULL )
      return static_cast<mitk::MaterialProperty::RepresentationType>( representationProperty->GetValueAsId() );
    else
      return m_Representation;
  }
  else
    return m_Representation;
}


int mitk::MaterialProperty::GetVtkInterpolation() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::VtkInterpolationProperty * interpolationProperty = dynamic_cast<mitk::VtkInterpolationProperty*>( m_DataTreeNode->GetProperty( INTERPOLATION_KEY, m_Renderer ).GetPointer() );
    if ( interpolationProperty != NULL )
      return interpolationProperty->GetVtkInterpolation();
    else
      return ( int ) m_Interpolation;
  }
  else
  {
    switch ( m_Interpolation )
    {
    case( Flat ) : return VTK_FLAT;
    case( Gouraud ) : return VTK_GOURAUD;
    case( Phong ) : return VTK_PHONG;
    }
    return VTK_GOURAUD;
  }
}

int mitk::MaterialProperty::GetVtkRepresentation() const
{
  if ( ForwardToDataTreeNode() )
  {
    mitk::VtkRepresentationProperty * representationProperty = dynamic_cast<mitk::VtkRepresentationProperty*>( m_DataTreeNode->GetProperty( REPRESENTATION_KEY, m_Renderer ).GetPointer() );
    if ( representationProperty != NULL )
      return representationProperty->GetVtkRepresentation();
    else
      return ( int ) m_Representation;
  }
  else
  {
    switch ( m_Representation )
    {
    case( Points ) : return VTK_POINTS;
    case( Wireframe ) : return VTK_WIREFRAME;
    case( Surface ) : return VTK_SURFACE;
    }
    return VTK_SURFACE;
  }
}



void mitk::MaterialProperty::Initialize( const MaterialProperty& property, const bool& copyDataTreeNode )
{
  if ( copyDataTreeNode )
  {
    this->SetDataTreeNode( property.GetDataTreeNode() );
    this->SetRenderer( property.GetRenderer() );
  }
  this->SetColor( property.GetColor() );
  this->SetColorCoefficient( property.GetColorCoefficient() );
  this->SetSpecularColor( property.GetSpecularColor() );
  this->SetSpecularCoefficient( property.GetSpecularCoefficient() );
  this->SetSpecularPower( property.GetSpecularPower() );
  this->SetOpacity( property.GetOpacity() );
  this->SetInterpolation( property.GetInterpolation() );
  this->SetRepresentation( property.GetRepresentation() );
  this->SetName( property.GetName() );
}


bool mitk::MaterialProperty::operator==( const BaseProperty& property ) const
{
  const Self * other = dynamic_cast<const Self*>( &property );

  if ( other == NULL )
    return false;
  else
    return ( m_DataTreeNode == other->GetDataTreeNode() &&
             m_Renderer == other->GetRenderer() &&
             m_Color == other->GetColor() &&
             m_ColorCoefficient == other->GetColorCoefficient() &&
             m_SpecularColor == other->GetSpecularColor() &&
             m_SpecularCoefficient == other->GetSpecularCoefficient() &&
             m_SpecularPower == other->GetSpecularPower() &&
             m_Opacity == other->GetOpacity() &&
             m_Interpolation == other->GetInterpolation() &&
             m_Name == other->GetName() &&
             m_Representation == other->GetRepresentation()
           );
}



bool mitk::MaterialProperty::ForwardToDataTreeNode() const
{
  return ( m_DataTreeNode != NULL );
}

void mitk::MaterialProperty::InitializeStandardValues()
{
  m_DataTreeNode = NULL;
  m_Renderer = NULL;
  m_Color.Set( 0.5, 0.5, 0.0 );
  m_ColorCoefficient = 0.5 ;
  m_SpecularColor.Set( 1.0, 1.0, 1.0 );
  m_SpecularCoefficient = 0.5;
  m_SpecularPower = 10.0;
  m_Opacity = 1.0 ;
  m_Interpolation = Gouraud;
  m_Representation = Surface;
  m_Name = "";
}

void mitk::MaterialProperty::Update()
{
  this->SetColor( this->GetColor() );
  this->SetColorCoefficient( this->GetColorCoefficient() );
  this->SetSpecularColor( this->GetSpecularColor() );
  this->SetSpecularCoefficient( this->GetSpecularCoefficient() );
  this->SetSpecularPower( this->GetSpecularPower() );
  this->SetOpacity( this->GetOpacity() );
  this->SetInterpolation( this->GetInterpolation() );
  this->SetRepresentation( this->GetRepresentation() );
}

void mitk::MaterialProperty::PrintSelf ( std::ostream &os ) const
{
  os << "Data tree node: " << GetDataTreeNode() << std::endl;
  os << "Renderer: " << GetRenderer() << std::endl;
  os << "Name: " << GetName() << std::endl;
  os << "Color: " << GetColor() << std::endl;
  os << "ColorCoefficient" << GetColorCoefficient() << std::endl;
  os << "SpecularColor: " << GetSpecularColor() << std::endl;
  os << "SpecularCoefficient: " << GetSpecularCoefficient() << std::endl;
  os << "SpecularPower: " << GetSpecularPower() << std::endl;
  os << "Opacity: " << GetOpacity() << std::endl;
  switch ( GetInterpolation() )
  {
  case ( Flat ) : os << "Interpolation: Flat" << std::endl;
    break;
  case ( Gouraud ) : os << "Interpolation: Gouraud" << std::endl;
    break;
  case ( Phong ) : os << "Interpolation: Phong" << std::endl;
    break;
  }
  switch ( GetRepresentation() )
  {
  case ( Points ) : os << "Representation: Points" << std::endl;
    break;
  case ( Wireframe ) : os << "Representation: Wireframe" << std::endl;
    break;
  case ( Surface ) : os << "Representation: Surface" << std::endl;
    break;
  }
}

bool mitk::MaterialProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  this->Update();
  xmlWriter.WriteProperty( COLOR, m_Color );
  xmlWriter.WriteProperty( COLOR_COEFFICIENT, m_ColorCoefficient );
  xmlWriter.WriteProperty( SPECULAR_COLOR, m_SpecularColor );
  xmlWriter.WriteProperty( SPECULAR_COEFFICIENT, m_SpecularCoefficient );
  xmlWriter.WriteProperty( SPECULAR_POWER, m_SpecularPower );
  xmlWriter.WriteProperty( OPACITY, m_Opacity );
  xmlWriter.WriteProperty( INTERPOLATION, m_Interpolation );
  xmlWriter.WriteProperty( REPRESENTATION, m_Representation );
  return true;
}
 
bool mitk::MaterialProperty::ReadXMLData( XMLReader& xmlReader )
{
  float specular, opacity, color_coeff, specular_coeff;
  int interpolation, representation;
  Color color, specular_color;

  if (xmlReader.GetAttribute(COLOR, color)){
    SetColor(color);
  }

  if (xmlReader.GetAttribute(COLOR_COEFFICIENT, color_coeff))
    SetColorCoefficient(color_coeff);

  if (xmlReader.GetAttribute(SPECULAR_COLOR, specular_color))
    SetSpecularColor(specular_color);

  if (xmlReader.GetAttribute(SPECULAR_COEFFICIENT, specular_coeff))
    SetSpecularCoefficient(specular_coeff);

  if (xmlReader.GetAttribute(SPECULAR_POWER, specular))
    SetSpecularPower(specular);

  if(xmlReader.GetAttribute(OPACITY, opacity))
    SetOpacity(opacity);

  if(xmlReader.GetAttribute(INTERPOLATION, interpolation))
    SetInterpolation((InterpolationType)interpolation);

  if(xmlReader.GetAttribute(REPRESENTATION, representation))
    SetRepresentation((RepresentationType)representation);

  return true;
}

const char* mitk::MaterialProperty::COLOR_KEY = "color";
const char* mitk::MaterialProperty::SPECULAR_COLOR_KEY = "specular_color";
const char* mitk::MaterialProperty::COLOR_COEFFICIENT_KEY = "color_coefficient";
const char* mitk::MaterialProperty::SPECULAR_COEFFICIENT_KEY = "specular_coefficient";
const char* mitk::MaterialProperty::SPECULAR_POWER_KEY = "specular_power";
const char* mitk::MaterialProperty::OPACITY_KEY = "opacity";
const char* mitk::MaterialProperty::INTERPOLATION_KEY = "interpolation";
const char* mitk::MaterialProperty::REPRESENTATION_KEY = "representation";

const char* mitk::MaterialProperty::COLOR = "COLOR";
const char* mitk::MaterialProperty::SPECULAR_COLOR = "SPECULAR_COLOR";
const char* mitk::MaterialProperty::COLOR_COEFFICIENT = "COLOR_COEFFICIENT";
const char* mitk::MaterialProperty::SPECULAR_COEFFICIENT = "SPECULAR_COEFFICIENT";
const char* mitk::MaterialProperty::SPECULAR_POWER = "SPECULAR_POWER";
const char* mitk::MaterialProperty::OPACITY = "OPACITY";
const char* mitk::MaterialProperty::INTERPOLATION = "INTERPOLATION";
const char* mitk::MaterialProperty::REPRESENTATION = "REPRESENTATION";
