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


#ifndef CHERRYPARAMETERIZATION_H_
#define CHERRYPARAMETERIZATION_H_

#include "cherryCommandsDll.h"
#include "common/cherryCommandExceptions.h"

#include <string>

#include <cherrySmartPointer.h>

namespace cherry {

struct IParameter;

/**
 * <p>
 * A parameter with a specific value. This is usually a part of a
 * <code>ParameterizedCommand</code>, which is used to refer to a command
 * with a collection of parameterizations.
 * </p>
 *
 * @since 3.1
 */
class CHERRY_COMMANDS Parameterization {

private:

  /**
   * The constant integer hash code value meaning the hash code has not yet
   * been computed.
   */
  static const std::size_t HASH_CODE_NOT_COMPUTED; // = 0;

  /**
   * A factor for computing the hash code for all parameterized commands.
   */
  static const std::size_t HASH_FACTOR; // = 89;

  /**
   * The seed for the hash code for all parameterized commands.
   */
  static const std::size_t HASH_INITIAL;

  /**
   * The hash code for this object. This value is computed lazily, and marked
   * as invalid when one of the values on which it is based changes.
   */
  mutable std::size_t hashCode;

  /**
   * The parameter that is being parameterized. This value is never
   * <code>null</code>.
   */
  SmartPointer<const IParameter> parameter;

  /**
   * The value that defines the parameterization. This value may be
   * <code>null</code>.
   */
  std::string value;


public:

  /**
   * Constructs a new instance of <code>Parameterization</code>.
   *
   * @param parameter
   *            The parameter that is being parameterized; must not be
   *            <code>null</code>.
   * @param value
   *            The value for the parameter; may be <code>null</code>.
   */
  Parameterization(const SmartPointer<const IParameter> parameter, const std::string& value);

  /**
   * Copy constructor
   */
  Parameterization(const Parameterization& p);

  Parameterization& operator=(const Parameterization& p);

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator==(const Parameterization& parameterization) const;

  operator bool() const;

  /**
   * Returns the parameter that is being parameterized.
   *
   * @return The parameter; never <code>null</code>.
   */
  SmartPointer<const IParameter> GetParameter() const;

  /**
   * Returns the value for the parameter in this parameterization.
   *
   * @return The value; may be <code>null</code>.
   */
  std::string GetValue() const;

  /**
   * Returns the human-readable name for the current value, if any. If the
   * name cannot be found, then it simply returns the value. It also ensures
   * that any <code>null</code> values are converted into an empty string.
   *
   * @return The human-readable name of the value; never <code>null</code>.
   * @throws ParameterValuesException
   *             If the parameter needed to be initialized, but couldn't be.
   */
  std::string GetValueName() const throw(ParameterValuesException);

  /* (non-Javadoc)
   * @see java.lang.Object#hashCode()
   */
  std::size_t HashCode() const;
};

}

#endif /* CHERRYPARAMETERIZATION_H_ */
