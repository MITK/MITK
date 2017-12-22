/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491
#define MITKSTRINGPROPERTY_H_HEADER_INCLUDED_C1C02491

#include <itkConfigure.h>

#include "mitkBaseProperty.h"
#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * @brief Property for strings
   * @ingroup DataManagement
   */
  class MITKCORE_EXPORT StringProperty : public BaseProperty
  {
  protected:
    std::string m_Value;

    StringProperty(const char *string = nullptr);
    StringProperty(const std::string &s);

    StringProperty(const StringProperty &);

  public:
    mitkClassMacro(StringProperty, BaseProperty);
    typedef std::string ValueType;

    itkFactorylessNewMacro(Self) itkCloneMacro(Self) mitkNewMacro1Param(StringProperty, const char *);
    mitkNewMacro1Param(StringProperty, const std::string &)

      itkGetStringMacro(Value);
    itkSetStringMacro(Value);

    std::string GetValueAsString() const override;

    static const char *PATH;

    using BaseProperty::operator=;

  private:
    // purposely not implemented
    StringProperty &operator=(const StringProperty &);

    itk::LightObject::Pointer InternalClone() const override;

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
