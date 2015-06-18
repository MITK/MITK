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

#ifndef _BERRY_QUALIFIED_NAME_
#define _BERRY_QUALIFIED_NAME_

#include <string>
#include "berryObject.h"
#include <org_blueberry_core_jobs_Export.h>

namespace berry
{

/**
 * Qualified names are two-part names: qualifier and local name.
 * The qualifier must be in URI form (see RFC2396).
 * Note however that the qualifier may be <code>null</code> if
 * the default name space is being used.  The empty string is not
 * a valid local name.
 * <p>
 * This class can be used without OSGi running.
 * </p><p>
 * This class is not intended to be subclassed by clients.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.ds
 */
struct BERRY_JOBS QualifiedName
{

  /** Qualifier part (potentially <code>null</code>). */
  /*package*/

  std::string qualifier;

  /** Local name part. */
  /*package*/
  std::string localName;

  /**
   * Creates and returns a new qualified name with the given qualifier
   * and local name.  The local name must not be the empty string.
   * The qualifier may be <code>null</code>.
   * <p>
   * Clients may instantiate.
   * </p>
   * @param qualifier the qualifier string, or <code>null</code>
   * @param localName the local name string
   */
  QualifiedName(std::string qualifier, std::string localName);

  /**
   * Returns whether this qualified name is equivalent to the given object.
   * <p>
   * Qualified names are equal if and only if they have the same
   * qualified parts and local parts.
   * Qualified names are not equal to objects other than qualified names.
   * </p>
   *
   * @param obj the object to compare to
   * @return <code>true</code> if these are equivalent qualified
   *    names, and <code>false</code> otherwise
   */
  bool operator==(const QualifiedName& qName) const;

  bool operator<(const QualifiedName& qName) const;

  /**
   * Returns the local part of this name.
   *
   * @return the local name string
   */
  std::string GetLocalName() const;

  /**
   * Returns the qualifier part for this qualified name, or <code>null</code>
   * if none.
   *
   * @return the qualifier string, or <code>null</code>
   */
  std::string GetQualifier() const;

  /*
   * Implements the method <code>Object.hashCode</code>.
   *
   * Returns the hash code for this qualified name.
   */
  std::size_t HashCode() const;

  /**
   * Converts this qualified name into a string, suitable for
   * debug purposes only.
   */
  std::string ToString();

};

}

#endif // _BERRY_QUALIFIED_NAME_
