/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryParameterValueConverterProxy.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryCommandExceptions.h"
#include "berryIConfigurationElement.h"
#include "berryCoreException.h"

namespace berry {

IParameterValueConverter* ParameterValueConverterProxy::GetConverter() const
{
  if (parameterValueConverter.isNull())
  {
    try
    {
      parameterValueConverter = QSharedPointer<IParameterValueConverter>(
            converterConfigurationElement->CreateExecutableExtension<IParameterValueConverter>(WorkbenchRegistryConstants::ATT_CONVERTER)
            );
    }
    catch (const CoreException& e)
    {
      throw ParameterValueConversionException(
          "Problem creating parameter value converter", e);
    }

    if (parameterValueConverter.isNull())
    {
      throw ParameterValueConversionException(
          "Parameter value converter was not a subclass of AbstractParameterValueConverter");
    }
  }
  return parameterValueConverter.data();
}

ParameterValueConverterProxy::ParameterValueConverterProxy(
    const SmartPointer<IConfigurationElement>& converterConfigurationElement)
  : converterConfigurationElement(converterConfigurationElement)
{
  if (converterConfigurationElement.IsNull())
  {
    throw ctkInvalidArgumentException(
        "converterConfigurationElement must not be null");
  }
}

Object::Pointer ParameterValueConverterProxy::ConvertToObject(const QString& parameterValue)
{
  return GetConverter()->ConvertToObject(parameterValue);
}

QString ParameterValueConverterProxy::ConvertToString(const Object::Pointer& parameterValue)
{
  return GetConverter()->ConvertToString(parameterValue);
}

}
