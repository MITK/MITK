/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPARAMETERTYPE_H_
#define BERRYPARAMETERTYPE_H_

#include "common/berryHandleObject.h"
#include "berryIParameterTypeListener.h"
#include "berryIParameterValueConverter.h"

#include <QSharedPointer>

namespace berry
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
class BERRY_COMMANDS ParameterType: public HandleObject
{ //implements Comparable {

public:

  berryObjectMacro(ParameterType);

  /**
   * Adds a listener to this parameter type that will be notified when its
   * state changes.
   *
   * @param listener
   *            The listener to be added; must not be <code>null</code>.
   */
void  AddListener(IParameterTypeListener* listener);

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
  bool operator<(const Object* object) const override;

  /**
   * <p>
   * Defines this parameter type, setting the defined property to
   * <code>true</code>.
   * </p>
   * <p>
   * Notification is sent to all listeners that something has changed.
   * </p>
   *
   * @param type
   *            a string identifying the object type for this parameter
   *            type; <code>null</code> is interpreted as
   *            <code>"QObject"</code>
   * @param parameterTypeConverter
   *            an {@link AbstractParameterValueConverter} to perform
   *            string/object conversions for parameter values; may be
   *            <code>null</code>
   */
  void Define(const QString& type,
              const QSharedPointer<IParameterValueConverter>& parameterTypeConverter);

  /**
   * Returns the value converter associated with this parameter, if any.
   *
   * @return The parameter value converter, or <code>null</code> if there is
   *         no value converter for this parameter.
   * @throws NotDefinedException
   *             if the parameter type is not currently defined
   */
  IParameterValueConverter* GetValueConverter() const;

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
  bool IsCompatible(const QObject* const value) const;

  /**
   * Unregisters listener for changes to properties of this parameter type.
   *
   * @param listener
   *            the instance to unregister. Must not be <code>null</code>.
   *            If an attempt is made to unregister an instance which is not
   *            already registered with this instance, no operation is
   *            performed.
   */
  void RemoveListener(IParameterTypeListener* listener);

  /**
   * The string representation of this parameter type. For debugging purposes
   * only. This string should not be shown to an end user.
   *
   * @return The string representation; never <code>null</code>.
   */
  QString ToString() const override;

  /**
   * Makes this parameter type become undefined. Notification is sent to all
   * listeners.
   */
  void Undefine() override;

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
  ParameterType(const QString& id);

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
  QSharedPointer<IParameterValueConverter> parameterTypeConverter;

  /**
   * A string specifying the object type of this parameter type. This will be
   * <code>null</code> when the parameter type is undefined but never null
   * when it is defined.
   */
  QString type;

  IParameterTypeListener::Events parameterTypeEvents;

};

}

#endif /* BERRYPARAMETERTYPE_H_ */
