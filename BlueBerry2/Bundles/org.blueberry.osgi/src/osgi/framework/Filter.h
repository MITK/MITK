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


#ifndef CHERRYFILTER_H_
#define CHERRYFILTER_H_

#include "Object.h"
#include "Macros.h"

#include "Exceptions.h"

#include "../util/Dictionary.h"

#include <Poco/HashMap.h>


namespace osgi {
namespace framework {

using namespace osgi::util;

struct ServiceReference;

/**
 * An RFC 1960-based Filter.
 * <p>
 * <code>Filter</code>s can be created by calling
 * {@link BundleContext#CreateFilter} or {@link FrameworkUtil#CreateFilter} with
 * a filter string.
 * <p>
 * A <code>Filter</code> can be used numerous times to determine if the match
 * argument matches the filter string that was used to create the
 * <code>Filter</code>.
 * <p>
 * Some examples of LDAP filters are:
 *
 * <pre>
 *  &quot;(cn=Babs Jensen)&quot;
 *  &quot;(!(cn=Tim Howes))&quot;
 *  &quot;(&amp;(&quot; + Constants.OBJECTCLASS + &quot;=Person)(|(sn=Jensen)(cn=Babs J*)))&quot;
 *  &quot;(o=univ*of*mich*)&quot;
 * </pre>
 *
 * @since 1.1
 * @see "Core Specification, section 5.5, for a description of the filter string syntax."
 * @ThreadSafe
 * @version $Revision$
 */
struct Filter : public Object {

  osgiInterfaceMacro(osgi::framework::Filter)

  /**
   * Filter using a service's properties.
   * <p>
   * This <code>Filter</code> is executed using the keys and values of the
   * referenced service's properties. The keys are case insensitively matched
   * with this <code>Filter</code>.
   *
   * @param reference The reference to the service whose properties are used
   *        in the match.
   * @return <code>true</code> if the service's properties match this
   *         <code>Filter</code>; <code>false</code> otherwise.
   */
  virtual bool Match(SmartPointer<ServiceReference> reference) = 0;

  /**
   * Filter using a <code>Dictionary</code>. This <code>Filter</code> is
   * executed using the specified <code>Dictionary</code>'s keys and values.
   * The keys are case insensitively matched with this <code>Filter</code>.
   *
   * @param dictionary The <code>Dictionary</code> whose keys are used in the
   *        match.
   * @return <code>true</code> if the <code>Dictionary</code>'s keys and
   *         values match this filter; <code>false</code> otherwise.
   * @throws IllegalArgumentException If <code>dictionary</code> contains case
   *         variants of the same key name.
   */
  virtual bool Match(SmartPointer<const Dictionary> dictionary) throw(IllegalArgumentException) = 0;

  /**
   * Returns this <code>Filter</code>'s filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return This <code>Filter</code>'s filter string.
   */
  virtual std::string ToString() const = 0;

  /**
   * Compares this <code>Filter</code> to another <code>Filter</code>.
   *
   * <p>
   * This method returns the result of calling
   * <code>this->ToString() == obj->ToString()</code>.
   *
   * @param obj The object to compare against this <code>Filter</code>.
   * @return If the other object is a <code>Filter</code> object, then returns
   *         the result of calling
   *         <code>this->ToString() == obj->ToString()</code>;
   *         <code>false</code> otherwise.
   */
  virtual bool operator==(const Object* obj) const = 0;

  /**
   * Returns the hashCode for this <code>Filter</code>.
   *
   * <p>
   * This method returns the result of calling
   * <code>Poco::Hash(this->ToString())</code>.
   *
   * @return The hashCode of this <code>Filter</code>.
   */
  virtual std::size_t HashCode() const = 0;

  /**
   * Filter with case sensitivity using a <code>Dictionary</code>. This
   * <code>Filter</code> is executed using the specified
   * <code>Dictionary</code>'s keys and values. The keys are case sensitively
   * matched with this <code>Filter</code>.
   *
   * @param dictionary The <code>Dictionary</code> whose keys are used in the
   *        match.
   * @return <code>true</code> if the <code>Dictionary</code>'s keys and
   *         values match this filter; <code>false</code> otherwise.
   */
  virtual bool MatchCase(SmartPointer<const Dictionary> dictionary) = 0;
};

}
}

#endif /* CHERRYFILTER_H_ */
