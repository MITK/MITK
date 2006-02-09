#include "mitkEnumerationProperty.h"
#include <algorithm>
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

mitk::EnumerationProperty::EnumerationProperty()
{
  m_CurrentValue = 0;
}





bool mitk::EnumerationProperty::AddEnum( const std::string& name, const IdType& id )
{
  if ( ( ! IsValidEnumerationValue( name ) ) && ( ! IsValidEnumerationValue( id ) ) )
  {
    m_EnumIds.insert( std::make_pair( id, name ) );
    m_EnumStrings.insert( std::make_pair( name, id ) );
    return true;
  }
  else
  {
    return false;
  }
}





bool mitk::EnumerationProperty::SetValue( const std::string& name )
{
  if ( IsValidEnumerationValue( name ) )
  {
    m_CurrentValue = GetEnumId( name );
    return true;
  }
  else
  {
    return false;
  }
}





bool mitk::EnumerationProperty::SetValue( const IdType& id )
{
  if ( IsValidEnumerationValue( id ) )
  {
    m_CurrentValue = id;
    return true;
  }
  else
  {
    return false;
  }
}





mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetValueAsId() const
{
  return m_CurrentValue;
}





std::string mitk::EnumerationProperty::GetValueAsString() const
{
  return GetEnumString( m_CurrentValue );
}





void mitk::EnumerationProperty::Clear()
{
  m_EnumIds.clear();
  m_EnumStrings.clear();
  m_CurrentValue = 0;
}





unsigned int mitk::EnumerationProperty::Size() const
{
  return m_EnumIds.size();
}






mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::Begin() const
{
  return m_EnumIds.begin();
}





mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::End() const
{
  return m_EnumIds.end();
}





std::string mitk::EnumerationProperty::GetEnumString( const IdType& id ) const
{
  if ( IsValidEnumerationValue( id ) )
  {
    return m_EnumIds.find( id )->second;
  }
  else
  {
    return "invalid enum id or enums empty";
  }
}





mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetEnumId( const std::string& name ) const
{
  if ( IsValidEnumerationValue( name ) )
  {
    return  m_EnumStrings.find( name )->second;
  }
  else
  {
    return 0;
  }
}





bool mitk::EnumerationProperty::operator==( const BaseProperty& property ) const
{
  const Self * other = dynamic_cast<const Self*>( &property );

  if ( other == NULL )
    return false;

  if ( this->Size() != other->Size() )
    return false;

  if ( this->GetValueAsId() != other->GetValueAsId() )
    return false;

  return std::equal( this->Begin(), this->End(), other->Begin() );
}





bool mitk::EnumerationProperty::IsValidEnumerationValue( const IdType& val ) const
{
  return ( m_EnumIds.find( val ) != m_EnumIds.end() );
}





bool mitk::EnumerationProperty::IsValidEnumerationValue( const std::string& val ) const
{
  return ( m_EnumStrings.find( val ) != m_EnumStrings.end() );
}


bool mitk::EnumerationProperty::WriteXMLData( XMLWriter& xmlWriter )
{
  xmlWriter.WriteProperty( VALUE, GetValueAsString() );
  return true;
}


bool mitk::EnumerationProperty::ReadXMLData( XMLReader& xmlReader )
{
  std::string m_EnumValueString;
  xmlReader.GetAttribute( VALUE, m_EnumValueString );
  SetValue(m_EnumValueString);
  std::cout << "EnumerationProperty: " << GetValueAsString() << std::endl;
  return true;
}
