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

#include <MitkExports.h>
#include "mitkBaseProperty.h"

#include <string>

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

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

      virtual std::string GetValueAsString() const;
      
      static const char* PATH;

      using BaseProperty::operator=;

    private:
      // purposely not implemented
      StringProperty(const StringProperty&);
      StringProperty& operator=(const StringProperty&);

      virtual bool IsEqual(const BaseProperty& property ) const;
      virtual bool Assign(const BaseProperty& property );
  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif 

