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


#ifndef MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491
#define MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491

#include <itkConfigure.h>
#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include <string>

namespace mitk {

//##ModelId=3E3FDF05028B
//##Documentation
//## @brief Property for strings
//## @ingroup DataTree
class MITK_CORE_EXPORT StringProperty : public BaseProperty
{
protected:
    //##ModelId=3E3FDF21017D
    std::string m_Value;
    
    //##ModelId=3E3FF04F005F
    StringProperty( const char* string = 0 );
    StringProperty( const std::string&  s );

public:
    mitkClassMacro(StringProperty, BaseProperty);
    typedef std::string ValueType;
    
    itkNewMacro(StringProperty);
    mitkNewMacro1Param(StringProperty, const char*);
    mitkNewMacro1Param(StringProperty, const std::string&)
    
    itkGetStringMacro(Value);
    itkSetStringMacro(Value);

    /// @todo: remove after ITK 2.8 compatibility has been dropped 
#if ITK_VERSION_MAJOR < 3     
    virtual void SetValue (const std::string & _arg) 
    { 
      this->SetValue( _arg.c_str() ); 
    } 


#endif

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
