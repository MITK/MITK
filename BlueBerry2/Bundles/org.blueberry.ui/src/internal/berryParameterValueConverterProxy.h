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


#ifndef BERRYPARAMETERVALUECONVERTERPROXY_H_
#define BERRYPARAMETERVALUECONVERTERPROXY_H_

#include <berryIParameterValueConverter.h>
#include <berryIConfigurationElement.h>

namespace berry {

/**
 * A proxy for a parameter value converter that has been defined in the regisry.
 * This delays the class loading until the converter is really asked to do
 * string/object conversions.
 *
 * @since 3.2
 */
class ParameterValueConverterProxy : public IParameterValueConverter {

private:

  /**
   * The configuration element providing the executable extension that will
   * extend <code>AbstractParameterValueConverter</code>. This value will
   * not be <code>null</code>.
   */
  const IConfigurationElement::Pointer converterConfigurationElement;

  /**
   * The real parameter value converter instance. This will be
   * <code>null</code> until one of the conversion methods are used.
   */
  mutable IParameterValueConverter::Pointer parameterValueConverter;

  /**
   * Returns the real parameter value converter for this proxy or throws an
   * exception indicating the converter could not be obtained.
   *
   * @return the real converter for this proxy; never <code>null</code>.
   * @throws ParameterValueConversionException
   *             if the converter could not be obtained
   */
  IParameterValueConverter::Pointer GetConverter() const throw(ParameterValueConversionException);


public:

  /**
   * Constructs a <code>ParameterValueConverterProxy</code> to represent the
   * real converter until it is needed.
   *
   * @param converterConfigurationElement
   *            The configuration element from which the real converter can be
   *            loaded.
   */
  ParameterValueConverterProxy(const IConfigurationElement::Pointer converterConfigurationElement);

  bool IsCompatible(const Object::ConstPointer value) const;

  Object::Pointer ConvertToObject(const std::string& parameterValue)
      throw(ParameterValueConversionException);

  std::string ConvertToString(const Object::Pointer parameterValue)
      throw(ParameterValueConversionException);

};

}

#endif /* BERRYPARAMETERVALUECONVERTERPROXY_H_ */
