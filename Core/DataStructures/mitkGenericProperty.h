/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>
#include <stdlib.h>


#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

namespace mitk {

//##ModelId=3EF99DB6009C
template <typename T>
class GenericProperty : public BaseProperty
{
  public:
   
    mitkClassMacro(GenericProperty, BaseProperty);
//    itkNewMacro(Self);
    //##ModelId=3EF99E2C0290
    GenericProperty() {};
    GenericProperty(T x) : m_Value(x) {};	
    //##ModelId=3EF99E2C02AE
    virtual ~GenericProperty() {}; 
    //##ModelId=3EF99E3A0196
    virtual bool operator==(const BaseProperty& property) const {
        const Self *other = dynamic_cast<const Self*>(&property);
        if(other==NULL) return false;
        return other->m_Value==m_Value;
    }
    
    itkSetMacro(Value,T);
    itkGetConstMacro(Value,T);
    
   	
    std::string GetValueAsString() const {
       std::stringstream myStr;
       myStr << GetValue() ;
       return myStr.str(); 
    }

  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  
  virtual bool ReadXMLData( XMLReader& xmlReader );
    
  protected:
    //##ModelId=3EF99E45001F
    T m_Value;

};

template <class T>
bool GenericProperty<T>::WriteXMLData( XMLWriter& xmlWriter )
{
  xmlWriter.WriteProperty( VALUE, m_Value );  
  return true;
}

template <class T>
bool GenericProperty<T>::ReadXMLData( XMLReader& xmlReader )
{
  T value;
  xmlReader.GetAttribute( VALUE, value );
  m_Value = (T) value;
  return true;
}


} // namespace mitk

#endif /* MITKGENERICPROPERTY_H_HEADER_INCLUDED_C1061CEE */
