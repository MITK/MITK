#include "mitkStringProperty.h"

//##ModelId=3E3FF04F005F
mitk::StringProperty::StringProperty( const char* string ) 
: m_String( string ) {

}

//##ModelId=3E3FF04F00E1
bool mitk::StringProperty::operator==(const BaseProperty& property ) const {
	
	// auchtung Type-Prüfung durchführen !!!!!
	if ( ((mitk::StringProperty&)property).m_String == m_String )
		return true;

	return false;
}