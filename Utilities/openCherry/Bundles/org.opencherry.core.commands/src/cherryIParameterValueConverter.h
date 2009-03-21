/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYABSTRACTPARAMETERVALUECONVERTER_H_
#define CHERRYABSTRACTPARAMETERVALUECONVERTER_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "common/cherryCommandExceptions.h"
#include "cherryCommandsDll.h"

namespace cherry {

/**
 * <p>
 * Supports conversion between objects and strings for command parameter values.
 * Extenders must produce strings that identify objects (of a specific command
 * parameter type) as well as consume the strings to locate and return the
 * objects they identify.
 * </p>
 * <p>
 * This class offers multiple handlers of a command a consistent way of
 * converting string parameter values into the objects that the handlers would
 * prefer to deal with. This class also gives clients a way to serialize
 * object parameters as strings so that entire parameterized commands can be
 * serialized, stored and later deserialized and executed.
 * </p>
 * <p>
 * This class will typically be extended so the subclass can be referenced from
 * the <code>converter</code> attribute of the
 * <code>commandParameterType</code> elemement of the
 * <code>org.eclipse.ui.commands</code> extension-point. Objects implementing
 * this interface may also be passed directly to
 * {@link ParameterType#Define(IParameterValueConverter::Pointer)} by
 * clients.
 * </p>
 *
 * @see ParameterType#Define(IParameterValueConverter::Pointer)
 * @see ParameterizedCommand#Serialize()
 */
struct CHERRY_COMMANDS IParameterValueConverter : public virtual Object {

  cherryInterfaceMacro(IParameterValueConverter, cherry)

  /**
   * Returns whether the provided value is compatible with this parameter
   * value converter. An object is compatible with a converter if the object is an
   * instance of the class defined in the <code>type</code> attribute of
   * the <code>commandParameterType</code> element.
   *
   * @param value
   *            an object to check for compatibility with this parameter type;
   *            may be <code>null</code>.
   * @return <code>true</code> if the value is compatible with this converter,
   *         <code>false</code> otherwise
   */
  virtual bool IsCompatible(const Object::ConstPointer value) const = 0;

  /**
   * Converts a string encoded command parameter value into the parameter
   * value object.
   *
   * @param parameterValue
   *            a command parameter value string describing an object; may be
   *            <code>null</code>
   * @return the object described by the command parameter value string; may
   *         be <code>null</code>
   * @throws ParameterValueConversionException
   *             if an object cannot be produced from the
   *             <code>parameterValue</code> string
   */
  virtual Object::Pointer ConvertToObject(const std::string& parameterValue)
      throw(ParameterValueConversionException) = 0;

  /**
   * Converts a command parameter value object into a string that encodes a
   * reference to the object or serialization of the object.
   *
   * @param parameterValue
   *            an object to convert into an identifying string; may be
   *            <code>null</code>
   * @return a string describing the provided object; may be <code>null</code>
   * @throws ParameterValueConversionException
   *             if a string reference or serialization cannot be provided for
   *             the <code>parameterValue</code>
   */
  virtual std::string ConvertToString(const Object::Pointer parameterValue)
      throw(ParameterValueConversionException) = 0;

};

}

#endif /* CHERRYABSTRACTPARAMETERVALUECONVERTER_H_ */
