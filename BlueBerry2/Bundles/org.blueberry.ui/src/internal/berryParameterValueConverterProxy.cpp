/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryParameterValueConverterProxy.h"

#include "berryWorkbenchRegistryConstants.h"

namespace berry {

IParameterValueConverter::Pointer ParameterValueConverterProxy::GetConverter() const
      throw(ParameterValueConversionException) {
    if (!parameterValueConverter) {
      try {
        parameterValueConverter = converterConfigurationElement
            ->CreateExecutableExtension<IParameterValueConverter>(WorkbenchRegistryConstants::ATT_CONVERTER);
      } catch (const CoreException& e) {
        throw ParameterValueConversionException(
            "Problem creating parameter value converter", e);
      }

      if (!parameterValueConverter) {
        throw ParameterValueConversionException(
            "Parameter value converter was not a subclass of AbstractParameterValueConverter");
      }
    }
    return parameterValueConverter;
  }

  ParameterValueConverterProxy::ParameterValueConverterProxy(const IConfigurationElement::Pointer converterConfigurationElement)
  : converterConfigurationElement(converterConfigurationElement)
  {
    if (!converterConfigurationElement) {
      throw Poco::NullPointerException(
          "converterConfigurationElement must not be null");
    }
  }

  bool ParameterValueConverterProxy::IsCompatible(const Object::ConstPointer value) const
  {
    return this->GetConverter()->IsCompatible(value);
  }

  Object::Pointer ParameterValueConverterProxy::ConvertToObject(const std::string& parameterValue)
      throw(ParameterValueConversionException) {
    return this->GetConverter()->ConvertToObject(parameterValue);
  }

  std::string ParameterValueConverterProxy::ConvertToString(const Object::Pointer parameterValue)
      throw(ParameterValueConversionException) {
    return this->GetConverter()->ConvertToString(parameterValue);
  }

}
