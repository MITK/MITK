/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
