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

#ifndef BERRYPARAMETERVALUECONVERTERPROXY_H
#define BERRYPARAMETERVALUECONVERTERPROXY_H

#include "berryIParameterValueConverter.h"

#include <QSharedPointer>

namespace berry {

struct IConfigurationElement;

/**
 * A proxy for a parameter value converter that has been defined in the regisry.
 * This delays the class loading until the converter is really asked to do
 * string/object conversions.
 */
class ParameterValueConverterProxy : public IParameterValueConverter
{

private:

  /**
   * The configuration element providing the executable extension that will
   * extend <code>AbstractParameterValueConverter</code>. This value will
   * not be <code>null</code>.
   */
  const SmartPointer<IConfigurationElement> converterConfigurationElement;

  /**
   * The real parameter value converter instance. This will be
   * <code>null</code> until one of the conversion methods are used.
   */
  mutable QSharedPointer<IParameterValueConverter> parameterValueConverter;

  /**
   * Returns the real parameter value converter for this proxy or throws an
   * exception indicating the converter could not be obtained.
   *
   * @return the real converter for this proxy; never <code>null</code>.
   * @throws ParameterValueConversionException
   *             if the converter could not be obtained
   */
  IParameterValueConverter* GetConverter() const;

public:

  /**
   * Constructs a <code>ParameterValueConverterProxy</code> to represent the
   * real converter until it is needed.
   *
   * @param converterConfigurationElement
   *            The configuration element from which the real converter can be
   *            loaded.
   */
  ParameterValueConverterProxy(
      const SmartPointer<IConfigurationElement>& converterConfigurationElement);

  Object::Pointer ConvertToObject(const QString& parameterValue);

  QString ConvertToString(const Object::Pointer& parameterValue);

};

}

#endif // BERRYPARAMETERVALUECONVERTERPROXY_H
