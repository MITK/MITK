#include <vtkProperty.h>
#include "mitkVtkRepresentationProperty.h"


mitk::VtkRepresentationProperty::VtkRepresentationProperty( )
{
  AddRepresentationTypes();
  SetValue( static_cast<IdType>( VTK_SURFACE ) );
}


mitk::VtkRepresentationProperty::VtkRepresentationProperty( const IdType& value )
{
  AddRepresentationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( static_cast<IdType>( VTK_SURFACE ) );
  }
}


mitk::VtkRepresentationProperty::VtkRepresentationProperty( const std::string& value )
{
  AddRepresentationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( static_cast<IdType>( VTK_SURFACE ) );
  }
}


int mitk::VtkRepresentationProperty::GetVtkRepresentation()
{
  return static_cast<int>( GetValueAsId() );
}


void mitk::VtkRepresentationProperty::SetRepresentationToPoints()
{
  SetValue( static_cast<IdType>( VTK_POINTS ) );
}


void mitk::VtkRepresentationProperty::SetRepresentationToWireframe()
{
  SetValue( static_cast<IdType>( VTK_WIREFRAME ) );
}


void mitk::VtkRepresentationProperty::SetRepresentationToSurface()
{
  SetValue( static_cast<IdType>( VTK_SURFACE ) );
}

void mitk::VtkRepresentationProperty::AddRepresentationTypes()
{
  AddEnum( "Points", static_cast<IdType>( VTK_POINTS ) );
  AddEnum( "Wireframe", static_cast<IdType>( VTK_WIREFRAME ) );
  AddEnum( "Surface", static_cast<IdType>( VTK_SURFACE ) );
}


bool mitk::VtkRepresentationProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );  
}
