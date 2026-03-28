/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USLDAPPROP_H
#define USLDAPPROP_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>
#include <usAny.h>

namespace us {

/// \cond
class MITKCPPMICROSERVICES_EXPORT LDAPPropExpr
{
public:

  explicit LDAPPropExpr(const std::string& expr);

  LDAPPropExpr& operator!();

  operator std::string() const;

  bool IsNull() const;

private:

  LDAPPropExpr& operator=(const LDAPPropExpr&);

  std::string m_ldapExpr;
};
/// \endcond

/**
 * \ingroup MicroServicesUtils
 *
 * A fluent API for creating LDAP filter strings.
 *
 * Examples for creating LDAPFilter objects:
 * \code
 * // This creates the filter "(&(name=Ben)(!(count=1)))"
 * LDAPFilter filter(LDAPProp("name") == "Ben" && !(LDAPProp("count") == 1));
 *
 * // This creates the filter "(|(presence=*)(!(absence=*)))"
 * LDAPFilter filter(LDAPProp("presence") || !LDAPProp("absence"));
 *
 * // This creates the filter "(&(ge>=-3)(approx~=hi))"
 * LDAPFilter filter(LDAPProp("ge") >= -3 && LDAPProp("approx").Approx("hi"));
 * \endcode
 *
 * \sa LDAPFilter
 */
class MITKCPPMICROSERVICES_EXPORT LDAPProp
{
public:

  /**
   * Create a LDAPProp instance for the named LDAP property.
   *
   * @param property The name of the LDAP property.
   */
  LDAPProp(const std::string& property);

  /**
   * LDAP equality '='
   *
   * @param s A type convertible to std::string.
   * @return A LDAP expression object.
   *
   * @{
   */
  LDAPPropExpr operator==(const std::string& s) const;
  LDAPPropExpr operator==(const us::Any& s) const;
  template<class T>
  LDAPPropExpr operator==(const T& s) const
  {
    std::stringstream ss;
    ss << s;
    return LDAPPropExpr("(" + m_property + "=" + ss.str() + ")");
  }
  /// @}

  operator LDAPPropExpr () const;

  /**
   * States the absence of the LDAP property.
   *
   * @return A LDAP expression object.
   */
  LDAPPropExpr operator!() const;

  /**
   * Convenience operator for LDAP inequality.
   *
   * Writing either
   * \code
   * LDAPProp("attr") != "val"
   * \endcode
   * or
   * \code
   * !(LDAPProp("attr") == "val")
   * \endcode
   * leads to the same string "(!(attr=val))".
   *
   * @param s A type convertible to std::string.
   * @return A LDAP expression object.
   *
   * @{
   */
  LDAPPropExpr operator!=(const std::string& s) const;
  LDAPPropExpr operator!=(const us::Any& s) const;
  template<class T>
  LDAPPropExpr operator!=(const T& s) const
  {
    std::stringstream ss;
    ss << s;
    return operator!=(ss.str());
  }
  /// @}

  /**
   * LDAP greater or equal '>='
   *
   * @param s A type convertible to std::string.
   * @return A LDAP expression object.
   *
   * @{
   */
  LDAPPropExpr operator>=(const std::string& s) const;
  LDAPPropExpr operator>=(const us::Any& s) const;
  template<class T>
  LDAPPropExpr operator>=(const T& s) const
  {
    std::stringstream ss;
    ss << s;
    return operator>=(ss.str());
  }
  /// @}

  /**
   * LDAP less or equal '<='
   *
   * @param s A type convertible to std::string.
   * @return A LDAP expression object.
   *
   * @{
   */
  LDAPPropExpr operator<=(const std::string& s) const;
  LDAPPropExpr operator<=(const us::Any& s) const;
  template<class T>
  LDAPPropExpr operator<=(const T& s) const
  {
    std::stringstream ss;
    ss << s;
    return operator<=(ss.str());
  }
  /// @}

  /**
   * LDAP approximation '~='
   *
   * @param s A type convertible to std::string.
   * @return A LDAP expression object.
   *
   * @{
   */
  LDAPPropExpr Approx(const std::string& s) const;
  LDAPPropExpr Approx(const us::Any& s) const;
  template<class T>
  LDAPPropExpr Approx(const T& s) const
  {
    std::stringstream ss;
    ss << s;
    return Approx(ss.str());
  }
  /// @}

private:

  LDAPProp& operator=(const LDAPProp&);

  std::string m_property;
};

}

/**
 * \ingroup MicroServicesUtils
 *
 * LDAP logical and '&'
 *
 * @param left A LDAP expression.
 * @param right A LDAP expression.
 * @return A LDAP expression
 */
MITKCPPMICROSERVICES_EXPORT us::LDAPPropExpr operator&&(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right);

/**
 * \ingroup MicroServicesUtils
 *
 * LDAP logical or '|'
 *
 * @param left A LDAP expression.
 * @param right A LDAP expression.
 * @return A LDAP expression
 */
MITKCPPMICROSERVICES_EXPORT us::LDAPPropExpr operator||(const us::LDAPPropExpr& left, const us::LDAPPropExpr& right);

#endif // USLDAPPROP_H
