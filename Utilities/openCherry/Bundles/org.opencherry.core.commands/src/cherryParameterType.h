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

#ifndef CHERRYPARAMETERTYPE_H_
#define CHERRYPARAMETERTYPE_H_

#include "common/cherryHandleObject.h"
#include "cherryIParameterTypeListener.h"
#include "cherryIParameterValueConverter.h"

namespace cherry
{

struct IParameterValueConverter;

/**
 * <p>
 * Provides information about the type of a command parameter. Clients can use a
 * parameter type to check if an object matches the type of the parameter with
 * {@link #IsCompatible(Object::Pointer)} and can get an
 * {@link IParameterValueConverter} to convert between objects matching
 * the parameter type and strings that encode the object's identity.
 * </p>
 * <p>
 * A command parameter is not required to declare a type. To determine if a
 * given parameter has a type, check if an {@link IParameter} implements
 * {@link ITypedParameter} and if so, use
 * {@link ITypedParameter#GetParameterType()} like this:
 * </p>
 *
 * <pre>
 *                   IParameter::Pointer parameter = // ... get IParameter from Command
 *                   if (ITypedParameter::Pointer typedParameter = parameter.Cast<ITypedParameter>())
 *                   {
 *                     ParameterType::Pointer type = typedParameter->GetParameterType();
 *                     if (type) {
 *                       // this parameter has a ParameterType
 *                     }
 *                   }
 * </pre>
 *
 * @see IParameter
 * @see ITypedParameter#GetParameterType()
 */
class CHERRY_COMMANDS ParameterType: public HandleObject
{ //implements Comparable {

public:

  cherryObjectMacro(ParameterType)

  /**
   * Adds a listener to this parameter type that will be notified when its
   * state changes.
   *
   * @param listener
   *            The listener to be added; must not be <code>null</code>.
   */
void  AddListener(const IParameterTypeListener::Pointer listener);

  /**
   * Compares this parameter type with another object by comparing each of the
   * non-transient attributes.
   *
   * @param object
   *            The object with which to compare; must be an instance of
   *            {@link ParameterType}.
   * @return A negative integer, zero or a positive integer, if the object is
   *         greater than, equal to or less than this parameter type.
   */
  bool operator<(const Object* object) const;

  /**
   * <p>
   * Defines this parameter type, setting the defined property to
   * <code>true</code>.
   * </p>
   * <p>
   * Notification is sent to all listeners that something has changed.
   * </p>
   *
   * @param parameterTypeConverter
   *            an {@link AbstractParameterValueConverter} to perform
   *            string/object conversions for parameter values; may be
   *            <code>null</code>
   */
  void Define(const SmartPointer<IParameterValueConverter> parameterTypeConverter);

  /**
   * Returns the value converter associated with this parameter, if any.
   *
   * @return The parameter value converter, or <code>null</code> if there is
   *         no value converter for this parameter.
   * @throws NotDefinedException
   *             if the parameter type is not currently defined
   */
  SmartPointer<IParameterValueConverter> GetValueConverter() const;

  /**
   * Returns whether the provided value is compatible with this parameter
   * type. An object is compatible with a parameter type if the object is an
   * instance of the class defined as the parameter's type class.
   *
   * @param value
   *            an object to check for compatibility with this parameter type;
   *            may be <code>null</code>.
   * @return <code>true</code> if the value is compatible with this type,
   *         <code>false</code> otherwise
   * @throws NotDefinedException
   *             if the parameter type is not currently defined
   */
  bool IsCompatible(const Object::ConstPointer value) const;

  /**
   * Unregisters listener for changes to properties of this parameter type.
   *
   * @param listener
   *            the instance to unregister. Must not be <code>null</code>.
   *            If an attempt is made to unregister an instance which is not
   *            already registered with this instance, no operation is
   *            performed.
   */
  void RemoveListener(const IParameterTypeListener::Pointer listener);

  /**
   * The string representation of this parameter type. For debugging purposes
   * only. This string should not be shown to an end user.
   *
   * @return The string representation; never <code>null</code>.
   */
  std::string ToString() const;

  /**
   * Makes this parameter type become undefined. Notification is sent to all
   * listeners.
   */
  void Undefine();

protected:

  friend class CommandManager;

  /**
   * Constructs a new instance based on the given identifier. When a parameter
   * type is first constructed, it is undefined. Parameter types should only
   * be constructed by the {@link CommandManager} to ensure that the
   * identifier remains unique.
   *
   * @param id
   *            The identifier for this type. This value must not be
   *            <code>null</code>, and must be unique amongst all parameter
   *            types.
   */
  ParameterType(const std::string& id);

private:

  /**
   * Notifies all listeners that this parameter type has changed. This sends
   * the given event to all of the listeners, if any.
   *
   * @param event
   *            The event to send to the listeners; must not be
   *            <code>null</code>.
   */
  void FireParameterTypeChanged(const SmartPointer<ParameterTypeEvent> event);

  /**
   * An {@link AbstractParameterValueConverter} for converting parameter
   * values between objects and strings. This may be <code>null</code>.
   */
  SmartPointer<IParameterValueConverter> parameterTypeConverter;

  IParameterTypeListener::Events parameterTypeEvents;

};

}

#endif /* CHERRYPARAMETERTYPE_H_ */
