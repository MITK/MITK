/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCOMMANDPARAMETER_H
#define BERRYCOMMANDPARAMETER_H

#include "berryIParameter.h"
#include "berryITypedParameter.h"

namespace berry {

struct IConfigurationElement;

/**
 * <p>
 * A parameter for a command. A parameter identifies a type of information that
 * the command might accept. For example, a "Show View" command might accept the
 * id of a view for display. This parameter also identifies possible values, for
 * display in the user interface.
 * </p>
 * <p>
 * Parameters are mutable, and can change as the command changes. Notifications
 * will not be sent if the parameter itself changes. Listeners can be attached
 * to the command.
 * </p>
 */
class CommandParameter : public IParameter, public ITypedParameter
{

public:

  berryObjectMacro(berry::CommandParameter);

private:

  /**
   * The name of the configuration element attribute contain the values. This
   * is used to retrieve the executable extension
   * <code>IParameterValues</code>.
   */
  static const QString ATTRIBUTE_VALUES; // = "values";

  /**
   * The constant integer hash code value meaning the hash code has not yet
   * been computed.
   */
  static const int HASH_CODE_NOT_COMPUTED; // = 0;

  /**
   * A factor for computing the hash code for all schemes.
   */
  static const int HASH_FACTOR; // = 89;

  /**
   * The seed for the hash code for all schemes.
   */
  static const int HASH_INITIAL;

  /**
   * The hash code for this object. This value is computed lazily, and marked
   * as invalid when one of the values on which it is based changes.
   */
  mutable int hashCode; // = HASH_CODE_NOT_COMPUTED;

  /**
   * The non-externalized name of this parameter. The name is used as the in a
   * name-value parameter map. This value will never be <code>null</code>.
   */
  const QString name;

  /**
   * Whether the parameter is optional (as opposed to required).
   */
  const bool optional;

  /**
   * The type for this parameter. This value may be <code>null</code> if the
   * parameter is not typed.
   */
  const SmartPointer<ParameterType> parameterType;

  /**
   * The actual <code>IParameterValues</code> implementation. This is lazily
   * loaded from the <code>valuesConfigurationElement</code>, to avoid
   * unnecessary class-loading.
   */
  mutable QScopedPointer<IParameterValues> values;

  /**
   * The configuration element providing the executable extension that will
   * implement <code>IParameterValues</code>. This value will not be
   * <code>null</code>.
   */
  const SmartPointer<IConfigurationElement> valuesConfigurationElement;

protected:

  /**
   * The identifier for this object. This identifier should be unique across
   * all objects of the same type and should never change. This value will
   * never be <code>null</code>.
   */
  const QString id;

  /**
   * The string representation of this object. This string is for debugging
   * purposes only, and is not meant to be displayed to the user. This value
   * is computed lazily, and is cleared if one of its dependent values
   * changes.
   */
  mutable QString str;

public:

  /**
   * Constructs a new instance of <code>Parameter</code> with all of its
   * values pre-defined.
   *
   * @param id
   *            The identifier for this parameter; must not be
   *            <code>null</code>.
   * @param name
   *            The name for this parameter; must not be <code>null</code>.
   * @param values
   *            The values for this parameter; must not be <code>null</code>.
   * @param parameterType
   *            the type for this parameter; may be <code>null</code> if the
   *            parmeter doesn't declare type.
   * @param optional
   *            Whether this parameter is optional (as opposed to required).
   * @param commandService
   *            The command service from which parameter types can be
   *            retrieved; must not be <code>null</code>.
   */
  CommandParameter(const QString& id, const QString& name,
                   const SmartPointer<IConfigurationElement>& values,
                   const SmartPointer<ParameterType>& parameterType,
                   const bool optional);

  /**
   * Tests whether this object is equal to another object. A parameter is only
   * equal to another parameter with the same properties.
   *
   * @param object
   *            The object with which to compare; may be <code>null</code>.
   * @return <code>true</code> if the objects are equal; <code>false</code>
   *         otherwise.
   */
  bool operator==(const Object* object) const override;

  QString GetId() const override;

  QString GetName() const override;

  SmartPointer<ParameterType> GetParameterType() const override;

  IParameterValues* GetValues() const override;

  uint HashCode() const override
  {
    if (hashCode == HASH_CODE_NOT_COMPUTED)
    {
      hashCode = HASH_INITIAL * HASH_FACTOR + qHash(id);
      if (hashCode == HASH_CODE_NOT_COMPUTED)
      {
        hashCode++;
      }
    }
    return hashCode;
  }

  bool IsOptional() const override;

  QString ToString() const override;
};

}
#endif // BERRYCOMMANDPARAMETER_H
