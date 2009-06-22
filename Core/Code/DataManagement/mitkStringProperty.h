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

/**
 * @brief Property for strings
 * @ingroup DataManagement
 */
  class MITK_CORE_EXPORT StringProperty : public BaseProperty
  {
    protected:
      std::string m_Value;

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

      virtual bool operator==(const BaseProperty& property ) const;
      virtual std::string GetValueAsString() const;

      bool Assignable(const BaseProperty& other) const;
      virtual BaseProperty& operator=(const BaseProperty& other);
      
      static const char* PATH;
  };

} // namespace mitk

#endif 

