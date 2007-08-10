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


#ifndef MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491
#define MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <string>

namespace mitk {

//##ModelId=3E3FDF05028B
//##Documentation
//## @brief Property for strings
//## @ingroup DataTree
class StringProperty : public BaseProperty
{
protected:
    //##ModelId=3E3FDF21017D
    std::string m_Value;

public:
    mitkClassMacro(StringProperty, BaseProperty);
    typedef std::string ValueType;
    //##ModelId=3E3FF04F005F
    StringProperty( const char* string = NULL );
    StringProperty( std::string  s );
    
    itkGetStringMacro(Value);
    itkSetStringMacro(Value);

    //##ModelId=3E3FF04F00E1
    virtual bool operator==(const BaseProperty& property ) const;
    virtual std::string GetValueAsString() const;
  //## 
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );

  static const char* PATH;
};

} // namespace mitk

#endif /* MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491 */
