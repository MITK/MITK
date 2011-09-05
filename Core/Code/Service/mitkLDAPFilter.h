/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKLDAPFILTER_H
#define MITKLDAPFILTER_H

#include "mitkServiceReference.h"
#include "mitkServiceProperties.h"

#include "mitkSharedData.h"

namespace mitk {

class LDAPFilterData;

/**
 * \ingroup MicroServices
 *
 * An <a href="http://www.ietf.org/rfc/rfc1960.txt">RFC 1960</a>-based Filter.
 *
 * <p>
 * A <code>LDAPFilter</code> can be used numerous times to determine if the match
 * argument matches the filter string that was used to create the <code>LDAPFilter</code>.
 * <p>
 * Some examples of LDAP filters are:
 *
 *   - "(cn=Babs Jensen)"
 *   - "(!(cn=Tim Howes))"
 *   - "(&(" + ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))"
 *   - "(o=univ*of*mich*)"
 *
 * \remarks This class is thread safe.
 */
class MITK_CORE_EXPORT LDAPFilter {

public:

  /**
   * Creates in invalid <code>LDAPFilter</code> object.
   * Test the validity by using the boolean conversion operator.
   *
   * <p>
   * Calling methods on an invalid <code>LDAPFilter</code>
   * will result in undefined behavior.
   */
  LDAPFilter();

  /**
   * Creates a <code>LDAPFilter</code> object. This <code>LDAPFilter</code>
   * object may be used to match a <code>ServiceReference</code> object or a
   * <code>ServiceProperties</code> object.
   *
   * <p>
   * If the filter cannot be parsed, an std::invalid_argument will be
   * thrown with a human readable message where the filter became unparsable.
   *
   * @param filter The filter string.
   * @return A <code>LDAPFilter</code> object encapsulating the filter string.
   * @throws std::invalid_argument If <code>filter</code> contains an invalid
   *         filter string that cannot be parsed.
   * @see "Framework specification for a description of the filter string syntax." TODO!
   */
  LDAPFilter(const std::string& filter);

  LDAPFilter(const LDAPFilter& other);

  ~LDAPFilter();

  operator bool() const;

  /**
   * Filter using a service's properties.
   * <p>
   * This <code>LDAPFilter</code> is executed using the keys and values of the
   * referenced service's properties. The keys are looked up in a case
   * insensitive manner.
   *
   * @param reference The reference to the service whose properties are used
   *        in the match.
   * @return <code>true</code> if the service's properties match this
   *         <code>LDAPFilter</code> <code>false</code> otherwise.
   */
  bool Match(const ServiceReference& reference) const;

  /**
   * Filter using a <code>ServiceProperties</code> object with case insensitive key lookup. This
   * <code>LDAPFilter</code> is executed using the specified <code>ServiceProperties</code>'s keys
   * and values. The keys are looked up in a case insensitive manner.
   *
   * @param dictionary The <code>ServiceProperties</code> whose key/value pairs are used
   *        in the match.
   * @return <code>true</code> if the <code>ServiceProperties</code>'s values match this
   *         filter; <code>false</code> otherwise.
   */
  bool Match(const ServiceProperties& dictionary) const;

  /**
   * Filter using a <code>ServiceProperties</code>. This <code>LDAPFilter</code> is executed using
   * the specified <code>ServiceProperties</code>'s keys and values. The keys are looked
   * up in a normal manner respecting case.
   *
   * @param dictionary The <code>ServiceProperties</code> whose key/value pairs are used
   *        in the match.
   * @return <code>true</code> if the <code>ServiceProperties</code>'s values match this
   *         filter; <code>false</code> otherwise.
   */
  bool MatchCase(const ServiceProperties& dictionary) const;

  /**
   * Returns this <code>LDAPFilter</code>'s filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return This <code>LDAPFilter</code>'s filter string.
   */
  std::string ToString() const;

  /**
   * Compares this <code>LDAPFilter</code> to another <code>LDAPFilter</code>.
   *
   * <p>
   * This implementation returns the result of calling
   * <code>this->ToString() == other.ToString()</code>.
   *
   * @param other The object to compare against this <code>LDAPFilter</code>.
   * @return Returns the result of calling
   *         <code>this->ToString() == other.ToString()</code>.
   */
  bool operator==(const LDAPFilter& other) const;

  LDAPFilter& operator=(const LDAPFilter& filter);

protected:

  SharedDataPointer<LDAPFilterData> d;

};

}

/**
 * \ingroup MicroServices
 */
MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, const mitk::LDAPFilter& filter);

#endif // MITKLDAPFILTER_H
